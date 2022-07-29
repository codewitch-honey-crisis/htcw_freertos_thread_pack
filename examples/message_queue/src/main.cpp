#include <Arduino.h>
#include <thread.hpp>
#include <message_queue.hpp>


using namespace freertos;

thread worker;
message_queue<int> queue;
int count;
void setup() {
    Serial.begin(115200);
    queue.initialize();
    // create a thread with
    // extra stack for Serial.print()
    worker = thread::create([](void*){
        int i;
        while(queue.receive(&i)) {
            Serial.print("Queue received ");
            Serial.println(i);
        }
    },nullptr,1,2000);
    worker.start();
    count = 0;
    
}
void loop() {
    // send a message to the waiting worker thread
    queue.send(count++);
    delay(500);
}