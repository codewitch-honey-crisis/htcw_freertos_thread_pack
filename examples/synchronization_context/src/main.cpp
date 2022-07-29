#include <Arduino.h>
#include <thread.hpp>
#include <synchronization_context.hpp>


using namespace freertos;

thread worker;
synchronization_context sync_ctx;

void setup() {
    Serial.begin(115200);
    sync_ctx.initialize();
    // create a thread on the secondary core with
    // extra stack for Serial.print()
    worker = thread::create_affinity(1-thread::current().affinity(), [](void*){
        while(sync_ctx.update(true));
    },nullptr,1,2000);
    worker.start();
    
}
void loop() {
    // dispatch some code on a waiting worker thread
    sync_ctx.post([](void*){
        Serial.print("Hello from core ");
        Serial.println(thread::current().affinity());
    });
    Serial.print("Hello from core ");
    Serial.println(thread::current().affinity());
    delay(500);
}