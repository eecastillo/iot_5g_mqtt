#include "mqtt_client.h"

// MQTT details
const char* broker = "1ef84d7190794d81ade79467c7c158a5.s1.eu.hivemq.cloud";
const char* user   = "prueba";
const char* password   = "prueba123";
const int mqtt_port = 8883;

const char* topicLed = "GsmClientTest/led";
const char* topicInit = "GsmClientTest/init";
const char* topicLedStatus = "GsmClientTest/ledStatus";

const char* topicPower = "GsmClientTest/led";
const char* topicBattery = "nivel-gas";



PubSubClient mqtt;

#define LED_PIN 13
int ledStatus = LOW;

uint32_t lastReconnectAttempt = 0;

void mqttClient()
{
  TinyGsmClient client = getClient();
  PubSubClient mqtt(client);
}

void mqttCallback(char* topic, byte* payload, unsigned int len) {
  SerialMon.print("Message arrived [");
  SerialMon.print(topic);
  SerialMon.print("]: ");
  SerialMon.write(payload, len);
  SerialMon.println();

  // Only proceed if incoming message's topic matches
  if (String(topic) == topicLed) {
    ledStatus = !ledStatus;
    digitalWrite(LED_PIN, ledStatus);
    mqtt.publish(topicLedStatus, ledStatus ? "1" : "0");
    
  }
}
boolean mqttConnect() {
  SerialMon.print("Connecting to ");
  SerialMon.print(broker);

  // Connect to MQTT Broker
  boolean status = mqtt.connect("GsmClientName", user, password);

  if (status == false) {
    SerialMon.println(" fail");
    return false;
  }
  SerialMon.println(" success");
  mqtt.publish(topicInit, "GsmClientTest started");
  mqtt.subscribe(topicLed);
  return mqtt.connected();
}

void mqtt_init()
{
  pinMode(LED_PIN, OUTPUT);
  setupSIM800();
  mqttClient();

  // MQTT Broker setup
  mqtt.setServer(broker, mqtt_port); //broker and port
  mqtt.setCallback(mqttCallback);
  mqttConnect();
}

void mqtt_publish_power(bool isOn)
{
    mqtt.publish(topicPower,isOn ? "1":"0");
}
void mqtt_publish_battery(float battery_percentage)
{
    mqtt.publish(topicBattery,String(battery_percentage).c_str());
}

void mqtt_sleep_module()
{

}