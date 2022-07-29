#pragma once
#include "freertos_core.hpp"
#include "message_queue.hpp"
namespace freertos {
namespace htcw_helpers {
    // implement std::move to limit dependencies on the STL, which may not be there
    template< class T > struct synchronization_context_remove_reference      { typedef T type; };
    template< class T > struct synchronization_context_remove_reference<T&>  { typedef T type; };
    template< class T > struct synchronization_context_remove_reference<T&&> { typedef T type; };
    template <typename T>
    typename synchronization_context_remove_reference<T>::type&& synchronization_context_move(T&& arg) {
        return static_cast<typename synchronization_context_remove_reference<T>::type&&>(arg);
    }
    
}
class synchronization_context final {
    synchronization_context(const synchronization_context& rhs)=delete;
    synchronization_context& operator=(const synchronization_context& rhs)=delete;
    struct message {
        void *state;
        void (*callback)(void*);
        void* finish_notify_handle;
    };
    message_queue<message> m_queue;
public:
    inline synchronization_context() {

    }
    inline synchronization_context(synchronization_context&& rhs) {
        m_queue = htcw_helpers::synchronization_context_move(rhs.m_queue);
        rhs.m_queue = htcw_helpers::synchronization_context_move(message_queue<message>());
    }
    inline synchronization_context& operator=(synchronization_context&& rhs) {
        m_queue = htcw_helpers::synchronization_context_move(rhs.m_queue);
        rhs.m_queue = htcw_helpers::synchronization_context_move(message_queue<message>());
        return *this;
    }
    inline ~synchronization_context() {

    }
    inline bool initialized() const { return m_queue.initialized(); }
    inline bool initialize(size_t size=0) {
        return m_queue.initialize(size);
    }
    bool post(void fn(void*), void* state = nullptr, bool wait = false) {
        if(!initialize()) {
            return false;
        }
        message msg;
        msg.callback=fn;
        msg.state = state;
        msg.finish_notify_handle = nullptr;
        return m_queue.send(msg,wait);
    }
    bool send(void fn(void*), void* state = nullptr) {
        if(!initialize()) {
            return false;
        }
        message msg;
        msg.callback=fn;
        msg.state = state;
        msg.finish_notify_handle = htcw_helpers::get_task_handle();
        if(!m_queue.send(msg,true)) {
            return false;
        }
        // sleep the task
        htcw_helpers::task_notify_take();
        return true;
    }
    bool update(bool wait=false) {
        if(!initialize()) {
            return false;
        }
        message msg;
        if(!m_queue.receive(&msg,wait)) {
            return wait==false;
        }
        if(msg.callback!=nullptr) {
            msg.callback(msg.state);
            if(nullptr!=msg.finish_notify_handle) {
                htcw_helpers::task_notify_give(msg.finish_notify_handle);
            }
        }
        return true;
    }
};
}