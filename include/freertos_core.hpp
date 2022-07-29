#pragma once
namespace freertos {
namespace htcw_helpers {
    void* get_task_handle();
    void task_notify_give(const void* handle);
    void task_notify_take();
}   
}