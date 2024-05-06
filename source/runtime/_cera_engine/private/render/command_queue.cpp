#include "render/command_queue.h"
#include "render/command_list.h"
#include "render/device.h"
#include "render/resource_state_tracker.h"
#include "render/d3dx12_call.h"

#include "device/windows_declarations.h"

#include "util/log.h"
#include "util/threading/thread_helpers.h"

namespace cera
{
    namespace conversions
    {
        static std::string to_string(D3D12_COMMAND_LIST_TYPE type)
        {
            switch (type)
            {
            case D3D12_COMMAND_LIST_TYPE_DIRECT: return "D3D12_COMMAND_LIST_TYPE_DIRECT";
            case D3D12_COMMAND_LIST_TYPE_COPY: return "D3D12_COMMAND_LIST_TYPE_COPY";
            case D3D12_COMMAND_LIST_TYPE_COMPUTE: return "D3D12_COMMAND_LIST_TYPE_COMPUTE";
            }

            return "UNKNOWN";
        }
    }

    namespace internal
    {
        wrl::ComPtr<ID3D12CommandQueue> create_command_queue(wrl::ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type)
        {
            wrl::ComPtr<ID3D12CommandQueue> d3d12_command_queue;

            D3D12_COMMAND_QUEUE_DESC desc = {};
            desc.Type = type;
            desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
            desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            desc.NodeMask = 0;

            if (DX_FAILED(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&d3d12_command_queue))))
            {
                log::error("Unable to create the ID3D12CommandQueue");
                return nullptr;
            }

            return d3d12_command_queue;
        }

        wrl::ComPtr<ID3D12Fence> create_fence(wrl::ComPtr<ID3D12Device2> device)
        {
            wrl::ComPtr<ID3D12Fence> fence;

            if (DX_FAILED((device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)))))
            {
                log::error("Unable to create ID3D12Fence");
                return nullptr;
            }

            return fence;
        }
    }

    namespace adaptors
    {
        class make_command_list : public command_list
        {
        public:
            make_command_list(device& device, D3D12_COMMAND_LIST_TYPE type)
                : command_list(device, type)
            {}

            ~make_command_list() override = default;
        };
    }

    command_queue::command_queue(device& device, D3D12_COMMAND_LIST_TYPE type)
        :m_device(device)
        ,m_command_list_type(type)
        ,m_fence_value(0)
        ,m_bprocess_in_flight_command_lists(true)
    {
        auto d3d_device = m_device.get_d3d_device();

        m_d3d_command_queue = internal::create_command_queue(d3d_device, type);
        assert(m_d3d_command_queue != nullptr && "Failed to create ID3D12CommandQueue");
        m_d3d_fence = internal::create_fence(d3d_device);
        assert(m_d3d_fence != nullptr && "Failed to create ID3D12Fence");

        switch (type)
        {
        case D3D12_COMMAND_LIST_TYPE_COPY:
            m_d3d_command_queue->SetName(L"Copy Command Queue");
            break;
        case D3D12_COMMAND_LIST_TYPE_COMPUTE:
            m_d3d_command_queue->SetName(L"Compute Command Queue");
            break;
        case D3D12_COMMAND_LIST_TYPE_DIRECT:
            m_d3d_command_queue->SetName(L"Direct Command Queue");
            break;
        }

        // Set the thread name for easy debugging.
        char thread_name[256];
        sprintf_s(thread_name, "Proccess In Flight Command Lists ");
        switch (type)
        {
        case D3D12_COMMAND_LIST_TYPE_DIRECT:
            strcat_s(thread_name, "(Direct)");
            break;
        case D3D12_COMMAND_LIST_TYPE_COMPUTE:
            strcat_s(thread_name, "(Compute)");
            break;
        case D3D12_COMMAND_LIST_TYPE_COPY:
            strcat_s(thread_name, "(Copy)");
            break;
        default:
            break;
        }

        m_process_in_flight_command_lists_thread = std::thread(&command_queue::proccess_in_flight_command_lists, this);
        threading::set_thread_name(m_process_in_flight_command_lists_thread, thread_name);
    }

    command_queue::~command_queue()
    {
        m_bprocess_in_flight_command_lists = false;
        m_process_in_flight_command_lists_thread.join();
    }

    std::shared_ptr<command_list> command_queue::get_command_list()
    {
        std::shared_ptr<command_list> command_list;

        // If there is a command list on the queue.
        if (!m_available_command_lists.empty())
        {
            m_available_command_lists.try_pop(command_list);
            if (command_list)
            {
                log::info("Available command list found of type: {0} - Instance nr: {1}", conversions::to_string(m_command_list_type), command_list->instance_nr());
            }
        }
        else
        {
            // Otherwise create a new command list.
            command_list = std::make_shared<adaptors::make_command_list>(m_device, m_command_list_type);

            log::info("No available command list found of type: {0} creating new one - Instance nr: {1}", conversions::to_string(m_command_list_type), command_list->instance_nr());
        }

        return command_list;
    }

    wrl::ComPtr<ID3D12CommandQueue> command_queue::get_d3d_command_queue() const
    {
        return m_d3d_command_queue;
    }

    // Execute a command list.
    // Returns the fence value to wait for for this command list.
    uint64_t command_queue::execute_command_list(std::shared_ptr<command_list> commandList)
    {
        return execute_command_lists(std::vector<std::shared_ptr<command_list>>({ commandList }));
    }

    uint64_t command_queue::execute_command_lists(const std::vector<std::shared_ptr<command_list>>& commandLists)
    {
        resource_state_tracker::lock();

        // Command lists that need to put back on the command list queue.
        std::vector<std::shared_ptr<command_list>> to_be_queued;
        to_be_queued.reserve(commandLists.size() * 2);  // 2x since each command list will have a pending command list.

        // Command lists that need to be executed.
        std::vector<ID3D12CommandList*> d3d_command_lists;
        d3d_command_lists.reserve(commandLists.size() * 2);  // 2x since each command list will have a pending command list.

        for (auto commandList : commandLists)
        {
            auto pending_command_list = get_command_list();
            bool has_pending_barriers = commandList->close(pending_command_list);
            pending_command_list->close();

            // If there are no pending barriers on the pending command list, there is no reason to
            // execute an empty command list on the command queue.
            if (has_pending_barriers)
            {
                d3d_command_lists.push_back(pending_command_list->get_graphics_command_list().Get());
            }
            
            d3d_command_lists.push_back(commandList->get_graphics_command_list().Get());

            to_be_queued.push_back(pending_command_list);
            to_be_queued.push_back(commandList);
        }

        UINT num_command_lists = static_cast<UINT>(d3d_command_lists.size());
        log::info("Execute command lists: {0}", num_command_lists);
        for (auto& cmd_list : d3d_command_lists)
        {
            log::info("CommandList type: {0}", conversions::to_string(cmd_list->GetType()));
        }

        m_d3d_command_queue->ExecuteCommandLists(num_command_lists, d3d_command_lists.data());
        uint64_t fenceValue = signal();

        resource_state_tracker::unlock();

        // Queue command lists for reuse.
        for (auto commandList : to_be_queued)
        {
            m_in_flight_command_lists.push({ fenceValue, commandList });
        }

        return fenceValue;
    }

    u64 command_queue::signal()
    {
        u64 fence_value = ++m_fence_value;
        m_d3d_command_queue->Signal(m_d3d_fence.Get(), fence_value);
        return fence_value;
    }

    bool command_queue::is_fence_complete(u64 fenceValue) const
    {
        return m_d3d_fence->GetCompletedValue() >= fenceValue;
    }

    void command_queue::wait_for_fence_value(u64 fenceValue)
    {
        if (!is_fence_complete(fenceValue))
        {
            auto event = ::CreateEvent(NULL, FALSE, FALSE, NULL);
            if (event)
            {
                m_d3d_fence->SetEventOnCompletion(fenceValue, event);
                ::WaitForSingleObject(event, DWORD_MAX);

                ::CloseHandle(event);
            }
        }
    }

    void command_queue::flush()
    {
        log::info("Flush command queue: {0}", conversions::to_string(m_command_list_type));

        std::unique_lock<std::mutex> lock(m_process_in_flight_command_lists_thread_mutex);
        m_process_in_flight_command_lists_thread_CV.wait(lock, [this] { return m_in_flight_command_lists.empty(); });

        // In case the command queue was signaled directly
        // using the command_queue::Signal method then the
        // fence value of the command queue might be higher than the fence
        // value of any of the executed command lists.
        wait_for_fence_value(m_fence_value);
    }

    void command_queue::wait(const command_queue& other)
    {
        m_d3d_command_queue->Wait(other.m_d3d_fence.Get(), other.m_fence_value);
    }

    void command_queue::proccess_in_flight_command_lists()
    {
        std::unique_lock<std::mutex> lock(m_process_in_flight_command_lists_thread_mutex, std::defer_lock);

        while (m_bprocess_in_flight_command_lists)
        {
            command_list_entry list_entry;

            lock.lock();
            while (m_in_flight_command_lists.try_pop(list_entry))
            {
                auto fence_value = std::get<0>(list_entry);
                auto command_list = std::get<1>(list_entry);

                log::info("Popping CommandListEntry from in flight CommandLists: Fence Value {0} - Type {1}", fence_value, conversions::to_string(command_list->get_command_list_type()));

                wait_for_fence_value(fence_value);

                command_list->reset();

                m_available_command_lists.push(command_list);

                log::info("Pushing CommandList into Available List of CommandLists: - Type {0}, Instance nr: {1}", conversions::to_string(m_command_list_type), command_list->instance_nr());
            }
            lock.unlock();

            m_process_in_flight_command_lists_thread_CV.notify_one();

            std::this_thread::yield();
        }
    }
}