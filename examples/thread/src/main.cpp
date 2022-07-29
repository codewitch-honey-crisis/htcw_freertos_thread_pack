#include <Arduino.h>
#include <thread.hpp>

using namespace freertos;

thread worker;

void setup() {
    Serial.begin(115200);
    // create a thread on the secondary core
    // we increase the stack size because of Serial.println()
    // requiring it
    worker = thread::create_affinity(1-thread::current().affinity(),[](void*){
        while(true) {
            Serial.println("Hello from worker thread");
            delay(500);
        }
    },nullptr,1,2000);
    
    worker.start();
}
void loop() {
    Serial.println("Hello from main thread");
    delay(500);
}