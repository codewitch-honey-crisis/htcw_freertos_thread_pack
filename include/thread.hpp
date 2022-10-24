#pragma once
#include "freertos_core.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
namespace freertos {
class thread final {
    void* m_handle;
    struct entry_thunk {
        void *calling_handle;
        void *state;
        void (*fn)(void*);
    };
    static void task_entry_thunk(void* state);
    thread(const thread& rhs)=delete;
    thread& operator=(const thread& rhs)=delete;
public:
    inline thread() : m_handle(nullptr) {}
    inline thread(thread&& rhs) {
        m_handle = rhs.m_handle;
        rhs.m_handle = nullptr;
    }
    inline thread& operator=(thread&& rhs) {
        m_handle = rhs.m_handle;
        rhs.m_handle = nullptr;
        return *this;
    }
    ~thread()=default;
    inline void set(void* thread_handle) {
        m_handle = thread_handle;
    }
    void sleep(size_t ms);
    static thread current();
#ifdef ESP32
    static thread current(int cpu);
#endif
    static thread idle();
#ifdef ESP32
    static thread idle(int cpu);
#endif
    int priority() const;
    bool priority(int value);
#ifdef ESP32
    int affinity() const;
#endif
    bool start();
    bool abort();
    bool suspend();
    bool running() const; 
    void* handle() const;
    static thread create(void(*fn)(void*),void* state=nullptr,int priority=1,size_t stack_word_size=1000);
#ifdef ESP32
    static thread create_affinity(int cpu,void(*fn)(void*),void* state=nullptr,int priority=1,size_t stack_word_size=1000);
#endif
};
}