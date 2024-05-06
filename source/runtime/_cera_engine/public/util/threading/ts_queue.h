#pragma once

#include <queue>
#include <mutex>

namespace cera
{
    namespace threading
    {
        template<typename T>
        class queue
        {
        public:
            queue();
            queue(const queue& copy);

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

        template<typename T>
        queue<T>::queue()
        {}

        template<typename T>
        queue<T>::queue(const queue<T>& copy)
        {
            std::lock_guard<std::mutex> lock(copy.m_mutex);
            m_queue = copy.m_queue;
        }

        template<typename T>
        void queue<T>::push(T value)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push(std::move(value));
        }

        template<typename T>
        bool queue<T>::try_pop(T& value)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_queue.empty())
                return false;

            value = m_queue.front();
            m_queue.pop();

            return true;
        }

        template<typename T>
        bool queue<T>::empty() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_queue.empty();
        }

        template<typename T>
        size_t queue<T>::size() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_queue.size();
        }
    }
}