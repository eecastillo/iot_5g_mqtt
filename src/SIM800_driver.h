#ifndef SIM800_driver_h
#define SIM800_driver_h

// SIM module used
#define TINY_GSM_MODEM_SIM800
#define TINY_GSM_RX_BUFFER  1024    //Set RX buffer to 1Kb

// Serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Serial for AT commands (to SIM800 module)
#define SerialAT Serial1

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon

// Range to attempt to autobaud
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 115200

// Define how you're planning to connect to the internet
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false

// set GSM PIN, if any
#define GSM_PIN ""
 
//TTGO T-Call pins
#define MODEM_RST       5
#define MODEM_PWKEY     4
#define MODEM_POWER_ON  23
#define MODEM_TX        27
#define MODEM_RX        26
#define I2C_SDA         21
#define I2C_SCL         22

#include <TinyGsmClient.h>
#include <Wire.h>
TinyGsmClient getClient();
void setupSIM800();

#endif