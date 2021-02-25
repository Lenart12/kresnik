#include <indev_drivers.h>

void button_init(){
    for(uint8_t i = 0; i < 4; i++) pinMode(BUTTONS[i], INPUT);

    lv_indev_drv_t button_driver_drv;
    lv_indev_drv_init(&button_driver_drv);
    button_driver_drv.type = LV_INDEV_TYPE_BUTTON;
    button_driver_drv.read_cb = button_read_cb;
    button_driver_drv.feedback_cb = button_feedback_cb;
    lv_indev_t *button_driver =  lv_indev_drv_register(&button_driver_drv);
    lv_indev_set_button_points(button_driver, button_targets);
    LV_LOG_TRACE("Buttons initialized");
}

bool button_read_cb(lv_indev_drv_t *drv, lv_indev_data_t *data){
    static uint8_t last_button = -1;
    for(uint8_t i = 0; i < 4; i++){
        if(digitalRead(BUTTONS[i]) == ON){
            data->state = LV_INDEV_STATE_PR;
            data->btn_id = last_button = i;
            return false;
        }
    }
    data->btn_id = last_button;
    data->state = LV_INDEV_STATE_REL;
    return false;
}

void button_feedback_cb(lv_indev_drv_t *drv, lv_event_t event){
    if(event == LV_EVENT_CLICKED){
        LV_LOG_TRACE("Button clicked");
    }
}


void touchpad_init(){
    uint16_t caliData[5];
    memcpy(caliData, TOUCH_CALIBRATION, sizeof(uint16_t) * 5);
    tft.setTouch(caliData);

    lv_indev_drv_t touchpad_driver_drv;

    lv_indev_drv_init(&touchpad_driver_drv);
    touchpad_driver_drv.type = LV_INDEV_TYPE_POINTER;
    touchpad_driver_drv.read_cb = touchpad_read_cb;
    touchpad_driver_drv.feedback_cb = touchpad_feedback_cb;
    lv_indev_drv_register(&touchpad_driver_drv);
    LV_LOG_TRACE("Touchpad initialized");
}

bool touchpad_read_cb(lv_indev_drv_t *drv, lv_indev_data_t *data){
    uint16_t touchX, touchY;
    bool touched = tft.getTouch(&touchX, &touchY, 600);

    if(touchX>W || touchY > H){
      return false;
    }
    data->state = touched ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL; 
    data->point.x = touchX;
    data->point.y = touchY;
    return false;
}

void touchpad_feedback_cb(lv_indev_drv_t *drv, lv_event_t event){
    if(event == LV_EVENT_CLICKED){
        LV_LOG_TRACE("Touchpad clicked");
    }
}