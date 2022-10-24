#include <thread.hpp>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace freertos {
void thread::task_entry_thunk(void* state) {
    entry_thunk thunk = *(entry_thunk*)state;
    htcw_helpers::task_notify_give((TaskHandle_t)thunk.calling_handle);
    vTaskSuspend(nullptr);
    thunk.fn(thunk.state);
    vTaskDelete(nullptr);
}
int thread::priority() const {
    if(m_handle==nullptr) {
        return -1;
    }
    return (int)uxTaskPriorityGet((TaskHandle_t)m_handle);
}
bool thread::priority(int value) {
    if(m_handle==nullptr) {
        return false;
    }
    vTaskPrioritySet((TaskHandle_t)m_handle,(UBaseType_t)value);
    return true;
}
#ifdef ESP32
int thread::affinity() const {
    if(nullptr==m_handle) {
        return -1;
    }
    return xTaskGetAffinity((TaskHandle_t)m_handle);
}
#endif
bool thread::start() {
    if(m_handle==nullptr) {
        return false;
    }
    vTaskResume((TaskHandle_t)m_handle);
    return true;
}
bool thread::suspend() {
    if(m_handle==nullptr) {
        return false;
    }
    vTaskSuspend((TaskHandle_t)m_handle);
    return true;
}
bool thread::abort() {
    if(m_handle==nullptr) {
        return false;
    }
    vTaskDelete((TaskHandle_t)m_handle);
    m_handle=nullptr;
    return true;
}
bool thread::running() const {
    if(m_handle==nullptr) {
        return false;
    }
    eTaskState state = eTaskGetState(m_handle);
    return (state==eTaskState::eRunning);
}
thread thread::idle() {
    thread result;
    result.set(xTaskGetIdleTaskHandle());
    return result;
}
#ifdef ESP32
thread thread::idle(int cpu) {
    thread result;
    result.set(xTaskGetIdleTaskHandleForCPU(cpu));
    return result;
}
#endif
thread thread::current() {
    thread result;
    result.set(xTaskGetCurrentTaskHandle());
    return result;
}
#ifdef ESP32
thread thread::current(int cpu) {
    thread result;
    result.set(xTaskGetCurrentTaskHandleForCPU(cpu));
    return result;
}
#endif
void* thread::handle() const {
    return m_handle;
}
thread thread::create(void(*fn)(void*),void* state,int priority,size_t stack_word_size) {
    TaskHandle_t handle = nullptr;
    entry_thunk thunk;
    thunk.calling_handle = xTaskGetCurrentTaskHandle();
    thunk.fn = fn;
    thunk.state = state;
    thread result;
    if(pdPASS==xTaskCreate(task_entry_thunk,"htcw_thread",(uint32_t)stack_word_size,&thunk,priority,&handle)) {
        result.set(handle);
        ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
    }
    return result;
}
#ifdef ESP32
thread thread::create_affinity(int cpu,void(*fn)(void*),void* state,int priority,size_t stack_word_size) {
    TaskHandle_t handle = nullptr;
    entry_thunk thunk;
    thunk.calling_handle = xTaskGetCurrentTaskHandle();
    thunk.fn = fn;
    thunk.state = state;
    thread result;
    if(pdPASS==xTaskCreatePinnedToCore(task_entry_thunk,"htcw_thread",(uint32_t)stack_word_size,&thunk,priority,&handle,cpu)) {
        result.set(handle);
        ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
    }
    return result;
}
#endif
}