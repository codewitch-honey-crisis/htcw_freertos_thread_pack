#pragma once
#include <atomic>
#include "freertos_core.hpp"
#include "synchronization_context.hpp"
#include "thread.hpp"
namespace freertos {
class thread_pool {
    struct entry_thunk {
        void* calling_handle;
        synchronization_context* sync;
        std::atomic_bool* quitting;
        std::atomic_int* thread_count;
    };
    std::atomic_bool m_quitting;
    std::atomic_int m_thread_count;
    synchronization_context m_sync;

    static void task_entry_thunk(void* state);
    thread_pool(const thread_pool& rhs)=delete;
    thread_pool& operator=(const thread_pool& rhs)=delete;
public:
    // creates a new instance
    inline thread_pool() : m_quitting(false),m_thread_count(0) {
    }
    // shuts down the thread pool
    inline ~thread_pool() { shutdown(); }
    // indicates whether the pool is initialized
    inline bool initialized() const { return m_sync.initialized(); }
    // explicitly initializes the pool, with the maximum number of pending operations
    inline bool initialize(size_t max_pending=5) { return m_sync.initialize(max_pending); }
    // resource stealing overload
    thread_pool(thread_pool&& rhs);
    // resource stealing overload
    thread_pool& operator=(thread_pool&& rhs);
    // create a new thread in the pool with the specified priority and stack word size
    thread create_thread(int priority=1,size_t stack_word_size=1000);
#ifdef ESP32
    // create a new thread in the pool on the specified CPU core with the specified priority and stack word size
    thread create_thread_affinity(int cpu=0,int priority=1,size_t stack_word_size=1000);
#endif
    // indicates the number of threads in the pool
    inline size_t thread_count() const { return (size_t)m_thread_count; }
    // queues an item for completion by a waiting thread in the pool
    bool queue_work_item(void(*fn)(void*),void* state=nullptr, bool wait=true);
    // shuts the pool down optionally forcibly aborting the threads
    void shutdown(bool abort = false);
};
}
