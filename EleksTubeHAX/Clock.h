#ifndef CLOCK_H
#define CLOCK_H

#include <stdint.h>
#include "GLOBAL_DEFINES.h"
#include <TimeLib.h>

// For NTP
#include <WiFi.h>
#include "NTPClient_AO.h"

#include "StoredConfig.h"
// For TFTs::blanked
#include "TFTs.h"

class Clock {
public:
  Clock() : loop_time(0), local_time(0), time_valid(false), config(NULL) {}
  
  // The global WiFi from WiFi.h must already be .begin()'d before calling Clock::begin()
  void begin(StoredConfig::Config::Clock *config_); 
  void loop();

  // Calls NTPClient::getEpochTime() or RTC::get() as appropriate
  // This has to be static to pass to TimeLib::setSyncProvider.
  static time_t syncProvider();
  uint8_t GetWD(void);

  // Set preferred hour format. true = 12hr, false = 24hr
  void setTwelveHour(bool th)           { config->twelve_hour = th; }
  bool getTwelveHour()                  { return config->twelve_hour; }
  void toggleTwelveHour()               { config->twelve_hour = !config->twelve_hour; }
  // Blanked: 1:23   Not blanked: 01:23
  void setBlankHoursZero(bool bhz)      { config->blank_hours_zero = bhz; }
  bool getBlankHoursZero()              { return config->blank_hours_zero; }
  void toggleBlankHoursZero()           { config->blank_hours_zero = !config->blank_hours_zero; }

  // Internal time is kept in UTC. This affects the displayed time.
  void setTimeZoneOffset(time_t offset) { config->time_zone_offset = offset; }
  time_t getTimeZoneOffset()            { return config->time_zone_offset; }
  void adjustTimeZoneOffset(time_t adj) { config->time_zone_offset += adj; }
  void  setActiveGraphicIdx(int8_t idx) { if(tfts.ClockFacesPresent[idx]) { config->selected_graphic = idx;}}
  int8_t getActiveGraphicIdx()          { return config->selected_graphic; }
  void adjustClockGraphicsIdx(int8_t adj) {
    config->selected_graphic += adj;
    if (config->selected_graphic > tfts.NumberOfClockFaces) { config->selected_graphic = tfts.NumberOfClockFaces; }
    if (config->selected_graphic < 1) { config->selected_graphic = 1; }   
  }
  void setClockGraphicsIdx(int8_t set) {
    config->selected_graphic = set;
    if (config->selected_graphic > tfts.NumberOfClockFaces) { config->selected_graphic = tfts.NumberOfClockFaces; }
    if (config->selected_graphic < 1) { config->selected_graphic = 1; }   
  }

  // Proxy C functions from TimeLib.h
  // I really wish it were a class we could just subclass, but here we are.
  uint16_t getYear()       { return year(local_time); }
  uint8_t getMonth()       { return month(local_time); }
  uint8_t getDay()         { return day(local_time); }
  uint8_t getWeekDay()     { return weekday(local_time); }
  uint8_t getHour()        { return config->twelve_hour ? hourFormat12(local_time) : hour(local_time); }
  uint8_t getHour12()      { return hourFormat12(local_time); }
  uint8_t getHour24()      { return hour(local_time); }
  uint8_t getMinute()      { return minute(local_time); }
  uint8_t getSecond()      { return second(local_time); }
  bool isAm()              { return isAM(local_time); }
  bool isPm()              { return isPM(local_time); }

  // Helper functions for making a clock.
  uint8_t getYearTens()     { return (getYear()/10)%10; }
  uint8_t getYearOnes()     { return getYear()%10; }
  uint8_t getMonthTens()    { return getMonth()/10; }
  uint8_t getMonthOnes()    { return getMonth()%10; }
  uint8_t getDayTens()      { return getDay()/10; }
  uint8_t getDayOnes()      { return getDay()%10; }
  uint8_t getHoursTens();
  uint8_t getHoursOnes()    { return getHour()%10; }
  uint8_t getHours12Tens()  { return getHour12()/10; }
  uint8_t getHours12Ones()  { return getHour12()%10; }
  uint8_t getHours24Tens()  { return getHour24()/10; }
  uint8_t getHours24Ones()  { return getHour24()%10; }
  uint8_t getMinutesTens()  { return getMinute()/10; }
  uint8_t getMinutesOnes()  { return getMinute()%10; }
  uint8_t getSecondsTens()  { return getSecond()/10; }
  uint8_t getSecondsOnes()  { return getSecond()%10; }
  
private:
  time_t loop_time, local_time;
  bool time_valid;
  StoredConfig::Config::Clock *config;

  // Static variables needed for syncProvider()
  static WiFiUDP ntpUDP;
  static NTPClient ntpTimeClient;
  static uint32_t millis_last_ntp;
  const static uint32_t refresh_ntp_every_ms = 3600000; // Get new NTP every hour, use RTC in between.
};



#endif // CLOCK_H
