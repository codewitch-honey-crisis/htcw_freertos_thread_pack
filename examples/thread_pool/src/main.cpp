#include <Arduino.h>
#include <thread_pool.hpp>

using namespace freertos;

thread_pool pool;

void setup() {
    Serial.begin(115200);
    pool.initialize();
    // Serial.print() takes a lot of stack. Allocate extra stack
    // secondary core
    pool.create_thread_affinity(1-thread::current().affinity(),1,2000);
    // primary core
    pool.create_thread_affinity(thread::current().affinity(),1,2000);    
}
void loop() {
    // dispatch some code on a waiting thread from the pool
    pool.queue_work_item([](void*){ 
        Serial.print("Hello from core ");
        Serial.println(thread::current().affinity());
        delay(500);
    });
}