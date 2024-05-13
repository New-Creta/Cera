#pragma once

#include "util/types.h"

#include <type_traits>

namespace cera
{
  template <typename TState, std::enable_if_t<std::is_enum_v<TState>, bool> = true>
  class StateController
  {
  public:
      //-----------------------------------------------------------------------
    StateController()
        : m_state(static_cast<TState>(0))
    {
    }
    //-----------------------------------------------------------------------
    explicit StateController(TState state)
        : m_state(state)
    {
    }

    //-----------------------------------------------------------------------
    void add_state(TState state)
    {
      m_state = static_cast<TState>(static_cast<u32>(m_state) | static_cast<u32>(state));
    }
    //-----------------------------------------------------------------------
    void remove_state(TState state)
    {
      m_state = static_cast<TState>(static_cast<u32>(m_state) & ~static_cast<u32>(state));
    }
    //-----------------------------------------------------------------------
    void change_state(TState state)
    {
      m_state = static_cast<TState>(0);

      add_state(state);
    }

    //-----------------------------------------------------------------------
    bool has_state(TState state) const
    {
      return (static_cast<u32>(m_state) & static_cast<u32>(state)) != 0u;
    }
    //-----------------------------------------------------------------------
    TState state() const
    {
        return m_state;
    }

  private:
    TState m_state;
  };
} // namespace cera