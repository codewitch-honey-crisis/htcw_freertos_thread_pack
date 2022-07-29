#include <freertos_core.hpp>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
namespace freertos {
namespace htcw_helpers {

void* get_task_handle() {
    return xTaskGetCurrentTaskHandle();
}
void task_notify_give(const void* handle) {
    xTaskNotifyGive((TaskHandle_t)handle);
}
void task_notify_take() {
    ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
}
}
}
    