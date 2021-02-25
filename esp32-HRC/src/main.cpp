#include <Arduino.h>

#include <SPI.h>
#include <EEPROM.h>
#include <time.h>

#include <globals.h>

#include <indev_drivers.h>
#include <display_drivers.h>
#include <logging.h>
#include <tasks.h>
#include <lv_gui.h>

#include <mutex_util.h>

// Setup function
void setup(){
	// Start serial communication
	Serial.begin(115200);
	
	// Start littlevgl
	lv_init();
	lv_log_register_print_cb(logging_cb);

	// Initialize mutexes
	xSemaphoreGive(i2c_mutex);
	xSemaphoreGive(tempature_mutex);
	xSemaphoreGive(timing_mutex);

	for(uint8_t i = 0; i < 4; i++){
		xSemaphoreGive(motor_mutexes[i]);
	}
	LV_LOG_TRACE("Initialized mutexes");

	// Start display and I/O
	display_init();

	button_init();
	touchpad_init();

	// Start tick handler
	tick.attach_ms(LVGL_TICK_PERIOD, lv_tick_handler);
	LV_LOG_TRACE("Created tick handler");


	// Start expanders
	status_expander.begin(PCF8574_STATUS);
	status_expander.writeDDR(0xFF);
	thermo_expander.begin(PCF8574_THERMO);
	thermo_expander.writeDDR(0xFF);
	relay0_expander.begin(PCF8574_RELAY0);
	relay0_expander.writeDDR(0xFF);
	relay1_expander.begin(PCF8574_RELAY1);
	relay1_expander.writeDDR(0xFF);

	status_expander.digitalWrite(Status_led::power, ON);
	i2cLock();
	LV_LOG_TRACE("Started pcf8574 expanders");

	ds2482_0.setStrongPullup();
	ds2482_1.setStrongPullup();

	tempature_1.begin();
	tempature_2.begin();

	tempature_1.setWaitForConversion(false);
	tempature_2.setWaitForConversion(false);
	i2cUnlock();

	tempature_request_task(NULL);
	LV_LOG_TRACE("Started tempature sensors");

	// Load configuration
	EEPROM.begin(sizeof(config));
	#if CLEAN_CONFIG == 0
	EEPROM.get(0, config);
	config.control.load_default();
	EEPROM.put(0, config);
	EEPROM.commit();
	#else
	EEPROM.get(0, config);
	#endif
	LV_LOG_TRACE("Loaded configuration");

	// Start WiFi
	WiFi.begin(config.wifi_login.ssid, config.wifi_login.passwd);
	LV_LOG_TRACE("Started WiFi");


	// Start tasks
	lv_task_create(wifi_status_task, 1000, LV_TASK_PRIO_LOW, NULL);
	LV_LOG_TRACE("Created wifi status task");

	lv_task_create(time_display_task, 1000, LV_TASK_PRIO_LOWEST, NULL);
	LV_LOG_TRACE("Created time display task");

	lv_task_once(lv_task_create(tempature_request_task, 2000, LV_TASK_PRIO_MID, NULL));
	LV_LOG_TRACE("Measured tempature");

	request_temp_task_h = lv_task_create(tempature_request_task, 10000, LV_TASK_PRIO_MID, NULL);
	LV_LOG_INFO("Created tempature request task");

	screensaver_task_h = lv_task_create(screensaver_task, 1000, LV_TASK_PRIO_LOW, NULL);
	LV_LOG_TRACE("Created screensaver task");
	lv_task_create(tempature_shift_task, 60 * 60 * 1000, LV_TASK_PRIO_MID, NULL);
	LV_LOG_TRACE("Created tempature shift task");

	lv_task_create(control_minute_update_task, 60000, LV_TASK_PRIO_HIGH, NULL);
	LV_LOG_TRACE("Created control minute update task");

	config.control.setup();

	lv_main();
	WiFi.scanNetworks(true);

	LV_LOG_INFO("Setup complete");
}

// Loop function
void loop(){
	lv_task_handler();
	delay(1);
}
