#pragma once
#include <stddef.h>
namespace freertos {
namespace htcw_helpers {
    struct queue_impl {
        static void* create(size_t queue_size,size_t item_size);
        static void destroy(const void* handle);
        static bool send(const void* handle,const void* message, bool wait = true);
        static bool receive(const void* handle, void* out_message, bool wait = true);
        static bool peek(const void* handle, void* out_message=nullptr, bool wait = false);
    };
}
// provides a facility for operating a thread safe message passing scheme
template<typename MessageType>
class message_queue final {
public:
    using type = message_queue;
    using message_type = MessageType;
    constexpr static const size_t default_size = 10;
private:
    void* m_handle;
    size_t m_size;
    message_queue(const message_queue& rhs)=delete;
    message_queue& operator=(const message_queue& rhs)=delete;
public:
    // creates a new instance
    inline message_queue() : m_handle(nullptr), m_size(0) {

    }
    // destroys the queue
    ~message_queue() {
        if(m_handle!=nullptr) {
            htcw_helpers::queue_impl::destroy(m_handle);
        }
    }
    // resource stealing overload
    message_queue(message_queue&& rhs) {
        m_handle = rhs.m_handle;
        m_size = rhs.m_size;
        rhs.m_handle = nullptr;
    }
    // resource stealing overload
    message_queue& operator=(message_queue&& rhs) {
        if(m_handle!=nullptr) {
            htcw_helpers::queue_impl::destroy(m_handle);
        }
        m_handle = rhs.m_handle;
        m_size = rhs.m_size;
        rhs.m_handle = nullptr;
        return *this;
    }
    // indicates whether the queue has been initialized
    inline bool initialized() const { return m_handle!=nullptr;}
    // explicity initializes the queue optionally setting it to an indicated size
    bool initialize(size_t size = 0) {
        if(m_handle==nullptr) {
            if(size<1) {
                m_handle = htcw_helpers::queue_impl::create(default_size,sizeof(message_type));
            } else {
                m_handle = htcw_helpers::queue_impl::create(size,sizeof(message_type));
            }
            if(m_handle!=nullptr) {
                m_size = size;
                return true;
            }
            return false;
        } else {
            if(m_size!=size) {
                if(!deinitialize()) {
                    return false;
                }
                m_handle = htcw_helpers::queue_impl::create(size,sizeof(message_type));
                if(m_handle!=nullptr) {
                    m_size = size;
                    return true;
                }   
                return false;
            }
        }
        return true;
    }
    bool deinitialize() {
        if(m_handle!=nullptr) {
            htcw_helpers::queue_impl::destroy(m_handle);
        }
        m_size = 0;
        m_handle=nullptr;
        return true;
    }
    inline size_t size() const { return m_size; }
    bool send(const message_type& message, bool wait = true) {
        if(!initialize()) return false;
        return htcw_helpers::queue_impl::send(m_handle,&message,wait);
    }
    bool receive(message_type* out_message, bool wait = true) {
        if(!initialize()) return false;
        return htcw_helpers::queue_impl::receive(m_handle,out_message,wait);
    }
    bool peek(message_type* out_message, bool wait = false) {
        if(!initialize()) return false;
        return htcw_helpers::queue_impl::peek(m_handle,out_message,wait);
    }
};
}