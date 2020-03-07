#include <Arduino.h>

#include <defines.h>
#include <util.h>
#include <ButtonEvent.h>
#include <KeyboardInput.h>
#include <settings.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#include <Wire.h>
#include <PCF8574_WDDR.h>

#include <SPI.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <Fonts/FreeSans9pt7b.h>

#include <EEPROM.h>

#include <WiFi.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

PCF8574_WDDR status_expander;
PCF8574_WDDR thermo_expander;
PCF8574_WDDR relay0_expander;
PCF8574_WDDR relay1_expander;

Adafruit_ILI9341 display(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen ts(T_CS);

ButtonEvent buttons;
Settings settings;
KeyboardInput keyboard(&display, &ts, &buttons);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

OneWire ds2482_0(DS2482_0);
OneWire ds2482_1(DS2482_1);

DallasTemperature tempature_1(&ds2482_0);
DallasTemperature tempature_2(&ds2482_1);

uint16_t found_ap;
bool redraw = true;
wl_status_t last_status;
uint32_t last_draw = millis();
enum class Page : uint8_t{
	home,
	wifi,
	wifi_search,
	wifi_wait_connect
} current_page;

void setup(){
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

void draw(){
	last_draw = (current_page == Page::home) ? last_draw : millis();
	switch (current_page){
		case Page::home:{
			display.fillScreen(ILI9341_WHITE);
			display.setTextSize(1);
			display.setTextColor(ILI9341_BLACK);
			display.drawRect(10, 10, W-20, 30, ILI9341_BLACK);
			if(WiFi.status() == WL_CONNECTED){
				printAlignText(display, WiFi.localIP().toString(), Align::left, Align::center, 15, 10, W-30, 30);
				printAlignText(display, timeClient.getFormattedTime().substring(0, 5), Align::right, Align::center, 15, 10, W-30, 30);
			}
			else{
				printAlignText(display, "WiFi not connected", Align::center, Align::center, 10, 10, W-20, 30);
			}
			display.drawRect(10, 50, W-20, 20, ILI9341_BLACK);
			printAlignText(display, "WiFi nastavitve", Align::center, Align::center, 10, 50, W-20, 20);
			break;
		}
		case Page::wifi:{
			display.setTextColor(ILI9341_BLACK);


			display.fillScreen(ILI9341_WHITE);
			display.drawRect(10, 10, W-20, H-20, ILI9341_BLACK);

			for(uint8_t i = 0; i < found_ap; i++){
				if(WiFi.SSID(i) == (String)settings.wifi_ssid){
					display.fillRect(15, 15 + i*25, W-30, 20, ILI9341_BLACK);
					display.setTextColor(ILI9341_WHITE);
					printAlignText(display, WiFi.SSID(i), Align::center, Align::center, 15, 15 + i*25, W-30, 20);
					display.setTextColor(ILI9341_BLACK);
				}
				else{
					display.drawRect(15, 15 + i*25, W-30, 20, ILI9341_BLACK);
					printAlignText(display, WiFi.SSID(i), Align::center, Align::center, 15, 15 + i*25, W-30, 20);
				}
			}

			display.drawRect(W-100, H-30, 85, 15, ILI9341_BLACK);
			printAlignText(display, "Nazaj", Align::center, Align::center, W-100, H-30, 85, 15);
			break;
		}
		case Page::wifi_search:{
			display.setTextColor(ILI9341_BLACK);
			display.fillScreen(ILI9341_WHITE);
			display.drawRect(10, 10, W-20, H-20, ILI9341_BLACK);

			printAlignText(display, "Iskanje omrezji...", Align::center, Align::center, 0, 0, W, H);
			break;
		}
		case Page::wifi_wait_connect:{
			display.setTextColor(ILI9341_BLACK);
			display.fillScreen(ILI9341_WHITE);
			display.drawRect(10, 10, W-20, H-20, ILI9341_BLACK);

			printAlignText(display, "Povezava z", Align::center, Align::center, 0, 0, W, H);
			break;
		}
	}
}

bool update(){
	buttons.update();
	timeClient.update();
	settings.control.update();

	wl_status_t current_status = WiFi.status();
	bool status_changed = false;
	if(last_status != current_status){
		status_expander.digitalWrite(Status_led::wifi, (current_status == WL_CONNECTED) ? ON : OFF);
		last_status = current_status;
		status_changed = true;
	}

	static bool screen_off = false;
	if(millis() - last_draw > TFT_SCREEN_OFF || screen_off){
		if(!screen_off){
			screen_off = true;
			digitalWrite(TFT_LED, OFF);
		}
		if(ts.touched() || buttons.wasPressed()){
			buttons.clearQueue();
			screen_off = false;
			digitalWrite(TFT_LED, ON);
			delay(500);
			return true;
		}
		return false;
	} 
	
	switch (current_page){
		case Page::home:{
			if(ts.touched()){
				TS_Point p;
				while(ts.touched()) p = ts.getPoint();
				p.x = map(p.x, 240, 3820, 0, W);
				p.y = map(p.y, 430, 3840, 0, H);
				Sprint(p.x); Sprint(' '); Sprintln(p.y);
				if(p.x >= 10 && p.x <= W-10 && p.y >= 50 && p.y <= 70){
					current_page = Page::wifi_search;
					return true;
				}
			}
			
			if(current_status == WL_CONNECTED){
				static uint8_t last_minute = timeClient.getMinutes();
				uint8_t current_minute = timeClient.getMinutes();
				if(last_minute != current_minute){
					last_minute = current_minute;
					return true;
				}
			}
			
			return status_changed;
		}
		case Page::wifi:{
			if(ts.touched()){
				TS_Point p;
				while(ts.touched()) p = ts.getPoint();

				p.x = map(p.x, 240, 3820, 0, W);
				p.y = map(p.y, 430, 3840, 0, H);

				Sprint(p.x); Sprint(' '); Sprintln(p.y);

				if(p.x >= W-100 && p.x <= W-15 && p.y >= H-30 && p.y <= H-15){
					current_page = Page::home;
					return true;
				}
				if(p.x >= 15 && p.x <= W-15){
					uint8_t i = (p.y-15) / 25;
					if(i < found_ap){
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
		case Page::wifi_search: {
			found_ap = WiFi.scanNetworks();
			current_page = Page::wifi;
			delay(500);
			return true;
		}
		case Page::wifi_wait_connect: {
			static uint8_t timeout = 128;
			last_status = WiFi.status();
			if(last_status == WL_CONNECTED){
				current_page = Page::home;
				return true;
			}
			timeout--;
			delay(100);
			if(timeout == 0){
				current_page = Page::home;
				timeout = 100;
				return true;
			}
			return false;
		}
		default: return false;
	}
}

void loop(){
	if (redraw){
		draw();
	}
	redraw = update();
}