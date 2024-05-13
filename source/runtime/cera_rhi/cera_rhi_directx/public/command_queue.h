#pragma once

#include "util/types.h"
#include "util/windows_types.h"
#include "util/queue.h"

#include "directx_util.h"

#include <memory>
#include <queue>
#include <vector>
#include <atomic>
#include <condition_variable>
#include <thread>
#include <mutex>

namespace cera
{
  namespace renderer
  {
    class CommandList;
    class Device;

    class CommandQueue
    {
    public:
      std::shared_ptr<CommandList> command_list();
      wrl::ComPtr<ID3D12CommandQueue> d3d_command_queue() const;

      // Execute a command list.
      // Returns the fence value to wait for for this command list.
      u64 execute_command_list(std::shared_ptr<CommandList> commandList);
      u64 execute_command_lists(const std::vector<std::shared_ptr<CommandList>>& commandLists);

      u64 signal();
      bool is_fence_complete(u64 fenceValue) const;
      void wait_for_fence_value(u64 fenceValue);
      void flush();

      // Wait for another command queue to finish.
      void wait(const CommandQueue& other);

    protected:
      friend struct std::default_delete<CommandQueue>;

      // Only the device can create command queues.
      CommandQueue(Device& device, D3D12_COMMAND_LIST_TYPE type);
      virtual ~CommandQueue();

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
      struct CommandListEntry
      {
        u64 fence_value;
        std::shared_ptr<CommandList> command_list;
      };

      Device& m_device;
      D3D12_COMMAND_LIST_TYPE m_command_list_type;
      wrl::ComPtr<ID3D12CommandQueue> m_d3d_command_queue;
      wrl::ComPtr<ID3D12Fence> m_d3d_fence;
      u64 m_fence_value;

      threading::Queue<CommandListEntry> m_in_flight_command_lists;
      threading::Queue<std::shared_ptr<CommandList>> m_available_command_lists;

      // A thread to process in-flight command lists.
      std::thread m_process_in_flight_command_lists_thread;
      std::atomic<bool> m_bprocess_in_flight_command_lists;
      std::mutex m_process_in_flight_command_lists_thread_mutex;
      std::condition_variable m_process_in_flight_command_lists_thread_CV;
    };
  } // namespace renderer
} // namespace cera