#ifndef mqtt_client_h
#define mqtt_client_h

#include <PubSubClient.h>
#include "SIM800_driver.h"

void mqtt_init();
void mqtt_publish_power(bool isOn);
void mqtt_publish_battery(float battery_percentage);
void mqtt_sleep_module();

#endif
