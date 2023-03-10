/*
 * Author: Aljaz Ogrin
 * Project: Alternative firmware for EleksTube IPS clock
 * Original location: https://github.com/aly-fly/EleksTubeHAX
 * Hardware: ESP32
 * Based on: https://github.com/SmittyHalibut/EleksTubeHAX
 */

#include <stdint.h>
#include "GLOBAL_DEFINES.h"
#include "Buttons.h"
#include "Backlights.h"
#include "TFTs.h"
#include "Clock.h"
#include "Menu.h"
#include "StoredConfig.h"
#include "WiFi_WPS.h"
#include "Mqtt_client_ips.h"
#include "debug_files.h"
#include <esp_wifi.h>

Backlights backlights;
Buttons buttons;
TFTs tfts;
Clock uclock;
Menu menu;
StoredConfig stored_config;

bool FullHour = false;
uint8_t hour_old = 255;
uint8_t weekd, month_now, date_now;
unsigned long date_display_timer;
bool first_date_display, first_time_display;
bool UpdateTZandDSTeveryNight = false;

// Helper function, defined below.
void updateClockDisplay(TFTs::show_t show=TFTs::yes);
void updateClockDateDisplay(TFTs::show_t show=TFTs::yes);
void setupMenu();
void EveryFullHour();
static int SUIMODE;	// Serial UI Mode
#define SUI_NIL 0
#define SUISSID 1
#define SUIPASS 2
char cmdbuf[33];	// command buffer
int cmdidx;		// command buffer index

#if DEBUG_FILE_FNS
void my_dir(const char * dirname, uint8_t levels);
#endif

void SerialCommandHelp (void) {
    Serial.println("------------------------");
#if DEBUG_FILE_FNS
    Serial.println("d - file list");
#endif
    Serial.println("sSSID - set ssid to SSID");
    Serial.println("pPASS - set password to PASS");
    Serial.println("w - show wifi credentials & valid flag");
    Serial.println("v - toggle wifi.valid flag");
    Serial.println("r - reset processor");
    Serial.println("? - this help");
    Serial.println("------------------------");
}

extern void WiFiEvent(WiFiEvent_t event, system_event_info_t info);

void SerialCommandProcessor (void) {
static wifi_config_t config;
static wifi_init_config_t *iconfig;
esp_err_t err;
int ch;
  if(Serial.available()) {
    ch = Serial.read();
    switch(SUIMODE) {
	case 0 :
	    switch(ch) {
#if DEBUG_FILE_FNS
		case 'd' :	// "directory"
		    my_dir("/", 1);
		    break;
#endif
		case 's' :	// "set SSID"
		    SUIMODE = SUISSID;
		    memset(cmdbuf, '\0', sizeof(cmdbuf));
		    cmdidx = 0;
		    break;
		case 'w' :	// "show WiFi credentials"
		    err = esp_wifi_get_config(WIFI_IF_STA, &config);
		    if (err == ESP_OK) {
		        Serial.printf("esp_config: SSID = %s, PW = %s\n", (char*) config.sta.ssid, (char*) config.sta.password);
		    } else {
		        Serial.printf("Couldn't get config: %d\n", (int) err);
		    }
		    Serial.printf("stored_config: ssid = %s, ", stored_config.config.wifi.ssid);
		    Serial.printf("PW = %s, ", stored_config.config.wifi.password);
		    Serial.printf("valid = %2x\n", stored_config.config.wifi.WPS_connected);
		    break;
		case 'p' :	// "set password"
		    err = esp_wifi_get_config(WIFI_IF_STA, &config);
		    if (err != ESP_OK) {
		        Serial.printf("Couldn't get config: %d\n", (int) err);
			// err = esp_wifi_init(const wifi_init_config_t *ionfig);
			WiFi.mode(WIFI_STA);
			WiFi.onEvent(WiFiEvent);
			WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);  
			WiFi.setHostname(DEVICE_NAME);  
			WiFi.begin();  // use internally saved data
			err = esp_wifi_get_config(WIFI_IF_STA, &config);
			if (err != ESP_OK) {
			    Serial.printf("Couldn't get config on second try: %d\n", (int) err);
			}
		    }
		    SUIMODE = SUIPASS;
		    memset(cmdbuf, '\0', sizeof(cmdbuf));
		    cmdidx = 0;
		    break;
		case 'v' :	// "toggle wifi.valid flag"
		    stored_config.config.wifi.WPS_connected = (stored_config.config.wifi.WPS_connected == StoredConfig::valid) ? 0x11 : StoredConfig::valid;
		    Serial.printf("valid is %2x\n", stored_config.config.wifi.WPS_connected);
		    stored_config.save();
		    break;
		case 'r' :	// "reset"
		    ESP.restart();
		    break;
		case '?' :	// "help"
		    SerialCommandHelp();
		    break;
	    }
	    break;
	case SUISSID :
	    if(ch == '\r' || ch == '\n') {
		strncpy(stored_config.config.wifi.ssid, cmdbuf, 32);
		Serial.printf("ssid set to %s\n", stored_config.config.wifi.ssid);
		SUIMODE = SUI_NIL;
	    }
	    else {
		cmdbuf[cmdidx] = ch;
		if(cmdidx < sizeof(cmdbuf)) { cmdidx++; }
	    }
	    break;
	case SUIPASS :
	    if(ch == '\r' || ch == '\n') {
	        strncpy(stored_config.config.wifi.password, cmdbuf, 32);
		Serial.printf("password set to %s\n", stored_config.config.wifi.password);
		stored_config.config.wifi.WPS_connected = StoredConfig::valid;
		stored_config.save();
		strcpy((char*) config.sta.ssid, stored_config.config.wifi.ssid);
		strcpy((char*) config.sta.password, stored_config.config.wifi.password);
		err = esp_wifi_set_config(WIFI_IF_STA, &config);
		if (err != ESP_OK) {
		    Serial.printf("Couldn't set config: %d\n", (int)err);
		}
		SUIMODE = SUI_NIL;
	    }
	    else {
		cmdbuf[cmdidx] = ch;
		if(cmdidx < sizeof(cmdbuf)) { cmdidx++; }
	    }
	    break;
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);  // Waiting for serial monitor to catch up.
  Serial.println("");
  Serial.println(FIRMWARE_VERSION);
  Serial.println("In setup().");  
  SUIMODE = 0;

  stored_config.begin();
  stored_config.load();

  backlights.begin(&stored_config.config.backlights);
  buttons.begin();
  menu.begin();

  // Setup TFTs
  tfts.begin();
  tfts.fillScreen(TFT_BLACK);
  tfts.setTextColor(TFT_WHITE, TFT_BLACK);
  tfts.setCursor(0, 0, 2);  // Font 2. 16 pixel high
  tfts.println("setup...");

  // Setup WiFi connection. Must be done before setting up Clock.
  // This is done outside Clock so the network can be used for other things.
  if (stored_config.config.wifi.WPS_connected != StoredConfig::valid) {
      // allow user to enter ssid and password
      Serial.println("Enter wifi credentials:\n");
      SerialCommandHelp();
      while (stored_config.config.wifi.WPS_connected != StoredConfig::valid) {
	  SerialCommandProcessor();
      }
  }
//  WiFiBegin(&stored_config.config.wifi);
  tfts.println("WiFi start");
  WifiBegin();
  
  // wait for a bit before querying NTP
  for (uint8_t ndx=0; ndx < 5; ndx++) {
    tfts.print(">");
    delay(100);
  }
  tfts.println("");

  // Setup the clock.  It needs WiFi to be established already.
  tfts.println("Clock start");
  uclock.begin(&stored_config.config.uclock);

  // Setup MQTT
  tfts.println("MQTT start");
  MqttStart();

#ifdef GEOLOCATION_ENABLED
  tfts.println("Geoloc query");
  if (GetGeoLocationTimeZoneOffset()) {
    tfts.print("TZ: ");
    tfts.println(GeoLocTZoffset);
    uclock.setTimeZoneOffset(GeoLocTZoffset * 3600);
    Serial.print("Saving config...");
    stored_config.save();
    Serial.println(" Done.");
  } else {
    Serial.println("Geolocation failed.");    
    tfts.println("Geo FAILED");
  }
#endif

  tfts.println("Done with setup.");

  // Leave boot up messages on screen for a few seconds.
  for (uint8_t ndx=0; ndx < 10; ndx++) {
    tfts.print(">");
    delay(200);
  }

  tfts.current_graphic = uclock.getActiveGraphicIdx();

  // Start up the clock displays.
  tfts.fillScreen(TFT_BLACK);
  uclock.loop();
  weekd = uclock.getWeekDay();
  updateClockDisplay(TFTs::force);
  Serial.println("Setup finished.");
}

void loop() {
  uint32_t millis_at_top = millis();
  weekd = uclock.getWeekDay();
  date_now = uclock.getDay();
  month_now = uclock.getMonth();
  if(!weekd) weekd = 1;	// if not initialized, make it Sunday
  SerialCommandProcessor();
  // Do all the maintenance work
  WifiReconnect(); // if not connected attempt to reconnect

  MqttStatusPower = tfts.isEnabled();
  MqttStatusState = (uclock.getActiveGraphicIdx()+1) * 5;   // 10 
  MqttLoopFrequently();
  if (MqttCommandPowerReceived) {
    MqttCommandPowerReceived = false;
    if (MqttCommandPower) {
#ifndef HARDWARE_SI_HAI_CLOCK
      if (!tfts.isEnabled()) {
        tfts.reinit();  // reinit (original EleksTube HW: after a few hours in OFF state the displays do not wake up properly)
        updateClockDisplay(TFTs::force);
      }
#endif
      tfts.enableAllDisplays();
      backlights.PowerOn();
    } else {
      tfts.disableAllDisplays();
      backlights.PowerOff();
    }
  }

  if (MqttCommandStateReceived) {
    MqttCommandStateReceived = false;
    randomSeed(millis());
    uint8_t idx;
    if (MqttCommandState >= 90)
      { idx = random(1, tfts.NumberOfClockFaces+1); } else
      { idx = (MqttCommandState / 5) -1; }  // 10..40 -> graphic 1..6
    Serial.print("Graphic change request from MQTT; command: ");
    Serial.print(MqttCommandState);
    Serial.print(", index: ");
    Serial.println(idx);
    uclock.setClockGraphicsIdx(idx);  
    tfts.current_graphic = uclock.getActiveGraphicIdx();
    updateClockDisplay(TFTs::force);   // redraw everything
    /* do not save to flash everytime mqtt changes; can be frequent
    Serial.print("Saving config...");
    stored_config.save();
    Serial.println(" Done.");
    */
  }

  buttons.loop();

  // Power button: If in menu, exit menu. Else turn off displays and backlight.
  if (buttons.power.isDownEdge() && (menu.getState() == Menu::idle)) {
    tfts.toggleAllDisplays();
#ifndef HARDWARE_SI_HAI_CLOCK
    if (tfts.isEnabled()) {
      tfts.reinit();  // reinit (original EleksTube HW: after a few hours in OFF state the displays do not wake up properly)
      updateClockDisplay(TFTs::force);
    }
#endif
    backlights.togglePower();
  }
 
  menu.loop(buttons);  // Must be called after buttons.loop()
  backlights.loop();
  uclock.loop();

  EveryFullHour(); // night or daytime

  // Update the clock.
  
  if(date_display_timer > millis()) {
    if(first_date_display) {
      updateClockDateDisplay(TFTs::force);
      first_date_display = 0;
    }
    else {
      updateClockDateDisplay();
      first_time_display = 1;
    }
  }
  else {
    if(first_time_display) {
      updateClockDisplay(TFTs::force);
      first_time_display = 0;
    }
    else {
      updateClockDisplay();
      first_date_display = 1;
    }
  }
  
  // Menu
  if (menu.stateChanged() && tfts.isEnabled()) {
    Menu::states menu_state = menu.getState();
    int8_t menu_change = menu.getChange();

    if (menu_state == Menu::idle) {
      // We just changed into idle, so force redraw everything, and save the config.
      updateClockDisplay(TFTs::force);
      Serial.print("Saving config...");
      stored_config.save();
      Serial.println(" Done.");
    }
    else {
      // Backlight Pattern
      if (menu_state == Menu::backlight_pattern) {
        if (menu_change != 0) {
          backlights.setNextPattern(menu_change);
        }

        setupMenu();
        tfts.println("Pattern:");
        tfts.println(backlights.getPatternStr());
      }
      // Backlight Color
      else if (menu_state == Menu::pattern_color) {
        if (menu_change != 0) {
          backlights.adjustColorPhase(menu_change*16);
        }
        setupMenu();
        tfts.println("Color:");
        tfts.printf("%06X\n", backlights.getColor()); 
      }
      // Backlight Intensity
      else if (menu_state == Menu::backlight_intensity) {
        if (menu_change != 0) {
          backlights.adjustIntensity(menu_change);
        }
        setupMenu();
        tfts.println("Intensity:");
        tfts.println(backlights.getIntensity());
      }
      // 12 Hour or 24 Hour mode?
      else if (menu_state == Menu::twelve_hour) {
        if (menu_change != 0) {
          uclock.toggleTwelveHour();
          tfts.setDigit(HOURS_TENS, uclock.getHoursTens(), TFTs::force);
          tfts.setDigit(HOURS_ONES, uclock.getHoursOnes(), TFTs::force);
        }
        
        setupMenu();
        tfts.println("12 Hour?");
        tfts.println(uclock.getTwelveHour() ? "12 hour" : "24 hour"); 
      }
      // Blank leading zeros on the hours?
      else if (menu_state == Menu::blank_hours_zero) {
        if (menu_change != 0) {
          uclock.toggleBlankHoursZero();
          tfts.setDigit(HOURS_TENS, uclock.getHoursTens(), TFTs::force);
        }
        
        setupMenu();
        tfts.println("Blank zero?");
        tfts.println(uclock.getBlankHoursZero() ? "yes" : "no");
      }
      // UTC Offset, hours
      else if (menu_state == Menu::utc_offset_hour) {
        if (menu_change != 0) {
          uclock.adjustTimeZoneOffset(menu_change * 3600);
          tfts.setDigit(HOURS_TENS, uclock.getHoursTens(), TFTs::force);
          tfts.setDigit(HOURS_ONES, uclock.getHoursOnes(), TFTs::force);
        }

        setupMenu();
        tfts.println("UTC Offset");
        tfts.println(" +/- Hour");
        time_t offset = uclock.getTimeZoneOffset();
        int8_t offset_hour = offset/3600;
        int8_t offset_min = (offset%3600)/60;
        tfts.printf("%d:%02d\n", offset_hour, offset_min);
      }
      // UTC Offset, 15 minutes
      else if (menu_state == Menu::utc_offset_15m) {
        if (menu_change != 0) {
          uclock.adjustTimeZoneOffset(menu_change * 900);
          tfts.setDigit(HOURS_TENS, uclock.getHoursTens(), TFTs::force);
          tfts.setDigit(HOURS_ONES, uclock.getHoursOnes(), TFTs::force);
          tfts.setDigit(MINUTES_TENS, uclock.getMinutesTens(), TFTs::force);
          tfts.setDigit(MINUTES_ONES, uclock.getMinutesOnes(), TFTs::force);
        }

        setupMenu();
        tfts.println("UTC Offset");
        tfts.println(" +/- 15m");
        time_t offset = uclock.getTimeZoneOffset();
        int8_t offset_hour = offset/3600;
        int8_t offset_min = (offset%3600)/60;
        tfts.printf("%d:%02d\n", offset_hour, offset_min);
      }
      // select clock "font"
      else if (menu_state == Menu::selected_graphic) {
        if (menu_change != 0) {
          uclock.adjustClockGraphicsIdx(menu_change);
          tfts.current_graphic = uclock.getActiveGraphicIdx();
          updateClockDisplay(TFTs::force);   // redraw everything
        }

        setupMenu();
        tfts.println("Selected");
        tfts.println(" graphic:");
        tfts.printf("    %d\n", uclock.getActiveGraphicIdx());
      }
#ifdef WIFI_USE_WPS   ////  WPS code
      // connect to WiFi using wps pushbutton mode
      else if (menu_state == Menu::start_wps) {
        if (menu_change != 0) { // button was pressed
          if (menu_change < 0) { // left button
            Serial.println("WiFi WPS start request");
            tfts.clear();
            tfts.fillScreen(TFT_BLACK);
            tfts.setTextColor(TFT_WHITE, TFT_BLACK);
            tfts.setCursor(0, 0, 4);  // Font 4. 26 pixel high
            WiFiStartWps();
          }
        }
        
        setupMenu();
        tfts.println("Connect to WiFi?");
        tfts.println("Left=WPS");
      }
#endif
    }
  }

  uint32_t time_in_loop = millis() - millis_at_top;
  if (time_in_loop < 20) {
    // we have free time, spend it for loading next image into buffer
    tfts.LoadNextImage();

    // we still have extra time
    time_in_loop = millis() - millis_at_top;
    if (time_in_loop < 20) {
      MqttLoopInFreeTime();
      if (TemperatureUpdated) {
        tfts.setDigit(HOURS_ONES, uclock.getHoursOnes(), TFTs::force);  // show latest clock digit and temperature readout together
        TemperatureUpdated = false;
      }
      
      // run once a day (= 744 times per month which is below the limit of 5k for free account)
      if (UpdateTZandDSTeveryNight) { // Daylight savings time changes at 3 in the morning
        if (GetGeoLocationTimeZoneOffset()) {
          uclock.setTimeZoneOffset(GeoLocTZoffset * 3600);
        }
      UpdateTZandDSTeveryNight = false;
      }  
      // Sleep for up to 20ms, less if we've spent time doing stuff above.
      time_in_loop = millis() - millis_at_top;
      if (time_in_loop < 20) {
        delay(20 - time_in_loop);
      }
    }
  }
#ifdef DEBUG_OUTPUT
  if (time_in_loop <= 1) Serial.print(".");
  else Serial.println(time_in_loop);
#endif
}

void setupMenu() {
  tfts.chip_select.setHoursTens();
  tfts.setTextColor(TFT_WHITE, TFT_BLACK);
  tfts.fillRect(0, 120, 135, 120, TFT_BLACK);
  tfts.setCursor(0, 124, 4);  // Font 4. 26 pixel high
}

bool isNightTime(uint8_t current_hour) {
    if (DAY_TIME < NIGHT_TIME)
      // "Night" spans across midnight
      return (current_hour >= NIGHT_TIME) || (current_hour < DAY_TIME);
    else
      // "Night" starts after midnight, entirely contained within the day
      return (NIGHT_TIME <= current_hour) && (current_hour < DAY_TIME);  
}

void EveryFullHour() {
  // dim the clock at night
  uint8_t current_hour = uclock.getHour24();
  FullHour = current_hour != hour_old;
  if (FullHour) {
  Serial.print("current hour = ");
  Serial.println(current_hour);
    if (isNightTime(current_hour)) {
      Serial.println("Setting night mode (dimmed)");
      tfts.dimming = TFT_DIMMED_INTENSITY;
      tfts.InvalidateImageInBuffer(); // invalidate; reload images with new dimming value
      backlights.dimming = true;
      if (menu.getState() == Menu::idle) { // otherwise erases the menu
        updateClockDisplay(TFTs::force); // update all
      }
    } else {
      Serial.println("Setting daytime mode (normal brightness)");
      tfts.dimming = 255; // 0..255
      tfts.InvalidateImageInBuffer(); // invalidate; reload images with new dimming value
      backlights.dimming = false;
      if (menu.getState() == Menu::idle) { // otherwise erases the menu
        updateClockDisplay(TFTs::force); // update all
      }
    }
    UpdateTZandDSTeveryNight = (uclock.getHour24() == 3); // Daylight savings time changes at 3 in the morning
    hour_old = current_hour;
  }   
}


void updateClockDisplay(TFTs::show_t show) {
  // refresh starting on seconds
  tfts.setDigit(SECONDS_ONES, uclock.getSecondsOnes(), show);
  tfts.setDigit(SECONDS_TENS, uclock.getSecondsTens(), show);
  tfts.setDigit(MINUTES_ONES, uclock.getMinutesOnes(), show);
  tfts.setDigit(MINUTES_TENS, uclock.getMinutesTens(), show);
  tfts.setDigit(HOURS_ONES, uclock.getHoursOnes(), show);
  tfts.setDigit(HOURS_TENS, uclock.getHoursTens(), show);
}

void updateClockDateDisplay(TFTs::show_t show) {
  // refresh starting on seconds
  tfts.setDigit(SECONDS_ONES, uclock.getDayOnes(), show);
  tfts.setDigit(SECONDS_TENS, uclock.getDayTens(), show);
  tfts.setDigit(MINUTES_ONES, uclock.getMonthOnes(), show);
  tfts.setDigit(MINUTES_TENS, uclock.getMonthTens(), show);
  tfts.setDigit(HOURS_ONES, uclock.getYearOnes(), show);
  tfts.setDigit(HOURS_TENS, uclock.getYearTens(), show);
}
