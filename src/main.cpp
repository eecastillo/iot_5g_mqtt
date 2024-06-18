
// Please select the corresponding model

 #define SIM800L_IP5306_VERSION_20190610


// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG          SerialMon

#include "utilities.h"
//#include "driver/adc.h"

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial
// Set serial for AT commands (to the module)
#define SerialAT  Serial1

// Configure TinyGSM library
#define TINY_GSM_MODEM_SIM800          // Modem is SIM800
#define TINY_GSM_RX_BUFFER      1024   // Set RX buffer to 1Kb

#include <TinyGsmClient.h>
#include <PubSubClient.h>


TinyGsm modem(SerialAT);

#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  60        /* Time ESP32 will go to sleep (in seconds) */


// Server details
const char server[] = "vsh.pp.ua";
const char resource[] = "/TinyGSM/logo.txt";

// Your GPRS credentials (leave empty, if missing)
const char apn[]      = "internet.itelcel.com"; // Your APN
const char gprsUser[] = "webgprs"; // User
const char gprsPass[] = "webgprs2002"; // Password
const char simPIN[]   = ""; // SIM card PIN code, if any

TinyGsmClient client(modem);
PubSubClient mqtt(client);
const int  port = 80;

// MQTT details
const char* broker_adafruit = "io.adafruit.com";
const char* user_adafruit = "ie714410";
const char* password_adafruit = "aio_JiDj44A9Z1qKNJ6sJLfAEfCRfB3n";
const int port_adafruit = 1883;

const char* broker_hive = "1ef84d7190794d81ade79467c7c158a5.s1.eu.hivemq.cloud";
const char* user_hive   = "prueba";
const char* password_hive   = "prueba123";
const int port_hive = 8883;

const char* topicText = "ie714410/feeds/texto";
const char* topicGas = "ie714410/feeds/nivel_gas";

bool g_energy = false;
float battery_level;

boolean mqttConnect() {
  SerialMon.print("Connecting to ");
  SerialMon.print(broker_adafruit);

  // Connect to MQTT Broker
  //boolean status = mqtt.connect("GsmClientTest");

  // Or, if you want to authenticate MQTT:
  boolean status = mqtt.connect("GsmClientTest", user_adafruit, password_adafruit);

  if (status == false) {
    SerialMon.println(" fail");
    return false;
  }
  SerialMon.println(" success");
  return mqtt.connected();
}

void setupModem()
{
#ifdef MODEM_RST
    // Keep reset high
    pinMode(MODEM_RST, OUTPUT);
    digitalWrite(MODEM_RST, HIGH);
#endif

    pinMode(MODEM_PWRKEY, OUTPUT);
    pinMode(MODEM_POWER_ON, OUTPUT);

    // Turn on the Modem power first
    digitalWrite(MODEM_POWER_ON, HIGH);

    // Pull down PWRKEY for more than 1 second according to manual requirements
    digitalWrite(MODEM_PWRKEY, HIGH);
    delay(100);
    digitalWrite(MODEM_PWRKEY, LOW);
    delay(1000);
    digitalWrite(MODEM_PWRKEY, HIGH);

    // Initialize the indicator as an output
    pinMode(LED_GPIO, OUTPUT);
    digitalWrite(LED_GPIO, LED_OFF);
}

void MQTTT()
{   
    // MQTT Broker setup
    mqtt.setServer(broker_adafruit, port_adafruit);
    delay(1000); // delay provicional

    while (!mqtt.connected()) {
        SerialMon.println("=== MQTT NOT CONNECTED ===");   
        if (mqttConnect()) {
            mqtt.publish(topicGas, "35");
        }
        else
        {
                SerialMon.println("=== RESTARTING ===");
                mqtt.loop();
                mqtt.disconnect();
                //ESP.restart();
                //delay(5000);
                esp_sleep_enable_timer_wakeup(1 * uS_TO_S_FACTOR);
                esp_deep_sleep_start();
        }
        delay(1000);    /// delay a;adido para ver si funciona a la primera
    }
}

void turnOffNetlight()
{
    SerialMon.println("Turning off SIM800 Red LED...");
    modem.sendAT("+CNETLIGHT=0");
}

void turnOnNetlight()
{
    SerialMon.println("Turning on SIM800 Red LED...");
    modem.sendAT("+CNETLIGHT=1");
}
void gethour()
{
      int year = 0;
  int month = 0;
  int day = 0;
  int hour = 0;
  int min = 0;
  int sec = 0;
  float timezone = 0;
  //ciclo para obtener la fecha y hora
  for(;;)
  {
    DBG("Requesting current network time");
    if(modem.getNetworkTime(&year, &month, &day, &hour, &min, &sec, &timezone))
    {
      DBG("Year: ", year, "\tMonth: ", month, "\tDay: ", day);
      DBG("Hour: ", hour, "\tMinute: ", min, "\tSecond: ", sec);
      DBG("Timezone: ", timezone);
      break;
    }
    else
    {
      DBG("Couldn't get network time, retrying in 5s");
      delay(5000L);
    }
  }  
}
//#define FIRST_TIME
float voltage;
float batteryLevel;
float sum;
#define BATTERY_METTER_PIN 35
#define NUM_SAMPLES 21
void getBateryValue() {
// tomar un número de muestras analógicas y sumarlas
int sample_count = 0;
while (sample_count < NUM_SAMPLES) {
sum += analogRead(BATTERY_METTER_PIN);
sample_count++;
delay(100);
}
voltage = (sum/NUM_SAMPLES)/ 2350.0;
batteryLevel = _min(map(sum / NUM_SAMPLES, 2000, 2440.0, 0, 100), 100); //1100
if (sum/NUM_SAMPLES < 1200) batteryLevel = 0;
Serial.print("Valor Analogo: ");
Serial.println(sum);
Serial.print("Battery Lavel:");
Serial.print(batteryLevel);
Serial.println("%");
sample_count = 0; //Reset counter
sum = 0; //Reset counter
voltage = voltage * 4.20;
Serial.print(voltage);
Serial.println (" V");
}

void setup()
{
    SerialMon.begin(115200);
    // Start I2C communication  
  bool isOk = setupPMU();
  SerialMon.println(String("IP5306 KeepOn ") + (isOk ? "OK" : "FAIL"));
  pinMode(LED_GPIO,OUTPUT);

 
    setCpuFrequencyMhz(80);
    // Set console baud rate
    SerialMon.begin(115200);

    delay(10);
    //adc_power_on();
///////////////////////////////////////////////////
    //pinMode(18, INPUT_PULLUP);
    //pinMode(19, INPUT_PULLUP);

       digitalWrite(LED_GPIO, LED_ON);
    delay(2000);

    // Start power management
    if (setupPMU() == false) {
        Serial.println("Setting power error");
    }

    // Some start operations
    setupModem();

    // Set GSM module baud rate and UART pins
    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);

     // Restart takes quite some time
    // To skip it, call init() instead of restart()
    SerialMon.println("Initializing modem2...");
    modem.init();

    // Turn off network status lights to reduce current consumption
    turnOffNetlight();

    // The status light cannot be turned off, only physically removed
    //turnOffStatuslight();

    // Or, use modem.init() if you don't need the complete restart
    String modemInfo = modem.getModemInfo();
    SerialMon.print("Modem: ");
    SerialMon.println(modemInfo);

    // Unlock your SIM card with a PIN if needed
    if (strlen(simPIN) && modem.getSimStatus() != 3 ) {
        modem.simUnlock(simPIN);
    }

    SerialMon.print("Waiting for network333...");
    if (!modem.waitForNetwork(240000L)) {
        SerialMon.println(" fail");
        delay(10000);
        return;
    }
    SerialMon.println(" OK");

    // When the network connection is successful, turn on the indicator
    digitalWrite(LED_GPIO, LED_ON);

    if (modem.isNetworkConnected()) {
        SerialMon.println("Network connected");
    }

    SerialMon.print(F("Connecting to APN: "));
    SerialMon.print(apn);
    if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
        SerialMon.println(" fail");
        delay(10000);
        return;
    }
    SerialMon.println(" OK");

    // battery_level = map(analogRead(33), 0.0f, 4095.0f, 0, 100);
    //DBG("Battery level:    ",battery_level);
   
    //uint8_t opto_1 = digitalRead(18);
    //uint8_t opto_2 = digitalRead(19);
   // DBG("Valor en el GPIO18: ", opto_1, "\n\rValor en el GPIO19:", opto_2);
    /*if(opto_1 == 0 || opto_2 == 0)
    {
        g_energy = true;
    }
    else
    {
        g_energy = false; 
    }*/

    MQTTT();
    mqtt.loop();
    mqtt.disconnect();

    modem.gprsDisconnect();
    SerialMon.println(F("GPRS disconnected"));

    gethour();

    //After all off
    modem.poweroff();
    //adc_power_off();

    SerialMon.println(F("Poweroff"));

    //esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

    //esp_deep_sleep_start();
}

uint32_t lastReconnectAttempt = 0;


void loop()
{
 // delay(1000);
 // digitalWrite(LED_GPIO,HIGH);
 // delay(100);
 // digitalWrite(LED_GPIO,LOW);
    /*
    The sleep current using AXP192 power management is about 500uA,
    and the IP5306 consumes about 1mA
    */
}