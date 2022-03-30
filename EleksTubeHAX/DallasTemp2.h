#ifndef DALLAS_H_
#define DALLAS_H_

#include "GLOBAL_DEFINES.h"
#ifdef ONE_WIRE_BUS_PIN

#include "mqtt_client_ips.h"

#include <OneWire.h>
#include <DallasTemperature.h>

// Setup a oneWire instance to communicate with OneWire devices (DS18B20)
OneWire oneWire(ONE_WIRE_BUS_PIN);

// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);




void GetTemperatureDS18B20(){
  Serial.print("Reading temperature... ");
  sensors.requestTemperatures();  // all sensors on the bus
  MqttStatusTemperature = sensors.getTempCByIndex(0); // only one sensor (search ROM and read temp)

  // sprintf(MqttStatusTemperatureTxt, "%d", (int)MqttStatusTemperature);  


// char* tempToAscii(float temp) // convert long to type char and store in variable array ascii
// https://forum.arduino.cc/t/printing-a-double-variable/44327/17
// http://www.nongnu.org/avr-libc/user-manual/group__avr__stdlib.html#gaa571de9e773dde59b0550a5ca4bd2f00
    int frac;
    int rnd;

    rnd  = (int)(MqttStatusTemperature*1000)%10;
    frac = (int)(MqttStatusTemperature*100)%100;  //get three numbers to the right of the deciaml point.
    if (rnd>=5) frac=frac+1;
    itoa((int)MqttStatusTemperature, MqttStatusTemperatureTxt, 10);
    strcat(MqttStatusTemperatureTxt,".");

    if (frac < 10) {itoa(0,&MqttStatusTemperatureTxt[strlen(MqttStatusTemperatureTxt)], 10); }   // if fract < 10 should print .0 fract ie if fract=6 print .06

    itoa(frac, &MqttStatusTemperatureTxt[strlen(MqttStatusTemperatureTxt)], 10); //put the frac after the deciaml
    
//    sprintf(MqttStatusTemperatureTxt, "%d.%d", intPart, fracPart);


  TemperatureUpdated = true;
 
  //Serial.print(MqttStatusTemperature); // print float directly, for testing purposes only
  //Serial.print("  /  ");
  Serial.print(MqttStatusTemperatureTxt);
  Serial.println(" C");  
}

#endif // sensor defined

#endif // DALLAS_H_
