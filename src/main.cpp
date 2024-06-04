#include <Arduino.h>
#include "WiFi.h" 
#include "driver/adc.h"
#include <esp_wifi.h>
#include <esp_bt.h>
#include "mqtt_client.h"
#include <Wire.h>
#include "time.h"
//#include "ntc.h"

// How many minutes the ESP should sleep
//#define DEEP_SLEEP_TIME 30
#define OPTOCOUPLER1_PIN 13
#define OPTOCOUPLER2_PIN 12
#define BATTERY_PIN 33

uint8_t g_deep_sleep_time = 30;

void goToDeepSleep()
{
  Serial.println("Going to sleep...");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  btStop();
  //SIM900A to sleep
  mqtt_sleep_module();

  adc_power_off();
  esp_wifi_stop();
  esp_bt_controller_disable();

  // Configure the wake up timer
  esp_sleep_enable_timer_wakeup(g_deep_sleep_time * 60L * 1000000L);

  // Go to slee
  esp_deep_sleep_start();
}

void setup() {
  static boolean FirstTime = true;
  // set CPU frequency to 80 MHz
  setCpuFrequencyMhz(80);

  //set optocouplers pins
  //pinMode(OPTOCOUPLER1_PIN, INPUT_PULLUP);
  //pinMode(OPTOCOUPLER2_PIN, INPUT_PULLUP);

  // power on adc
 // adc_power_on();

  //init SIM800 and start MQTT
  mqtt_init();

//  if(FirstTime)
  //{
   // ntcGetTime();
  //}

  //get energy reading
  //uint8_t opto_1 = digitalRead(OPTOCOUPLER1_PIN);
  //uint8_t opto_2 = digitalRead(OPTOCOUPLER2_PIN);
  //publish energy reading
  //if(opto_1 || opto_2)
  //{
    //mqtt_publish_power(true);
  //}
  //else
  //{
    //mqtt_publish_power(false);
  //}
  //get battery value and map it
  //100% -> 3.3v using R1 = 27k and R2 = 100k
  //float batteryLevel = map(analogRead(BATTERY_PIN), 0.0f, 4095.0f, 0, 100);
  //publish battery value
  mqtt_publish_battery(42.42);

 // goToDeepSleep();
}

void loop() {
  // put your main code here, to run repeatedly:
}