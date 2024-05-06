#pragma once

#include "util/types.h"
#include "util/threading/ts_queue.h"

#include "device/windows_types.h"

#include "render/d3dx12_declarations.h"

#include <queue>
#include <vector>
#include <memory>

namespace cera
{
    class command_list;
    class device;

    class command_queue
    {
    public:
        std::shared_ptr<command_list> get_command_list();
        wrl::ComPtr<ID3D12CommandQueue> get_d3d_command_queue() const;

        // Execute a command list.
        // Returns the fence value to wait for for this command list.
        uint64_t execute_command_list(std::shared_ptr<command_list> commandList);
        uint64_t execute_command_lists(const std::vector<std::shared_ptr<command_list>>& commandLists);

        u64 signal();
        bool is_fence_complete(u64 fenceValue) const;
        void wait_for_fence_value(u64 fenceValue);
        void flush();

        // Wait for another command queue to finish.
        void wait(const command_queue& other);

    protected:
        friend class std::default_delete<command_queue>;

        // Only the device can create command queues.
        command_queue(device& device, D3D12_COMMAND_LIST_TYPE type);
        virtual ~command_queue();

    private:
        // Free any command lists that are finished processing on the command queue.
        void proccess_in_flight_command_lists();

    private:
        // Keep track of command allocators that are in fligh
        struct CommandAllocatorEntry
        {
            u64 fence_value;
            wrl::ComPtr<ID3D12CommandAllocator> command_allocator;
        };

        // Keep track of command allocators that are "in-flight"
        // The first member is the fence value to wait for, the second is the
        // a shared pointer to the "in-flight" command list.
        using command_list_entry = std::tuple<uint64_t, std::shared_ptr<command_list>>;

        device&                             m_device;
        D3D12_COMMAND_LIST_TYPE             m_command_list_type;
        wrl::ComPtr<ID3D12CommandQueue>     m_d3d_command_queue;
        wrl::ComPtr<ID3D12Fence>            m_d3d_fence;
        u64                                 m_fence_value;

        threading::queue<command_list_entry>             m_in_flight_command_lists;
        threading::queue<std::shared_ptr<command_list>>  m_available_command_lists;

        // A thread to process in-flight command lists.
        std::thread                         m_process_in_flight_command_lists_thread;
        std::atomic_bool                    m_bprocess_in_flight_command_lists;
        std::mutex                          m_process_in_flight_command_lists_thread_mutex;
        std::condition_variable             m_process_in_flight_command_lists_thread_CV;
    };
}