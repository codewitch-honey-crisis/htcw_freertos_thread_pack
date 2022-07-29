#include <message_queue.hpp>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
namespace freertos {
namespace htcw_helpers {
    void* queue_impl::create(size_t queue_size,size_t item_size) {
        return xQueueCreate(queue_size,item_size);
    }
    void queue_impl::destroy(const void* handle) {
        vQueueDelete((QueueHandle_t)handle);
    }
    bool queue_impl::send(const void* handle, const void* message, bool wait) {
        return pdTRUE==xQueueSend((QueueHandle_t)handle,message,wait*portMAX_DELAY);
    }
    bool queue_impl::receive(const void* handle, void* out_message, bool wait) {
        return pdTRUE==xQueueReceive((QueueHandle_t)handle,out_message,wait*portMAX_DELAY);
    }
    bool queue_impl::peek(const void* handle, void* out_message, bool wait) {
        return pdTRUE==xQueuePeek((QueueHandle_t)handle,out_message,wait*portMAX_DELAY);
    }
    
}
}