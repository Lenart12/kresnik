#include <Arduino.h>

#include <SPI.h>
#include <EEPROM.h>

#include <globals.h>

#include <indev_drivers.h>
#include <display_drivers.h>
#include <logging.h>
#include <tasks.h>
#include <lv_gui.h>

void setup(){
	Serial.begin(115200);
	lv_init();
	xSemaphoreGive(i2c_semaphore);
	xSemaphoreGive(timing_semaphore);

	lv_log_register_print_cb(logging_cb);

	display_init();

	button_init();
	touchpad_init();

	tick.attach_ms(LVGL_TICK_PERIOD, lv_tick_handler);
	LV_LOG_TRACE("Created tick handler");

	status_expander.begin(PCF8574_STATUS);
	status_expander.writeDDR(0xFF);
	thermo_expander.begin(PCF8574_THERMO);
	thermo_expander.writeDDR(0xFF);
	relay0_expander.begin(PCF8574_RELAY0);
	relay0_expander.writeDDR(0xFF);
	relay1_expander.begin(PCF8574_RELAY1);
	relay1_expander.writeDDR(0xFF);

	status_expander.digitalWrite(Status_led::power, ON);
	LV_LOG_TRACE("Started pcf8574 expanders");

	xSemaphoreTake(i2c_semaphore, portMAX_DELAY);
	ds2482_0.setStrongPullup();
	ds2482_1.setStrongPullup();

	tempature_1.begin();
	tempature_2.begin();

	tempature_1.setWaitForConversion(false);
	tempature_2.setWaitForConversion(false);
	xSemaphoreGive(i2c_semaphore);
	tempature_request_task(NULL);
	LV_LOG_TRACE("Started tempature sensors");

	EEPROM.begin(sizeof(config));
	#ifdef CLEAN_CONFIG
	EEPROM.put(0, config);
	EEPROM.commit();
	#else
	EEPROM.get(0, config);
	#endif
	LV_LOG_TRACE("Loaded configuration");

	WiFi.begin(config.wifi_login.ssid, config.wifi_login.passwd);
	LV_LOG_TRACE("Started WiFi");

	time_client.begin();
	time_client.setTimeOffset(NTP_OFFSET);

	lv_task_create(wifi_status_task, 1000, LV_TASK_PRIO_LOW, NULL);
	LV_LOG_TRACE("Created wifi status task");

	lv_task_create(time_display_task, 1000, LV_TASK_PRIO_LOWEST, NULL);
	LV_LOG_TRACE("Created time display task");

	lv_task_create(screensaver_task, 1000, LV_TASK_PRIO_LOW, NULL);
	LV_LOG_TRACE("Created screensaver task");

	lv_task_create(tempature_request_task, 10000, LV_TASK_PRIO_MID, NULL);
	LV_LOG_TRACE("Created tempature request task");

	lv_task_create(control_update_task, 1000, LV_TASK_PRIO_HIGH, NULL);
	LV_LOG_TRACE("Created control update task");

	LV_LOG_INFO("Setup complete");
	lv_main();
}

void loop(){
	lv_task_handler();
	delay(5);
}

#ifdef OLD_VERSION

uint16_t found_ap;
bool redraw = true;
wl_status_t last_status;
uint32_t last_draw = millis();
enum class Page : uint8_t
{
	home,
	wifi,
	wifi_search,
	wifi_wait_connect
} current_page;

void setup()
{
	Sbegin(115200);
	Sprint("Setup.");

	pinMode(TFT_LED, OUTPUT);
	digitalWrite(TFT_LED, ON);
	display.begin();
	display.setRotation(TFT_ROTATION);
	Sprint('.');
	display.setFont(&FreeSans9pt7b);
	Sprint('.');

	ts.begin();
	Sprint('.');

	status_expander.begin(PCF8574_STATUS);
	status_expander.writeDDR(0xFF);
	Sprint('.');
	thermo_expander.begin(PCF8574_THERMO);
	thermo_expander.writeDDR(0xFF);
	Sprint('.');
	relay0_expander.begin(PCF8574_RELAY0);
	relay0_expander.writeDDR(0xFF);
	Sprint('.');
	relay1_expander.begin(PCF8574_RELAY1);
	relay1_expander.writeDDR(0xFF);
	Sprint('.');

	buttons.begin();
	Sprint('.');

	EEPROM.begin(sizeof(Settings));
	EEPROM.get(0, settings);
	Sprint('.');

	WiFi.begin(settings.wifi_ssid, settings.wifi_pass);
	last_status = WiFi.status();
	Sprint('.');

	timeClient.begin();
	timeClient.setTimeOffset(NTP_OFFSET);

	status_expander.digitalWrite(Status_led::power, ON);
	Sprintln("done!");
	current_page = Page::home;
}

void draw()
{
	last_draw = (current_page == Page::home) ? last_draw : millis();
	switch (current_page)
	{
	case Page::home:
	{
		display.fillScreen(ILI9341_WHITE);
		display.setTextSize(1);
		display.setTextColor(ILI9341_BLACK);
		display.drawRect(10, 10, W - 20, 30, ILI9341_BLACK);
		if (WiFi.status() == WL_CONNECTED)
		{
			printAlignText(display, WiFi.localIP().toString(), Align::left, Align::center, 15, 10, W - 30, 30);
			printAlignText(display, timeClient.getFormattedTime().substring(0, 5), Align::right, Align::center, 15, 10, W - 30, 30);
		}
		else
		{
			printAlignText(display, "WiFi not connected", Align::center, Align::center, 10, 10, W - 20, 30);
		}
		display.drawRect(10, 50, W - 20, 20, ILI9341_BLACK);
		printAlignText(display, "WiFi nastavitve", Align::center, Align::center, 10, 50, W - 20, 20);
		break;
	}
	case Page::wifi:
	{
		display.setTextColor(ILI9341_BLACK);

		display.fillScreen(ILI9341_WHITE);
		display.drawRect(10, 10, W - 20, H - 20, ILI9341_BLACK);

		for (uint8_t i = 0; i < found_ap; i++)
		{
			if (WiFi.SSID(i) == (String)settings.wifi_ssid)
			{
				display.fillRect(15, 15 + i * 25, W - 30, 20, ILI9341_BLACK);
				display.setTextColor(ILI9341_WHITE);
				printAlignText(display, WiFi.SSID(i), Align::center, Align::center, 15, 15 + i * 25, W - 30, 20);
				display.setTextColor(ILI9341_BLACK);
			}
			else
			{
				display.drawRect(15, 15 + i * 25, W - 30, 20, ILI9341_BLACK);
				printAlignText(display, WiFi.SSID(i), Align::center, Align::center, 15, 15 + i * 25, W - 30, 20);
			}
		}

		display.drawRect(W - 100, H - 30, 85, 15, ILI9341_BLACK);
		printAlignText(display, "Nazaj", Align::center, Align::center, W - 100, H - 30, 85, 15);
		break;
	}
	case Page::wifi_search:
	{
		display.setTextColor(ILI9341_BLACK);
		display.fillScreen(ILI9341_WHITE);
		display.drawRect(10, 10, W - 20, H - 20, ILI9341_BLACK);

		printAlignText(display, "Iskanje omrezji...", Align::center, Align::center, 0, 0, W, H);
		break;
	}
	case Page::wifi_wait_connect:
	{
		display.setTextColor(ILI9341_BLACK);
		display.fillScreen(ILI9341_WHITE);
		display.drawRect(10, 10, W - 20, H - 20, ILI9341_BLACK);

		printAlignText(display, "Povezava z", Align::center, Align::center, 0, 0, W, H);
		break;
	}
	}
}

bool update()
{
	buttons.update();
	timeClient.update();
	settings.control.update();

	wl_status_t current_status = WiFi.status();
	bool status_changed = false;
	if (last_status != current_status)
	{
		status_expander.digitalWrite(Status_led::wifi, (current_status == WL_CONNECTED) ? ON : OFF);
		last_status = current_status;
		status_changed = true;
	}

	static bool screen_off = false;
	if (millis() - last_draw > TFT_SCREEN_OFF || screen_off)
	{
		if (!screen_off)
		{
			screen_off = true;
			digitalWrite(TFT_LED, OFF);
		}
		if (ts.touched() || buttons.wasPressed())
		{
			buttons.clearQueue();
			screen_off = false;
			digitalWrite(TFT_LED, ON);
			delay(500);
			return true;
		}
		return false;
	}

	switch (current_page)
	{
	case Page::home:
	{
		if (ts.touched())
		{
			TS_Point p;
			while (ts.touched())
				p = ts.getPoint();
			p.x = map(p.x, 240, 3820, 0, W);
			p.y = map(p.y, 430, 3840, 0, H);
			Sprint(p.x);
			Sprint(' ');
			Sprintln(p.y);
			if (p.x >= 10 && p.x <= W - 10 && p.y >= 50 && p.y <= 70)
			{
				current_page = Page::wifi_search;
				return true;
			}
		}

		if (current_status == WL_CONNECTED)
		{
			static uint8_t last_minute = timeClient.getMinutes();
			uint8_t current_minute = timeClient.getMinutes();
			if (last_minute != current_minute)
			{
				last_minute = current_minute;
				return true;
			}
		}

		return status_changed;
	}
	case Page::wifi:
	{
		if (ts.touched())
		{
			TS_Point p;
			while (ts.touched())
				p = ts.getPoint();

			p.x = map(p.x, 240, 3820, 0, W);
			p.y = map(p.y, 430, 3840, 0, H);

			Sprint(p.x);
			Sprint(' ');
			Sprintln(p.y);

			if (p.x >= W - 100 && p.x <= W - 15 && p.y >= H - 30 && p.y <= H - 15)
			{
				current_page = Page::home;
				return true;
			}
			if (p.x >= 15 && p.x <= W - 15)
			{
				uint8_t i = (p.y - 15) / 25;
				if (i < found_ap)
				{
					Sprintln(WiFi.SSID(i));

					String ssid = WiFi.SSID(i);
					String pwd = keyboard.input("Geslo omrezja");

					strcpy(settings.wifi_ssid, ssid.c_str());
					strcpy(settings.wifi_pass, pwd.c_str());

					WiFi.begin(settings.wifi_ssid, settings.wifi_pass);

					EEPROM.put(0, settings);
					EEPROM.commit();

					current_page = Page::wifi_wait_connect;
					return true;
				}
			}
			return false;
		}
		return false;
	}
	case Page::wifi_search:
	{
		found_ap = WiFi.scanNetworks();
		current_page = Page::wifi;
		delay(500);
		return true;
	}
	case Page::wifi_wait_connect:
	{
		static uint8_t timeout = 128;
		last_status = WiFi.status();
		if (last_status == WL_CONNECTED)
		{
			current_page = Page::home;
			return true;
		}
		timeout--;
		delay(100);
		if (timeout == 0)
		{
			current_page = Page::home;
			timeout = 100;
			return true;
		}
		return false;
	}
	default:
		return false;
	}
}

void loop()
{
	if (redraw)
	{
		draw();
	}
	redraw = update();
}
#endif
