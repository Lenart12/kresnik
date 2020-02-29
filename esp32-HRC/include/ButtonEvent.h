#ifndef BUTTONEVENT_H
#define BUTTONEVENT_H

#include <Arduino.h>
#include <defines.h>

class ButtonEvent{
private:
    static uint8_t pressed_queue;
    static uint32_t last_pressed_0;
    static uint32_t last_pressed_1;
    static uint32_t last_pressed_2;
    static uint32_t last_pressed_3;

    static void setPressedQueue(uint8_t button, bool state){
        pressed_queue = (state) ?
            pressed_queue |  (1 << button):
            pressed_queue & ~(1 << button);
    }

    static void button_0_ISR(){ setPressedQueue(0, true); last_pressed_0 = millis(); detachInterrupt(digitalPinToInterrupt(BUTTON_0)); }
    static void button_1_ISR(){ setPressedQueue(1, true); last_pressed_1 = millis(); detachInterrupt(digitalPinToInterrupt(BUTTON_1)); }
    static void button_2_ISR(){ setPressedQueue(2, true); last_pressed_2 = millis(); detachInterrupt(digitalPinToInterrupt(BUTTON_2)); }
    static void button_3_ISR(){ setPressedQueue(3, true); last_pressed_3 = millis(); detachInterrupt(digitalPinToInterrupt(BUTTON_3)); }

    static void beginButtonListners(){
        attachInterrupt(digitalPinToInterrupt(BUTTON_0), button_0_ISR, FALLING);
        attachInterrupt(digitalPinToInterrupt(BUTTON_1), button_1_ISR, FALLING);
        attachInterrupt(digitalPinToInterrupt(BUTTON_2), button_2_ISR, FALLING);
        attachInterrupt(digitalPinToInterrupt(BUTTON_3), button_3_ISR, FALLING);
    }

public:
    ButtonEvent(){};

    static void begin(){
        for(uint8_t i = 0; i < 4; i++) pinMode(BUTTONS[i], INPUT);
        beginButtonListners();
    }

    static void update(){
        uint32_t now = millis();
        if(last_pressed_0 != 0 && now - last_pressed_0 > DEBOUNCE_TIME) {
            attachInterrupt(digitalPinToInterrupt(BUTTON_0), button_0_ISR, FALLING);
            last_pressed_0 = 0;
        }
        if(last_pressed_1 != 0 && now - last_pressed_1 > DEBOUNCE_TIME) {
            attachInterrupt(digitalPinToInterrupt(BUTTON_1), button_1_ISR, FALLING);
            last_pressed_1 = 0;
        }
        if(last_pressed_2 != 0 && now - last_pressed_2 > DEBOUNCE_TIME) {
            attachInterrupt(digitalPinToInterrupt(BUTTON_2), button_2_ISR, FALLING);
            last_pressed_2 = 0;
        }
        if(last_pressed_3 != 0 && now - last_pressed_3 > DEBOUNCE_TIME) {
            attachInterrupt(digitalPinToInterrupt(BUTTON_3), button_3_ISR, FALLING);
            last_pressed_3 = 0;
        }
        
    }

    static bool wasPressed(uint8_t button = 128, bool processEvent = true){
        if(button == 128) return (pressed_queue > 0);
        bool wasPressed = (pressed_queue & (1 << button)) > 0;
        if(wasPressed && processEvent) setPressedQueue(button, false);
        return wasPressed;
    }

    static void clearQueue(){
        pressed_queue = 0x00;
    }
};

#endif