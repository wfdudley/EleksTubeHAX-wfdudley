/*
 * Author: Aljaz Ogrin
 * Project: Alternative firmware for EleksTube IPS clock
 * Hardware: ESP32
 * File description: Global configuration for the complete project
 */
 
#ifndef GLOBAL_DEFINES_H_
#define GLOBAL_DEFINES_H_

#include <stdint.h>
#include <Arduino.h>
#include "USER_DEFINES.h"
#include "HARDWARE_DEFINES.h"
#include "secrets.h"

// ************ General config *********************

//#define DEBUG_OUTPUT

#define DEVICE_NAME       "IPS-clock"
#define FIRMWARE_VERSION  "dudley IPS clock v0.8"
#define SAVED_CONFIG_NAMESPACE  "configs"
#define USE_CLK_FILES	// select between .CLK and .BMP images
#define NIGHT_TIME   0	// dim displays at 12 pm 
#define DAY_TIME     7	// full brightness after 7 am

// ************ WiFi config *********************
#define ESP_WPS_MODE      WPS_TYPE_PBC  // push-button
#define ESP_MANUFACTURER  "ESPRESSIF"
#define ESP_MODEL_NUMBER  "ESP32"
#define ESP_MODEL_NAME    "IPS clock"

#define WIFI_CONNECT_TIMEOUT_SEC  20
#define WIFI_RETRY_CONNECTION_SEC  15

#define GEOLOCATION_ENABLED    // enable after creating an account and copying Geolocation API below:

// ************ MQTT config *********************

#define MQTT_ENABLED

#define MQTT_CLIENT "EleksTubeIPS"		// "base" topic
#define MQTT_RECONNECT_WAIT_SEC  30  // how long to wait between retries to connect to broker
#define MQTT_REPORT_STATUS_EVERY_SEC  71 // How often report status to MQTT Broker
/* MQTT messages are:
  topic/report/signal		integer = WiFi RSSI (normally < 0)
  topic/report/network		WiFi SSID
  topic/report/online		true/false
  topic/report/firmware		FIRMWARE_VERSION
  topic/report/ip		<ipv4 address>
  topic/report/powerState	ON/OFF turns display on/off
  topic/report/setpoint		integer font number 0-9 -> 5 to 50 by 5's ??
 */

// ************ Night time operation *********************
#define BACKLIGHT_DIMMED_INTENSITY  1  // 0..7
#define TFT_DIMMED_INTENSITY  20    // 0..255

// ************ Hardware definitions *********************

// Common indexing scheme, used to identify the digit
#define SECONDS_ONES (0)
#define SECONDS_TENS (1)
#define MINUTES_ONES (2)
#define MINUTES_TENS (3)
#define HOURS_ONES   (4)
#define HOURS_TENS   (5)
#define NUM_DIGITS   (6)

#define SECONDS_ONES_MAP (0x01 << SECONDS_ONES)
#define SECONDS_TENS_MAP (0x01 << SECONDS_TENS)
#define MINUTES_ONES_MAP (0x01 << MINUTES_ONES)
#define MINUTES_TENS_MAP (0x01 << MINUTES_TENS)
#define HOURS_ONES_MAP   (0x01 << HOURS_ONES)
#define HOURS_TENS_MAP   (0x01 << HOURS_TENS)

#endif /* GLOBAL_DEFINES_H_ */
