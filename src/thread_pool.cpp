#include "thread_pool.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
namespace freertos {
namespace htcw_helpers {
    // implement std::move to limit dependencies on the STL, which may not be there
    template< class T > struct atomic_int_remove_reference      { typedef T type; };
    template< class T > struct atomic_int_remove_reference<T&>  { typedef T type; };
    template< class T > struct atomic_int_remove_reference<T&&> { typedef T type; };
    template <typename T>
    typename atomic_int_remove_reference<T>::type&& atomic_int_move(T&& arg) {
        return static_cast<typename atomic_int_remove_reference<T>::type&&>(arg);
    }
    
}
void thread_pool::task_entry_thunk(void* state) {
    entry_thunk thunk = *((entry_thunk*)state);
    synchronization_context& sc = *thunk.sync;
    std::atomic_bool& quitting = *thunk.quitting;
    std::atomic_int& thread_count = *thunk.thread_count;
    ++thread_count;
    if(thunk.calling_handle!=nullptr) {
        xTaskNotifyGive(thunk.calling_handle);
    }
    while(!quitting && sc.update(true));
    --thread_count;
    vTaskDelete(nullptr);
}
thread_pool::thread_pool(thread_pool&& rhs) {
    // TODO: why the hell doesn't this work?
    //m_quitting = htcw_helpers::atomic_int_move(rhs.m_quitting);
    rhs.m_quitting = false;
    m_sync = htcw_helpers::synchronization_context_move(rhs.m_sync);
    rhs.m_sync = htcw_helpers::synchronization_context_move(synchronization_context());
}
thread_pool& thread_pool::operator=(thread_pool&& rhs) {
    // TODO: why the hell doesn't this work?
    //m_quitting = htcw_helpers::atomic_int_move(rhs.m_quitting);
    rhs.m_quitting = false;
    m_sync = htcw_helpers::synchronization_context_move(rhs.m_sync);
    rhs.m_sync = htcw_helpers::synchronization_context_move(synchronization_context());
    return *this;
}
thread thread_pool::create_thread(int priority,size_t stack_word_size) {
    if(initialize()) {
        entry_thunk thunk;
        TaskHandle_t handle = nullptr;
        thunk.calling_handle=xTaskGetCurrentTaskHandle();
        thunk.sync = &m_sync;
        thunk.thread_count = &m_thread_count;
        thunk.quitting = &m_quitting;
        if(pdPASS==xTaskCreate(task_entry_thunk,"htcw_thread_pool_thread",stack_word_size,&thunk,priority,&handle)) {
            thread result;
            result.set(handle);
            ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
            return result;
        }
    }
    return thread();
}
#ifdef ESP32
thread thread_pool::create_thread_affinity(int cpu,int priority,size_t stack_word_size) {
    if(initialize()) {
        entry_thunk thunk;
        TaskHandle_t handle = nullptr;
        thunk.calling_handle=xTaskGetCurrentTaskHandle();
        thunk.sync = &m_sync;
        thunk.thread_count = &m_thread_count;
        thunk.quitting = &m_quitting;
        if(pdPASS==xTaskCreatePinnedToCore(task_entry_thunk,"htcw_thread_pool_thread",stack_word_size,&thunk,priority,&handle,(BaseType_t)cpu)) {
            thread result;
            result.set(handle);
            ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
            return result;
        }
    }
    return thread();
}
#endif
bool thread_pool::queue_work_item(void(*fn)(void*),void* state,bool wait) {
    if(!initialize()) {
        return false;
    }
    return m_sync.post(fn,state,wait);
}
void thread_pool::shutdown(bool abort) {
    if(!initialize()) {
        return;
    }
    int c = m_thread_count;
    if(!abort) {
        if(m_quitting) {
            return;
        }
        m_quitting = true;
        while(c--) {
            m_sync.post(nullptr,nullptr);
        }
        while(m_thread_count) {
            vTaskDelay(1);
        }
        m_quitting=false;
        return;
    } 
    while(c--) {
        m_sync.post([](void*){vTaskDelete(nullptr);});
    }
    m_quitting = false;
}

}