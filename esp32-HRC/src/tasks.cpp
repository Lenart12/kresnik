#include <tasks.h>
#include <semaphore_util.h>
#include <lv_gui.h>
#include <EEPROM.h>

void wifi_status_task(lv_task_t *task){
	LV_LOG_TRACE("Started wifi task");
    static wl_status_t last_status = WiFi.status();
    wl_status_t status = WiFi.status();
    if(status != last_status){
        last_status = status;
		switch(status){
			case WL_CONNECTED:{
				LV_LOG_INFO("Got WiFi connection");
				lv_obj_clean(wifi_tab);
				lv_obj_t *txt;
				txt = lv_label_create(wifi_tab, NULL);
				char buf[128] = "Povezan z ";
				strcat(buf, config.wifi_login.ssid);
				strcat(buf, "\nIP: ");
				strcat(buf, WiFi.localIP().toString().c_str());
				lv_label_set_text(txt, buf);	
				lv_obj_align(txt, NULL, LV_ALIGN_CENTER, 0, 0);
				status_expander.digitalWrite(Status_led::wifi, ON);
				lv_label_set_text(status_label, LV_SYMBOL_WIFI);
				lv_obj_align(status_label, NULL, LV_ALIGN_IN_RIGHT_MID, -5, 0);
				goto end;
			}
			case WL_DISCONNECTED:
			case WL_CONNECTION_LOST:
			LV_LOG_INFO("Lost WiFi connection"); break;
			case WL_CONNECT_FAILED:
			LV_LOG_INFO("Failed connecting to WiFi"); break;
			case WL_SCAN_COMPLETED: goto end;
			default: 
			LV_LOG_INFO("WiFi not connected");
		}
		lv_label_set_text(status_label, "");
		status_expander.digitalWrite(Status_led::wifi, OFF);
		WiFi.reconnect();
    }
	end:
	LV_LOG_TRACE("Completed wifi task");
}

void wifi_search_task(lv_task_t *task){
	if(WiFi.scanComplete() == WIFI_SCAN_FAILED){
		lv_obj_clean(wifi_tab);
		lv_obj_t * lab = lv_label_create(wifi_tab, NULL);
		lv_label_set_text(lab, "Napaka pri iskanju omrezji");
		lv_obj_align(lab, NULL, LV_ALIGN_CENTER, 0, 0);
		lv_task_del(task);
	}
	else if(WiFi.scanComplete() > 0){
		lv_obj_clean(wifi_tab);
		lv_obj_t * list = lv_list_create(wifi_tab, NULL);
		lv_obj_set_size(list, lv_obj_get_width_fit(wifi_tab), lv_obj_get_height_fit(wifi_tab));
		lv_obj_align(list, NULL, LV_ALIGN_CENTER, 0, 0);

		lv_obj_t *list_btn;
		for(uint8_t i = 0; i < WiFi.scanComplete(); i++){
			list_btn = lv_list_add_btn(list, NULL, WiFi.SSID(i).c_str());
			lv_obj_set_event_cb(list_btn, wifi_choose);
		}
		lv_task_del(task);
	}
}

void wifi_choose(lv_obj_t * obj, lv_event_t event){
	if(event == LV_EVENT_CLICKED){
		strcpy(config.wifi_login.ssid, lv_list_get_btn_text(obj));

		lv_disp_load_scr(kb_screen);

		lv_obj_t *kb = lv_kb_create(kb_screen, NULL);
		lv_obj_set_size(kb, lv_obj_get_width_fit(kb_screen) - 30, 190);
		lv_obj_align(kb, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
		lv_kb_set_cursor_manage(kb, true);

		lv_obj_t *ta = lv_ta_create(kb_screen, NULL);
		lv_obj_set_size(ta, lv_obj_get_width_fit(kb_screen) - 30, 30);
		lv_obj_align(ta, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);
		lv_ta_set_text(ta, "");

		lv_kb_set_ta(kb, ta);
		lv_obj_set_event_cb(kb, wifi_connect);
	}
}

void wifi_connect(lv_obj_t * obj, lv_event_t event){
	switch(event){
		case LV_EVENT_APPLY:{
			strcpy(config.wifi_login.passwd, lv_ta_get_text(lv_kb_get_ta(obj)));
			WiFi.begin(config.wifi_login.ssid, config.wifi_login.passwd);
			Serial.println(config.wifi_login.ssid);
			Serial.println(config.wifi_login.passwd);
			EEPROM.put(0, config);
			EEPROM.commit();
		}
		case LV_EVENT_CANCEL:{
			lv_obj_clean(kb_screen);
			lv_disp_load_scr(main_screen);
			lv_task_create(wifi_search_task, 100, LV_TASK_PRIO_LOW, NULL);
			break;
		}
		default: lv_kb_def_event_cb(obj, event);
	}
}

void time_display_task(lv_task_t *task){
	if(time_client.update()){
		lv_label_set_text(time_label, time_client.getFormattedTime().c_str());
	}
}

void screensaver_task(lv_task_t *task){
	LV_LOG_TRACE("Started screensaver task");
    uint32_t last_interaction = lv_disp_get_inactive_time(NULL);

	static bool screensaver = false;
	if(screensaver || last_interaction > TFT_SCREEN_OFF){
		if(!screensaver){
			LV_LOG_INFO("Screen turning off");
			screensaver = true;
			digitalWrite(TFT_LED, OFF);
		}
		if(last_interaction < TFT_SCREEN_OFF){
			LV_LOG_INFO("Screen turning on");
			screensaver = false;
			digitalWrite(TFT_LED, ON);
		}
	}
	LV_LOG_TRACE("Completed screensaver task");
}

void tempature_request_task(lv_task_t *task){
	LV_LOG_TRACE("Started tempature request task");
	LV_LOG_TRACE("Taking i2c semaphore");
	xSemaphoreTake(i2c_semaphore, portMAX_DELAY);
	LV_LOG_TRACE("Took i2c semaphore");
	tempature_1.requestTemperatures();
	tempature_2.requestTemperatures();
	xSemaphoreGive(i2c_semaphore);
	LV_LOG_TRACE("Gave i2c semaphore");
	setWithSemaphore(last_conversion_request, (uint32_t)millis(), timing_semaphore);
	LV_LOG_TRACE("Completed tempature request task");
}

void control_update_task(lv_task_t *task){
	LV_LOG_TRACE("Started control update task");
	uint32_t _last_conversion_request = getWithSemaphore(last_conversion_request, timing_semaphore);
	uint32_t _last_control_update = getWithSemaphore(_last_control_update, timing_semaphore);
	uint32_t now = millis();

	if(now - _last_control_update >= CONTROL_INTERVAL &&
	   now - _last_conversion_request >= CONVERSION_REQUEST_DELAY ){
		   setWithSemaphore(_last_control_update, (uint32_t)millis(), timing_semaphore);

		   config.control.update();
	}
	LV_LOG_TRACE("Completed control update task");
}

void motor_control_task(uint8_t motor){
	LV_LOG_TRACE("Started motor control task");
	thermo_expander.digitalWrite(motor*2, OFF);
	thermo_expander.digitalWrite(motor*2+1, OFF);
	LV_LOG_TRACE("Completed motor control task");
}