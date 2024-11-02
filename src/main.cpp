/**
 * @file      ModemMqttPulishExample.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2022  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2022-09-16
 *
 */
#include <Arduino.h>
#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"
#include "utilities.h"

XPowersPMU  PMU;

// See all AT commands, if wanted

#define TINY_GSM_RX_BUFFER 1024

#define TINY_GSM_MODEM_SIM7080
#include <TinyGsmClient.h>
#include "utilities.h"


#define SerialAT  Serial1
TinyGsm        modem(SerialAT);

const char *register_info[] = {
    "Not registered, MT is not currently searching an operator to register to.The GPRS service is disabled, the UE is allowed to attach for GPRS if requested by the user.",
    "Registered, home network.",
    "Not registered, but MT is currently trying to attach or searching an operator to register to. The GPRS service is enabled, but an allowable PLMN is currently not available. The UE will start a GPRS attach as soon as an allowable PLMN is available.",
    "Registration denied, The GPRS service is disabled, the UE is not allowed to attach for GPRS if it is requested by the user.",
    "Unknown.",
    "Registered, roaming.",
};

const char *rootCA = "-----BEGIN CERTIFICATE-----\n\
MIIEAzCCAuugAwIBAgIUBY1hlCGvdj4NhBXkZ/uLUZNILAwwDQYJKoZIhvcNAQEL\n\
BQAwgZAxCzAJBgNVBAYTAkdCMRcwFQYDVQQIDA5Vbml0ZWQgS2luZ2RvbTEOMAwG\n\
A1UEBwwFRGVyYnkxEjAQBgNVBAoMCU1vc3F1aXR0bzELMAkGA1UECwwCQ0ExFjAU\n\
BgNVBAMMDW1vc3F1aXR0by5vcmcxHzAdBgkqhkiG9w0BCQEWEHJvZ2VyQGF0Y2hv\n\
by5vcmcwHhcNMjAwNjA5MTEwNjM5WhcNMzAwNjA3MTEwNjM5WjCBkDELMAkGA1UE\n\
BhMCR0IxFzAVBgNVBAgMDlVuaXRlZCBLaW5nZG9tMQ4wDAYDVQQHDAVEZXJieTES\n\
MBAGA1UECgwJTW9zcXVpdHRvMQswCQYDVQQLDAJDQTEWMBQGA1UEAwwNbW9zcXVp\n\
dHRvLm9yZzEfMB0GCSqGSIb3DQEJARYQcm9nZXJAYXRjaG9vLm9yZzCCASIwDQYJ\n\
KoZIhvcNAQEBBQADggEPADCCAQoCggEBAME0HKmIzfTOwkKLT3THHe+ObdizamPg\n\
UZmD64Tf3zJdNeYGYn4CEXbyP6fy3tWc8S2boW6dzrH8SdFf9uo320GJA9B7U1FW\n\
Te3xda/Lm3JFfaHjkWw7jBwcauQZjpGINHapHRlpiCZsquAthOgxW9SgDgYlGzEA\n\
s06pkEFiMw+qDfLo/sxFKB6vQlFekMeCymjLCbNwPJyqyhFmPWwio/PDMruBTzPH\n\
3cioBnrJWKXc3OjXdLGFJOfj7pP0j/dr2LH72eSvv3PQQFl90CZPFhrCUcRHSSxo\n\
E6yjGOdnz7f6PveLIB574kQORwt8ePn0yidrTC1ictikED3nHYhMUOUCAwEAAaNT\n\
MFEwHQYDVR0OBBYEFPVV6xBUFPiGKDyo5V3+Hbh4N9YSMB8GA1UdIwQYMBaAFPVV\n\
6xBUFPiGKDyo5V3+Hbh4N9YSMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEL\n\
BQADggEBAGa9kS21N70ThM6/Hj9D7mbVxKLBjVWe2TPsGfbl3rEDfZ+OKRZ2j6AC\n\
6r7jb4TZO3dzF2p6dgbrlU71Y/4K0TdzIjRj3cQ3KSm41JvUQ0hZ/c04iGDg/xWf\n\
+pp58nfPAYwuerruPNWmlStWAXf0UTqRtg4hQDWBuUFDJTuWuuBvEXudz74eh/wK\n\
sMwfu1HFvjy5Z0iMDU8PUDepjVolOCue9ashlS4EB5IECdSR2TItnAIiIwimx839\n\
LdUdRudafMu5T5Xma182OC0/u/xRlEm+tvKGGmfFcN0piqVl8OrSPBgIlb+1IKJE\n\
m/XriWr/Cq4h/JfB7NTsezVslgkBaoU=\n\
-----END CERTIFICATE-----\n";

enum {
    MODEM_CATM = 1,
    MODEM_NB_IOT,
    MODEM_CATM_NBIOT,
};

const int AUTOMATIC_NETWORK = 2;

#define randMax 35
#define randMin 18

// Your GPRS credentials, if any
const char apn[] = "emnify";
const char gprsUser[] = "";//"webgprs"; // User
const char gprsPass[] = "";//"webgprs2002"; // Password

// cayenne server address and port
const char server[]   = "io.adafruit.com";
const int  port       = 8883;
char buffer[1024] = {0};

// To create a device : https://cayenne.mydevices.com/cayenne/dashboard
//  1. Add new...
//  2. Device/Widget
//  3. Bring Your Own Thing
//  4. Copy the <MQTT USERNAME> <MQTT PASSWORD> <CLIENT ID> field to the bottom for replacement
char username[] = "ie714410";
char password[] = "aio_XWlF91lrgcTDWWQWodZqT2hbrfst";
char clientID[] = "ESP32";

// To create a widget
//  1. Add new...
//  2. Device/Widget
//  3. Custom Widgets
//  4. Value
//  5. Fill in the name and select the newly created equipment
//  6. Channel is filled as 0
//  7.  Choose ICON
//  8. Add Widget
int data_channel = 0;


bool isConnect()
{
    modem.sendAT("+SMSTATE?");
    if (modem.waitResponse("+SMSTATE: ")) {
        String res =  modem.stream.readStringUntil('\r');
        return res.toInt();
    }
    return false;
}


void writeCaFiles(int index, const char *filename, const char *data,
                  size_t lenght)
{
    modem.sendAT("+CFSTERM");
    modem.waitResponse();


    modem.sendAT("+CFSINIT");
    if (modem.waitResponse() != 1) {
        Serial.println("INITFS FAILED");
        return;
    }
    // AT+CFSWFILE=<index>,<filename>,<mode>,<filesize>,<input time>
    // <index>
    //      Directory of AP filesystem:
    //      0 "/custapp/" 1 "/fota/" 2 "/datatx/" 3 "/customer/"
    // <mode>
    //      0 If the file already existed, write the data at the beginning of the
    //      file. 1 If the file already existed, add the data at the end o
    // <file size>
    //      File size should be less than 10240 bytes. <input time> Millisecond,
    //      should send file during this period or you can’t send file when
    //      timeout. The value should be less
    // <input time> Millisecond, should send file during this period or you can’t
    // send file when timeout. The value should be less than 10000 ms.

    size_t payloadLenght = lenght;
    size_t totalSize     = payloadLenght;
    size_t alardyWrite   = 0;

    while (totalSize > 0) {
        size_t writeSize = totalSize > 10000 ? 10000 : totalSize;

        modem.sendAT("+CFSWFILE=", index, ",", "\"", filename, "\"", ",",
                     !(totalSize == payloadLenght), ",", writeSize, ",", 10000);
        modem.waitResponse(30000UL, "DOWNLOAD");
REWRITE:
        modem.stream.write(data + alardyWrite, writeSize);
        if (modem.waitResponse(30000UL) == 1) {
            alardyWrite += writeSize;
            totalSize -= writeSize;
            Serial.printf("Writing:%d overage:%d\n", writeSize, totalSize);
        } else {
            Serial.println("Write failed!");
            delay(1000);
            goto REWRITE;
        }
    }

    Serial.println("Wirte done!!!");

    modem.sendAT("+CFSTERM");
    if (modem.waitResponse() != 1) {
        Serial.println("CFSTERM FAILED");
        return;
    }
}

void setup()
{

    Serial.begin(115200);

    //Start while waiting for Serial monitoring
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
    //Set the working voltage of the modem, please do not modify the parameters
    PMU.setDC3Voltage(3000);    //SIM7080 Modem main power channel 2700~ 3400V
    PMU.enableDC3();

    //Modem GPS Power channel
    PMU.setBLDO2Voltage(3300);
    PMU.enableBLDO2();      //The antenna power must be turned on to use the GPS function

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


    /*********************************
     * step 4 : Set the network mode to NB-IOT
    ***********************************/

    modem.setNetworkMode(AUTOMATIC_NETWORK);    //automatic detection AUTOMATIC_NETWORK=2

    modem.setPreferredMode(MODEM_CATM);       // NB_IOT(2) as preferred mode

    uint8_t pre = modem.getPreferredMode();

    uint8_t mode = modem.getNetworkMode();

    Serial.printf("getNetworkMode:%u getPreferredMode:%u\n", mode, pre);


    /*********************************
    * step 5 : Wait for the network registration to succeed
    ***********************************/
   //check with and without this
    modem.sendAT("+CNCFG=0,1,\"emnify\"");
    if (modem.waitResponse() != 1) {
        Serial.println("Set operators apn Failed!");
        return;
    }
    modem.sendAT("+CNACT=0,1");
    if (modem.waitResponse() != 1) {
        Serial.println("Set operators cnact Failed!");
        return;
    }
    modem.sendAT("+CAOPEN=0,0,\"TCP\",\"10.189.169.49\",443");
    if (modem.waitResponse() != 1) {
        Serial.println("Set operators caopen Failed!");
        return;
    }
    SIM70xxRegStatus s;
    do {
        s = modem.getRegistrationStatus();
        if (s != REG_OK_HOME && s != REG_OK_ROAMING) {
            Serial.printf("Response: %u", s);
            Serial.print(".");
            delay(1000);
        }

    } while (s != REG_OK_HOME && s != REG_OK_ROAMING) ;

    Serial.println();
    Serial.print("Network register info:");
    Serial.println(register_info[s]);

    // Activate network bearer, APN can not be configured by default,
    // if the SIM card is locked, please configure the correct APN and user password, use the gprsConnect() method

    bool res = modem.isGprsConnected();
    if (!res) {
        modem.sendAT("+CNACT=0,1");
        //if (modem.waitResponse() != 1) {
        //    Serial.println("Activate network bearer Failed!");
         //   return;
        //}
         if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
             return ;
         }
    }

    Serial.print("GPRS status:");
    Serial.println(res ? "connected" : "not connected");


    // Before connecting, you need to confirm that the time has been synchronized.
    modem.sendAT("+CCLK?");
    modem.waitResponse(30000);


    /*********************************
     * step 6 : import  ca
     ***********************************/
    writeCaFiles(3, "server-ca.crt", rootCA, strlen(rootCA));

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

// Subscribe to MQTT topic
    /*
        snprintf(buffer, 1024, "+SMSUB=\"ie714410/feeds/%s\",1", topic);
    modem.sendAT(buffer);
    if (modem.waitResponse() != 1) {
        return;
    }

    Serial.print("MQTT Subscribe topic : ");
    Serial.println(buffer);
    */
    
    // random seed data
    //randomSeed(esp_random());
}

void loop()
{
    if (!isConnect()) {
        Serial.println("MQTT Client disconnect!"); delay(1000);
        return ;
    }

    Serial.println();
    // Publish fake temperature data
    String payload = "45\r\n";//"temp,c=";
    //int temp =  rand() % (randMax - randMin) + randMin;
    //payload.concat(temp);
    //payload.concat("\r\n");

    // AT+SMPUB=<topic>,<content length>,<qos>,<retain><CR>message is enteredQuit edit mode if messagelength equals to <contentlength>
    snprintf(buffer, 1024, "+SMPUB=\"ie714410/feeds/nivel_gas\",%d,1,1", payload.length());
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

    //Check if subscription has messages
    /*
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

        //Get command value
        char value = result[result.length() - 1];
        //Get Sep
        result = result.substring(0, result.length() - 2);

        String payload = "ok,";
        payload.concat(result);

        if (value == '1') {
            PMU.setChargingLedMode(XPOWERS_CHG_LED_ON);
        } else {
            PMU.setChargingLedMode(XPOWERS_CHG_LED_OFF);
        }

        // v1/username/things/clientID/response
        snprintf(buffer, 1024, "+SMPUB=\"v1/%s/things/%s/response\",%d,1,1", username, clientID, payload.length());
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
    */
    
    //Sleep and wakeup block
    delay(60000);
}
