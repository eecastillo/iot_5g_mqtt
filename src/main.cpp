/**
 * @file      ModemMqttSubscribeExample.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2022  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2022-09-28
 *
 */
#include <Arduino.h>
#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"
#include "utilities.h"

XPowersPMU  PMU;

// See all AT commands, if wanted
//#define DUMP_AT_COMMANDS

#define TINY_GSM_RX_BUFFER 1024

#define TINY_GSM_MODEM_SIM7080
#include <TinyGsmClient.h>


//#ifdef DUMP_AT_COMMANDS
//#include <StreamDebugger.h>
//StreamDebugger debugger(Serial1, Serial);
//TinyGsm        modem(debugger);
//#else
#define SerialAT  Serial1
TinyGsm        modem(SerialAT);
//#endif

const char *register_info[] = {
    "Not registered, MT is not currently searching an operator to register to. The GPRS service is disabled, the UE is allowed to attach for GPRS if requested by the user.",
    "Registered, home network.",
    "Not registered, but MT is currently trying to attach or searching an operator to register to. The GPRS service is enabled, but an allowable PLMN is currently not available. The UE will start a GPRS attach as soon as an allowable PLMN is available.",
    "Registration denied, the GPRS service is disabled, the UE is not allowed to attach for GPRS if it is requested by the user.",
    "Unknown.",
    "Registered, roaming.",
};

enum {
    MODEM_CATM = 1,
    MODEM_NB_IOT,
    MODEM_CATM_NBIOT,
};


//!! Set the APN manually. Some operators need to set APN first when registering the network.
//!! Set the APN manually. Some operators need to set APN first when registering the network.
//!! Set the APN manually. Some operators need to set APN first when registering the network.
// Using 7080G with Hologram.io , https://github.com/Xinyuan-LilyGO/LilyGo-T-SIM7080G/issues/19
// const char *apn = "hologram";

const char *apn = "internet.itelcel.com";
const char gprsUser[] = "webgprs";
const char gprsPass[] = "webgprs2002";


// mosquito address and port
const char server[]   = "io.adafruit.com";
const int  port       = 8883;
char buffer[1024] = {0};

char username[] = "ie714410";
char password[] = "aio_WSlF17KC643tlUBoVGUKI8lrPeyM";
char clientID[] = "drake";



bool isConnect()
{
    modem.sendAT("+SMSTATE?");
    if (modem.waitResponse("+SMSTATE: ")) {
        String res =  modem.stream.readStringUntil('\r');
        return res.toInt();
    }
    return false;
}


void setup()
{

    Serial.begin(115200);

    // Start while waiting for Serial monitoring
    while (!Serial);

    delay(3000);

    Serial.println();

    /*********************************
     *  step 1 : Initialize power chip,
     *  turn on modem and gps antenna power channel
    ***********************************/
    if (!PMU.begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
        Serial.println("Failed to initialize power.....");
        while (1) {
            delay(5000);
        }
    }

    // If it is a power cycle, turn off the modem power. Then restart it
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED ) {
        PMU.disableDC3();
        // Wait a minute
        delay(200);
    }

    // Set the working voltage of the modem, please do not modify the parameters
    PMU.setDC3Voltage(3000);    // SIM7080 Modem main power channel 2700~ 3400V
    PMU.enableDC3();

    // Modem GPS Power channel
    PMU.setBLDO2Voltage(3300);
    PMU.enableBLDO2();      // The antenna power must be turned on to use the GPS function

    // TS Pin detection must be disable, otherwise it cannot be charged
    PMU.disableTSPinMeasure();


    /*********************************
     * step 2 : start modem
    ***********************************/

    Serial1.begin(115200, SERIAL_8N1, BOARD_MODEM_RXD_PIN, BOARD_MODEM_TXD_PIN);

    pinMode(BOARD_MODEM_PWR_PIN, OUTPUT);
    pinMode(BOARD_MODEM_DTR_PIN, OUTPUT);
    pinMode(BOARD_MODEM_RI_PIN, INPUT);

    int retry = 0;
    while (!modem.testAT(1000)) {
        Serial.print(".");
        if (retry++ > 6) {
            // Pull down PWRKEY for more than 1 second according to manual requirements
            digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
            delay(100);
            digitalWrite(BOARD_MODEM_PWR_PIN, HIGH);
            delay(1000);
            digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
            retry = 0;
            Serial.println("Retry start modem .");
        }
    }
    Serial.println();
    Serial.print("Modem started!");

    /*********************************
     * step 3 : Check if the SIM card is inserted
    ***********************************/
    String result ;


    if (modem.getSimStatus() != SIM_READY) {
        Serial.println("SIM Card is not insert!!!");
        return ;
    }

    // Disable RF
    modem.sendAT("+CFUN=0");
    if (modem.waitResponse(20000UL) != 1) {
        Serial.println("Disable RF Failed!");
    }

    /*********************************
     * step 4 : Set the network mode to NB-IOT
    ***********************************/

    modem.setNetworkMode(2);    // use automatic

    modem.setPreferredMode(MODEM_NB_IOT);

    uint8_t pre = modem.getPreferredMode();

    uint8_t mode = modem.getNetworkMode();

    Serial.printf("getNetworkMode:%u getPreferredMode:%u\n", mode, pre);


    // Set the APN manually. Some operators need to set APN first when registering the network.
    modem.sendAT("+CGDCONT=1,\"IP\",\"", apn, "\"");
    if (modem.waitResponse() != 1) {
        Serial.println("Set operators apn Failed!");
        return;
    }

    //!! Set the APN manually. Some operators need to set APN first when registering the network.
    modem.sendAT("+CNCFG=0,1,\"", apn, "\"");
    if (modem.waitResponse() != 1) {
        Serial.println("Config apn Failed!");
        return;
    }

    // Enable RF
    modem.sendAT("+CFUN=1");
    if (modem.waitResponse(20000UL) != 1) {
        Serial.println("Enable RF Failed!");
    }

    /*********************************
    * step 5 : Wait for the network registration to succeed
    ***********************************/
    SIM70xxRegStatus s;
    do {
        s = modem.getRegistrationStatus();
        if (s != REG_OK_HOME && s != REG_OK_ROAMING) {
            Serial.print(".");
            delay(1000);
        }

    } while (s != REG_OK_HOME && s != REG_OK_ROAMING) ;

    Serial.println();
    Serial.print("Network register info:");
    Serial.println(register_info[s]);


    bool res = modem.isGprsConnected();
    if (!res) {
        // Activate network bearer, APN can not be configured by default,
        // if the SIM card is locked, please configure the correct APN and user password, use the gprsConnect() method
        modem.sendAT("+CNACT=0,1");
        if (modem.waitResponse() != 1) {
            Serial.println("Activate network bearer Failed!");
            return;
        }
        // if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
        //     return ;
        // }
    }

    Serial.print("GPRS status:");
    Serial.println(res ? "connected" : "not connected");

    /*********************************
    * step 6 : setup MQTT Client
    ***********************************/

    // If it is already connected, disconnect it first
    modem.sendAT("+SMDISC");
    modem.waitResponse();


    snprintf(buffer, 1024, "+SMCONF=\"URL\",\"%s\",%d", server, port);
    modem.sendAT(buffer);
    if (modem.waitResponse() != 1) {
        return;
    }
    snprintf(buffer, 1024, "+SMCONF=\"USERNAME\",\"%s\"", username);
    modem.sendAT(buffer);
    if (modem.waitResponse() != 1) {
        return;
    }

    snprintf(buffer, 1024, "+SMCONF=\"PASSWORD\",\"%s\"", password);
    modem.sendAT(buffer);
    if (modem.waitResponse() != 1) {
        return;
    }

    snprintf(buffer, 1024, "+SMCONF=\"CLIENTID\",\"%s\"", clientID);
    modem.sendAT(buffer);
    if (modem.waitResponse() != 1) {
        return;
    }

    int8_t ret;
    do {

        modem.sendAT("+SMCONN");
        ret = modem.waitResponse(30000);
        if (ret != 1) {
            Serial.println("Connect failed, retry connect ..."); delay(1000);
        }

    } while (ret != 1);


    Serial.println("MQTT Client connected!");


    /*********************************
    * step 7 : Subscribe topic
    ***********************************AT+SMSUB=”exampleTopic”,1 */
    snprintf(buffer, 1024, "+SMSUB=\"ie714410/feeds/set-alarm\",1");
    modem.sendAT(buffer);
    if (modem.waitResponse() != 1) {
        return;
    }

    Serial.print("MQTT Subscribe topic : ");
    Serial.println(buffer);

}


void loop()
{
    if (!isConnect()) {
        Serial.println("MQTT Client disconnect!"); delay(1000);
        return ;
    }

    // Waiting for subscribed messages
    if (modem.waitResponse("+SMSUB: ") == 1) {
        String result =  modem.stream.readStringUntil('\r');
        Serial.print("Recive payload:");
        Serial.println(result);


        int index = result.indexOf(",");
        if (index < 0) {
            Serial.println("index error !"); return;
        }

        result = result.substring(index + 1);
        result.replace("\"", "");

        // Get command value
        char value = result[result.length() - 1];
        // Get Sep
        result = result.substring(0, result.length() - 2);

        String payload = "ok,";
        payload.concat(result);

        if (value == '1') {
            PMU.setChargingLedMode(XPOWERS_CHG_LED_ON);
        } else {
            PMU.setChargingLedMode(XPOWERS_CHG_LED_OFF);
        }

        // v1/username/things/clientID/response
        snprintf(buffer, 1024, "+SMPUB=\"ie714410/feeds/gas\",%d,1,1", payload.length());
        modem.sendAT(buffer);
        if (modem.waitResponse(">") == 1) {
            modem.stream.write(payload.c_str(), payload.length());
            Serial.print("Try publish payload: ");
            Serial.println(payload);

            if (modem.waitResponse(3000)) {
                Serial.println("Send Packet success!");
            } else {
                Serial.println("Send Packet failed!");
            }
        }

    }
}
