#pragma once

#include <mutex>
#include <queue>

namespace cera
{
  namespace renderer
  {
    namespace threading
    {
      template <typename T>
      class Queue
      {
      public:
        Queue();
        Queue(const Queue& copy);

        /**
         * push a value into the back of the queue.
         */
        void push(T value);

        /**
         * Try to pop a value from the front of the queue.
         * @returns false if the queue is empty.
         */
        bool try_pop(T& value);

        /**
         * Check to see if there are any items in the queue.
         */
        bool empty() const;

        /**
         * Retrieve the number of items in the queue.
         */
        size_t size() const;

      private:
        std::queue<T> m_queue;
        mutable std::mutex m_mutex;
      };

      template <typename T>
      Queue<T>::Queue()
      {
      }

      template <typename T>
      Queue<T>::Queue(const Queue<T>& copy)
      {
        std::unique_lock<std::mutex> lock(copy.m_mutex);
        m_queue = copy.m_queue;
      }

      template <typename T>
      void Queue<T>::push(T value)
      {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(std::move(value));
      }

      template <typename T>
      bool Queue<T>::try_pop(T& value)
      {
        std::unique_lock<std::mutex> lock(m_mutex);
        if(m_queue.empty())
          return false;

        value = m_queue.front();
        m_queue.pop();

        return true;
      }

      template <typename T>
      bool Queue<T>::empty() const
      {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.empty();
      }

      template <typename T>
      size_t Queue<T>::size() const
      {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.size();
      }
    } // namespace threading
  }   // namespace renderer
} // namespace cera