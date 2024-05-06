#include "render/resource_state_tracker.h"

#include "render/command_list.h"
#include "render/resource.h"

namespace cera
{
    // Static definitions.
    std::mutex resource_state_tracker::s_global_mutex;
    bool resource_state_tracker::s_is_locked = false;
    resource_state_tracker::resource_state_map resource_state_tracker::s_global_resource_state;

    resource_state_tracker::resource_state_tracker()
    {}

    resource_state_tracker::~resource_state_tracker()
    {}

    void resource_state_tracker::resource_barrier(const D3D12_RESOURCE_BARRIER& barrier)
    {
        if (barrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION)
        {
            const D3D12_RESOURCE_TRANSITION_BARRIER& transition_barrier = barrier.Transition;

            // First check if there is already a known "final" state for the given resource.
            // If there is, the resource has been used on the command list before and
            // already has a known state within the command list execution.
            const auto iter = m_final_resource_state.find(transition_barrier.pResource);
            if (iter != m_final_resource_state.end())
            {
                auto& resource_state = iter->second;
                // If the known final state of the resource is different...
                if (transition_barrier.Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES &&
                    !resource_state.subresource_state.empty())
                {
                    // First transition all of the subresources if they are different than the StateAfter.
                    for (auto subresource_state : resource_state.subresource_state)
                    {
                        if (transition_barrier.StateAfter != subresource_state.second)
                        {
                            D3D12_RESOURCE_BARRIER new_barrier = barrier;
                            new_barrier.Transition.Subresource = subresource_state.first;
                            new_barrier.Transition.StateBefore = subresource_state.second;
                            m_resource_barriers.push_back(new_barrier);
                        }
                    }
                }
                else
                {
                    auto final_state = resource_state.get_subresource_state(transition_barrier.Subresource);
                    if (transition_barrier.StateAfter != final_state)
                    {
                        // Push a new transition barrier with the correct before state.
                        D3D12_RESOURCE_BARRIER new_barrier = barrier;
                        new_barrier.Transition.StateBefore = final_state;
                        m_resource_barriers.push_back(new_barrier);
                    }
                }
            }
            else // In this case, the resource is being used on the command list for the first time. 
            {
                // Add a pending barrier. The pending barriers will be resolved
                // before the command list is executed on the command queue.
                m_pending_resource_barriers.push_back(barrier);
            }

            // Push the final known state (possibly replacing the previously known state for the subresource).
            m_final_resource_state[transition_barrier.pResource].set_subresource_state(transition_barrier.Subresource, transition_barrier.StateAfter);
        }
        else
        {
            // Just push non-transition barriers to the resource barriers array.
            m_resource_barriers.push_back(barrier);
        }
    }

    void resource_state_tracker::transition_resource(ID3D12Resource* resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource)
    {
        if (resource)
        {
            resource_barrier(CD3DX12_RESOURCE_BARRIER::Transition(resource, D3D12_RESOURCE_STATE_COMMON, stateAfter, subResource));
        }
    }

    void resource_state_tracker::transition_resource(const resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource)
    {
        transition_resource(resource.get_d3d_resource().Get(), stateAfter, subResource);
    }

    void resource_state_tracker::flush_resource_barriers(const std::shared_ptr<command_list>& commandList)
    {
        UINT num_barriers = static_cast<UINT>(m_resource_barriers.size());
        if (num_barriers > 0)
        {
            auto d3d_command_list = commandList->get_graphics_command_list();
            d3d_command_list->ResourceBarrier(num_barriers, m_resource_barriers.data());
            m_resource_barriers.clear();
        }
    }

    uint32_t resource_state_tracker::flush_pending_resource_barriers(const std::shared_ptr<command_list>& commandList)
    {
        assert(s_is_locked);

        // Resolve the pending resource barriers by checking the global state of the 
        // (sub)resources. Add barriers if the pending state and the global state do
        //  not match.
        resource_barriers resource_barriers;
        // Reserve enough space (worst-case, all pending barriers).
        resource_barriers.reserve(m_pending_resource_barriers.size());

        for (auto pending_barrier : m_pending_resource_barriers)
        {
            if (pending_barrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION)  // Only transition barriers should be pending...
            {
                auto pending_transition = pending_barrier.Transition;

                const auto& iter = s_global_resource_state.find(pending_transition.pResource);
                if (iter != s_global_resource_state.end())
                {
                    // If all subresources are being transitioned, and there are multiple
                    // subresources of the resource that are in a different state...
                    auto& resource_state = iter->second;
                    if (pending_transition.Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES &&
                        !resource_state.subresource_state.empty())
                    {
                        // Transition all subresources
                        for (auto subresource_state : resource_state.subresource_state)
                        {
                            if (pending_transition.StateAfter != subresource_state.second)
                            {
                                D3D12_RESOURCE_BARRIER new_barrier = pending_barrier;
                                new_barrier.Transition.Subresource = subresource_state.first;
                                new_barrier.Transition.StateBefore = subresource_state.second;
                                resource_barriers.push_back(new_barrier);
                            }
                        }
                    }
                    else
                    {
                        // No (sub)resources need to be transitioned. Just add a single transition barrier (if needed).
                        auto global_state = (iter->second).get_subresource_state(pending_transition.Subresource);

                        if (pending_transition.StateAfter != global_state)
                        {
                            // Fix-up the before state based on current global state of the resource.
                            pending_barrier.Transition.StateBefore = global_state;
                            resource_barriers.push_back(pending_barrier);
                        }
                    }
                }
            }
        }

        UINT num_barriers = static_cast<UINT>(resource_barriers.size());
        if (num_barriers > 0)
        {
            auto d3d_command_list = commandList->get_graphics_command_list();
            d3d_command_list->ResourceBarrier(num_barriers, resource_barriers.data());
        }

        m_pending_resource_barriers.clear();

        return num_barriers;
    }

    void resource_state_tracker::commit_final_resource_states()
    {
        assert(s_is_locked);

        // Commit final resource states to the global resource state array (map).
        for (const auto& resource_state : m_final_resource_state)
        {
            s_global_resource_state[resource_state.first] = resource_state.second;
        }

        m_final_resource_state.clear();
    }

    void resource_state_tracker::reset()
    {
        // Reset the pending, current, and final resource states.
        m_pending_resource_barriers.clear();
        m_resource_barriers.clear();
        m_final_resource_state.clear();
    }

    void resource_state_tracker::lock()
    {
        s_global_mutex.lock();
        s_is_locked = true;
    }

    void resource_state_tracker::unlock()
    {
        s_global_mutex.unlock();
        s_is_locked = false;
    }

    void resource_state_tracker::add_global_resource_state(ID3D12Resource* resource, D3D12_RESOURCE_STATES state)
    {
        if (resource != nullptr)
        {
            std::lock_guard<std::mutex> lock(s_global_mutex);
            s_global_resource_state[resource].set_subresource_state(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, state);
        }
    }
}