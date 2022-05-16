/*
***************************************************************************  
**  Program  : ESP2RFlink
*/
#define _FW_VERSION "v1.1 (27-02-2020)"
/*
**  Copyright (c) 2020-2022 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*      
  Arduino-IDE settings:

    - Board             : "Generic ESP8266 Module"
    - Flash mode        : "DOUT" | "DIO"    // change only after power-off and on again!
    - Flash size        : "1MB (FS:none OAT:~502KB)  
    -   or maybe        : "512KB (FS:none OAT:~246KB)"
    - DebugT port       : "Disabled"
    - DebugT Level      : "None"
    - IwIP Variant      : "v2 Lower Memory"
    - Reset Method      : "none"   // but will depend on the programmer!
    - Crystal Frequency : "26 MHz" 
    - VTables           : "Flash"
    - Flash Frequency   : "40MHz"
    - CPU Frequency     : "80 MHz"
    - Buildin Led       : "2"  // not used?
    - Upload Speed      : "115200"                                                                                                                                                                                                                                                 
    - Erase Flash       : "Only Sketch"
*/

/*

    The MQTT_MAX_PACKET_SIZE set to 200 in PubSubClient.h
*/
#include <ESP8266WiFi.h>          // https://github.com/esp8266/Arduino
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          // https://github.com/tzapu/WiFiManager
#include <PubSubClient.h>
#include <EEPROM.h>
#include <Ticker.h>
#include "macros.h"

#define _MAX_BUFFER  200
#define _MAX_STACK    10
#define _TEMP_MAX     50.0
#define _HUM_MAX     100
#define _BLINK_LED     0  // GPIO00

WiFiClient      wifiClient;
WiFiClient      telnetClient;
WiFiServer      telnetServer(23);
PubSubClient    mqttClient(wifiClient);
Ticker          blinker;
Ticker          keepAlive;
Ticker          autoConnectTimeOut;

enum { ERROR, ALWAYS, WARNING, DEBUG, INFO };    // debug levels

typedef struct {
    char    configOK;           
    char    serverIP[30];
    char    user[25];
    char    passwd[17];
    char    topTopic[20];
} mqttStruct;

typedef struct {
    char     topic[_MAX_BUFFER];           
    char     payLoad[_MAX_BUFFER];
    uint32_t pushTime;
} stackStruct;

//-- we maken gebruik van de WiFiManager ----------------------------------------
// const char* ssid     = "xxxxxxxxxx"; // network SSID for ESP8266 to connect to
// const char* password = "xxxxxxxxxx"; // password for the network above
//-------------------------------------------------------------------------------
String      clientName;
String      topicMessage    = "RFlink/Message" ;
String      topicState      = "RFlink/State";
String      topicStateSet   = "RFlink/State/Set";
String      topicStateSetX  = "RFlink/State/Set/#";  // command topic ESP will subscribe to and pass as commands to the RFLink
String      topicDebug      = "RFlink/Debug" ;
String      topicJSON       = "RFlink/JSON" ;
String      MQTTTopic;
int         pingNum;
static      mqttStruct mqttConfig;
bool        connect2MQTT;

char        inBuffer[_MAX_BUFFER+5];    // +5 is marge zodat ik niet echt hoef na te denken
char        outBuffer[_MAX_BUFFER+5];
char        sTopic[_MAX_BUFFER];
char        sPayLoad[_MAX_BUFFER];
int         inBufferInx, outBufferInx  = 0;
char        inChar, outChar;
String      field[20];
String      RFlinkString, protocolID, deviceID, switchID, setCMD;
String      lastRFtopic     = "";
String      lastRFdevice    = ""; 
String      lastRFcommand   = "";
bool        isSwitch = false;
uint16_t    flashTimer;
uint32_t    reconnectTimer;
extern      stackStruct mqttStack[_MAX_QUEUE+1];

volatile int8_t      inStack = -1;
 

String CompassDirTable[17] = {"N","NNE","NE","ENE","E","ESE", "SE","SSE","S","SSW","SW","WSW", "W","WNW","NW","NNW","N"}; 

uint32_t    waitForOK = millis();

//============================================================
bool compareArray(char* In1, char* In2) {
int p = 0;
    while(In1[p] >= ' ' && In1[p] <= 'z' && In2[p] >= ' ' && In2[p] <= 'z') {
        if (In1[p] != In2[p]) return false;
        p++;
    }
    yield();
    return true;
}   // compareArray()

//============================================================
uint8_t pushMQTTmessage(char* Topic, char* payLoad) {
int iX = 0, len = 0;
    _DEBUG("pushMQTTmessage(): Stackpointer["); _DEBUG(inStack); _DEBUGLN("]");
    if (inStack < _MAX_STACK) {
        inStack++;
        if (inStack >= _MAX_STACK) inStack = _MAX_STACK;   // sanity check
        _DEBUG("pushMQTTmessage(): inStack");
        _DEBUG("[");
        _DEBUG(inStack);
        _DEBUG("] =>");
        iX = 0;
        while(Topic[iX]  >= ' ' && Topic[iX] <= 'z') {
            mqttStack[inStack].topic[iX] = Topic[iX];
            iX++;
        }
        mqttStack[inStack].topic[iX] = '\0';
        yield();
        iX = 0;
        while(payLoad[iX]  >= ' ' && payLoad[iX] <= 'z') {
            mqttStack[inStack].payLoad[iX] = payLoad[iX];
            iX++;
        }
        mqttStack[inStack].payLoad[iX] = '\0';
        mqttStack[inStack].pushTime = millis();
        yield();
        _DEBUG(" Topic [");
        _DEBUG(mqttStack[inStack].topic);
        _DEBUGLN("]");
        _DEBUG("pushMQTTmessage(): payLoad [");
        _DEBUG(mqttStack[inStack].payLoad);
        _DEBUG("] => now[");
        _DEBUG((inStack + 1));
        _DEBUGLN("] messages on Stack");
        if (inStack == 0) {
            _DEBUG("Pushed to Stack ["); _DEBUG(inStack); _DEBUGLN("]"); 
            return inStack; 
        }   // only one entry, nothing to test!
        // new entry == previous entry ??? then it's a duplicate => erase
        else if (   compareArray( mqttStack[inStack].topic,   mqttStack[(inStack - 1)].topic)
                 && compareArray( mqttStack[inStack].payLoad, mqttStack[(inStack - 1)].payLoad) ) {
            _DEBUGLN("pushMQTTmessage(): Duplicat message .. skipped!");
            return --inStack;
        } else {
            _DEBUG("Pushed to Stack ["); _DEBUG(inStack); _DEBUGLN("]"); 
            return inStack;     // new entry. All OK
        }
    } 
    return -1;
}   // pushMQTTmessage()


//============================================================
int popMQTTmessage(char* Topic, char* payLoad) {
int iX = 0;
    _DEBUG("popMQTTmessage(): pop[");
    _DEBUG(inStack);
    _DEBUGLN("] ...");
    if (inStack < 0)    return -1;
    else {
        _DEBUG("popMQTTmessage(): inStack");
        _DEBUG("[");
        _DEBUG(inStack);
        _DEBUG("] =>");

        iX = 0;
        while (mqttStack[inStack].topic[iX] != '\0') {
            Topic[iX] = mqttStack[inStack].topic[iX];
            iX++;
        }
        Topic[iX] = '\0';
        yield();
        iX = 0;
        while (mqttStack[inStack].payLoad[iX] != '\0') {
            payLoad[iX] = mqttStack[inStack].payLoad[iX];
            iX++;
        }
        payLoad[iX] = '\0';

        yield();

        _DEBUG(" Topic [");                     _DEBUG(Topic);   _DEBUGLN("]");
        _DEBUG("popMQTTmessage(): payLoad [");  _DEBUG(payLoad); _DEBUGLN("]");

        memset(mqttStack[inStack].topic,0,sizeof(mqttStack[inStack].topic));
        memset(mqttStack[inStack].payLoad,0,sizeof(mqttStack[inStack].payLoad));

        _DEBUG("Popped from Stack ["); _DEBUG(inStack); 
        _DEBUG("], waited["); _DEBUG((millis() - mqttStack[inStack].pushTime));
        _DEBUGLN("] milli-seconds");

        inStack--;
        return iX;  // lengte van payLoad
    } 
    return -1;
}   // popMQTTmessage()

//============================================================
//==== here we process Buffer that comes from the         ====
//==== telnetClient to be send to the RFlink              ====
//==== format has to be                                   ====
//==== '10;<proto>;<address>;<switch>;<command>;'         ====
//============================================================
void parseFromTelnet(char* Buffer, int len) {
int     numFields;

    Buffer[++len] = '\0';
    
    _INFO("parseFromTelnet(): len[");
    _INFO(len);  _INFO("] : [");
    _INFO(Buffer);
    _INFOLN("]");
    
    if (len >= 3 && (strncmp("ESP",Buffer, 3) == 0 )) {
        parseESPcommand(Buffer, len);
        return;
    }
    //==== we need to know these three values to be able to ====
    //==== compose a status reply (if received from RFlink) ====
    numFields = splitBuffer(Buffer, len);

    //==== not a "10" command, so don't send to RFlink ====
    if (field[0].equals("10") == 0) {
        return;
    }
    
    _DEBUG("parseFromTelnet(): Number of fields [");
    _DEBUG(numFields);  _DEBUGLN("]");
    
    if (numFields >= 4) {
        protocolID  = field[1];
        _DEBUG("parseFromExt(): protocolID["); _DEBUG(protocolID); _DEBUG("], ");
        deviceID    = field[2];
        _DEBUG("deviceID[");  _DEBUG(deviceID);   _DEBUG("], ");
        switchID    = field[3];
        isSwitch    = true;
        _DEBUG("switchID[");  _DEBUG(switchID);   _DEBUG("], ");
        lastRFcommand   = field[4];
        _DEBUG("setCMD[");    _DEBUG(lastRFcommand);     _DEBUGLN("] ");
    
        //waitForOK = millis() + 90000;
    }
    //==== now send Buffer straight to RFlink and wait for OK reply .. ====
    lastRFtopic = protocolID + ";" + deviceID + ";" + switchID + ";";
    if (telnetClient.connected()) {
        telnetClient.print("SEND :  => toRFlink(10;");
        telnetClient.print(lastRFtopic); 
        telnetClient.print(lastRFcommand); 
        telnetClient.println(";)");
    }
    toRFlink("10;" + lastRFtopic + lastRFcommand + ";");

    //==== empty Buffer ====
    len = 0;
    Buffer[len] = '\0';
    
}   // parseFromTelnet()


//============================================================
//==== here we process input from RFlink and create a     ====
//==== reply to be send to MQTT                           ====
//==== format is:                                         ====
//==== '20;83;Alecto V1;ID=031c;TEMP=002f;HUM=76;BAT=OK;' ====
//==== or:                                                ====
//==== '20;8E;Eurodomest;ID=1a82aa;SWITCH=00;CMD=OFF;'    ====
//============================================================
void replyRFlink2MQTT(char* Buffer, int len) {
int     fieldNum, tmpInt, ix;
String  responseObjRaw, responseObj, fieldID;
String  Topic, RFtopic, RFcommand; 
float   tmpFloat;
char    tmpChar[100];

    if (Buffer[0] == '\0') {
        _DEBUG("replyRFlink2MQTT(): received empty Buffer");
        return;
    }

    //==== opsplitsen van de Buffer in losse velden ====
    fieldNum = splitBuffer(Buffer, len);
    
    //==== iets anders dan een "20" response ontvangen ? ====
    if (field[0].equals("20") == 0) {
        _DEBUG("replyRFlink2MQTT(): Received [");  _DEBUG(field[0]);  
        _DEBUGLN("] reply from RFlink");
        _DEBUG("replyRFlink2MQTT(): Wrong Node ["); _DEBUG(Buffer); _DEBUGLN("] ");
        return;
    }
    //==== is het een "OK" reply? ====
    if (field[2].equals("OK")) {
        Topic   = topicState + "/" + lastRFtopic;
        
        _DEBUGLN("replyRFlink2MQTT(): Received \"OK\" reply from RFlink");
        _INFO("replyRFlink2MQTT(): => Topic[");
        _INFO(Topic);
        _INFOLN("]");
        _INFO("replyRFlink2MQTT(): => payload[");
        _INFO(lastRFcommand);
        _INFOLN("]");
        
        mqttClient.publish(Topic.c_str(), lastRFcommand.c_str());

        // and now as a JSON object
        Topic = topicJSON;
        Topic += "/RFlink";
        responseObj = "{";
        responseObj += "\"Protocol\":\"RFlink\"";
        responseObj += ",\"Response\":\"" + lastRFtopic + lastRFcommand + ";\"";
        responseObj += ",\"State\":\"OK\"";
        responseObj += "}";
        
        responseObj.toCharArray(Buffer,responseObj.length()+1);
        
        _INFO("replyRFlink2MQTT(): => responseObj [");
        _INFO(Buffer);
        _INFOLN("]");
        
        mqttClient.publish(Topic.c_str(), Buffer);
        waitForOK       = 0;
        lastRFcommand   = "";
        lastRFtopic     = "";
        
        return;
    }
    //==== is het een "CMD UNKNOWN" reply? ====
    if (field[2].equals("CMD UNKNOWN")) {
        Topic   = topicState + "/" + lastRFtopic;
        
        _DEBUGLN("replyRFlink2MQTT(): Received \"CMD UNKNOWN\" reply from RFlink");
        _INFO("replyRFlink2MQTT(): => Topic[");
        _INFO(Topic);
        _INFOLN("]");
        _INFO("replyRFlink2MQTT(): => payload[");
        _INFO(lastRFcommand);
        _INFOLN("]");

        mqttClient.publish(Topic.c_str(), lastRFcommand.c_str());

        // and now as a JSON object
        Topic = topicJSON;
        Topic += "/RFlink";
        responseObj = "{";
        responseObj += "\"Protocol\":\"RFlink\"";
        responseObj += ",\"Response\":\"" + lastRFtopic + "CMD UNKNOWN;\"";
        responseObj += ",\"State\":\"ERROR\"";
        responseObj += "}";
        
        responseObj.toCharArray(Buffer,responseObj.length()+1);
        
        _INFO("replyRFlink2MQTT(): => responseObj [");
        _INFO(Buffer);
        _INFOLN("]");
        
        mqttClient.publish(Topic.c_str(), Buffer);
        waitForOK       = 0;
        lastRFcommand   = "";
        lastRFtopic     = "";

        return;

    }   // end-of-OK-CMD-UNKNOWN-reply handling

    //==== is het een "VERSION" reply? ====
    if (field[2].equals("VER")) {
        //Topic   = topicState + "/" + lastRFtopic;
        
        _INFOLN("replyRFlink2MQTT(): Received \"VERSION\" reply from RFlink");
        _INFO("replyRFlink2MQTT(): => Topic[");
        _INFO(Topic);
        _INFOLN("]");
        _INFO("replyRFlink2MQTT(): => payload[");
        _INFO(lastRFcommand);
        _INFOLN("]");

        mqttClient.publish(topicState.c_str(), Buffer);

        // and now as a JSON object
        Topic = topicJSON;
        Topic += "/RFlink";
        responseObj = "{";
        responseObj += "\"Protocol\":\"RFlink\"";
        responseObj += ",\"Version\":\"" + field[3] + "\"";
        responseObj += ",\"Revision\":\"" + field[5] + "\"";
        responseObj += ",\"Build\":\"" + field[7] + "\"";
        responseObj += "}";
        
        responseObj.toCharArray(Buffer,responseObj.length()+1);
        
        _INFO("replyRFlink2MQTT(): => responseObj [");
        _INFO(Buffer);
        _INFOLN("]");
        
        mqttClient.publish(Topic.c_str(), Buffer);
        waitForOK       = 0;
        lastRFcommand   = "";
        lastRFtopic     = "";

        return;

    }   // end-of-OK-CMD-UNKNOWN-reply handling

    _DEBUGLN("replyRFlink2MQTT(): not a \"STATE\" reply, so construct a message");

    //==== is het een "SWITCH" reply? ====
    if (field[5].equals("SWITCH")) {
        _DEBUGLN("replyRFlink2MQTT(): a \"SWITCH\" message, so make it a one-on-one reply");

        // RFlink SWITCH format: 20;01;NewKaku;ID=007949fe;SWITCH=5;CMD=ON;
        // So, remove all the fieldnames from the message and create a topic
        RFtopic     = field[2] + ";";       // protocol
        RFtopic    += field[4] + ";";       // deviceID
        RFtopic    += field[6] + ";";       // switchID
        // and a command
        RFcommand   = field[8];             // the command
        // put it all together
        Topic   = topicState + "/" + RFtopic;
        
        _INFO("replyRFlink2MQTT(): => Topic[");
        _INFO(Topic);
        _INFOLN("]");
        _INFO("replyRFlink2MQTT(): => payload[");
        _INFO(RFcommand);
        _INFOLN("]");
        
        mqttClient.publish(Topic.c_str(), RFcommand.c_str());
    
    }

        _INFOLN("replyRFlink2MQTT(): not a SWITCH, make a propper JSON message");
        _INFO("replyRFlink2MQTT(): => Received[");
        _INFO(Buffer);
        _INFOLN("]");
    
    responseObj = "{";
    ix = 2;
    protocolID    = field[ix];
    _DEBUG("replyRFlink2MQTT(): protocolID["); _DEBUG(protocolID); _DEBUGLN("]");
    /*if (ix <= fieldNum)*/ responseObj += "\"Protocol\":\""+protocolID+"\"";
    ix++;
    for(int i=ix; i<fieldNum; i++) {
        fieldID = field[i];
        responseObj += ",\"" + fieldID + "\":";
        i++;    // pont to value
        field[i].toCharArray(tmpChar,(field[i].length() +1));
        tmpInt = atoi(tmpChar);
        _DEBUG("fieldID[");    _DEBUG(fieldID);
        _DEBUG("], tmpChar["); _DEBUG(tmpChar);
        _DEBUG("], tmpInt[");  _DEBUG(tmpInt);
        _DEBUGLN("]");
        if (fieldID.equals("ID")) {           
            responseObj += "\""+field[i]+"\"";
            deviceID = field[i];
        }
        else if (fieldID.equals("SWITCH")) {            // test if it is TEMP, which is HEX
            if (atoi(tmpChar) == 0) { // not an integer
                responseObj += "\""+field[i]+"\"";
              //switchID = "\""+field[i]+"\""; ??
                switchID = field[i];
            } else { 
                responseObj += tmpInt; // do int add
                switchID = String(tmpInt);
            }
            isSwitch = true;
        }
        else if (fieldID.equals("CMD")) {   // Command ?
            responseObj += "\""+field[i]+"\"";
            setCMD = field[i];              // save new state
        } 
        else if (fieldID.equals("TEMP")) {            // test if it is TEMP, which is HEX
            tmpFloat = hextofloat(tmpChar)*0.10;   // convert from hex to float and divide by 10 - using multiply as it is faster than divide
            if (tmpFloat < _TEMP_MAX) {             // plausebility test
                responseObj += tmpFloat;
            } else {         
                responseObj += "\""+field[i]+"\"";
            }
        }
        else if (fieldID.equals("HUM")) {
            if (protocolID.equals("DKW2012")) { // digitech weather station - assume it is a hex humidity, not straight int
                responseObj += hextoint(tmpChar);
            } else {
                if (tmpInt > 0 || tmpInt <= _HUM_MAX) {
                    responseObj += tmpInt;
                } else {         
                    responseObj += "\""+field[i]+"\"";
                }
            }
        }
        else if (fieldID.equals("RAIN")) {  // test if it is RAIN, which is HEX
                responseObj += hextofloat(tmpChar)*0.10; //  - add data to root
        }
        else if (fieldID.equals("WINSP")) { // test if it is WINSP, which is HEX
                responseObj += hextofloat(tmpChar)*0.10;
        } //  - add data to root
        else if (fieldID.equals("WINGS")) { // test if it is WINGS, which is HEX
                responseObj += hextofloat(tmpChar)*0.10; //  - add data to root
        }
        else if (fieldID.equals("WINDIR")) { // test if it is WINDIR, which is 0-15 representing the wind angle / 22.5 - convert to compas text
                responseObj += "\"" + CompassDirTable[tmpInt] + "\"" ;
        } 
        else // check if an int, add as int, else add as text
            if (atoi(tmpChar) == 0) { // not an integer
                responseObj += "\""+field[i]+"\"";
            } else { 
                responseObj += tmpInt; // do int add
        }
        yield();
    }   // for loop ...
    
    responseObj += "}";

    Topic = topicJSON;
    Topic += "/";
    Topic += protocolID;
    Topic += "/";
    Topic += deviceID;
    if (isSwitch) {
        Topic += "-" + switchID;
    }
        _INFO("replyRFlink2MQTT(): publish in [");
        _INFO(Topic);
        _INFOLN("]");
        _INFO("replyRFlink2MQTT():    payLoad [");
        _INFO(responseObj);
        _INFOLN("]");
    responseObj.toCharArray(Buffer,responseObj.length()+1);
    if (!mqttClient.publish(Topic.c_str(), Buffer)) {
        _ERRORLN("replyRFlink2MQTT(): Error publishing JSON!");
        if (responseObj.length() > 199) {
            _ERRORLN("replyRFlink2MQTT(): Message to big!");
        }
    }
    
}   // replyRFlink2MQTT()


void parseFromRFlink(char* Buffer, int len) {

    if (telnetClient.connected()) { 
        Dprint("RFLNK: ");
        Dprintln(Buffer);
    }
    if (connect2MQTT)    replyRFlink2MQTT(Buffer, len);
    
    //==== empty Buffer ====
    len = 0;
    Buffer[len] = '\0';
    
}   // parseFromRFlink()


//==============================================================
//==== here we process input from MQTT and create a         ====
//==== reply to be send to the RFlink                       ====
//==== topic is:                                            ====
//==== 'RFlink/State/Set/<protocol>;<deviceID>;<switchID>;' ====
//==== payload is:                                          ====
//====     'value>'                                         ====
//==============================================================
void replyMQTT2RFlink(String mqttTopic, char* Buffer, int len) {
int         numFields;
uint8_t     lastSlash;
int8_t      isBright;
String      RFcommand, subTopic;

    Buffer[++len] = '\0';

    _INFO("replyMQTT2RFlink(): mqttTopic [");
    _INFO(mqttTopic);
    _INFOLN("]");
    _INFO("replyMQTT2RFlink(): payload [");
    _INFO(Buffer);
    _INFO("], len[");
    _INFO(len);  
    _INFOLN("]");
    
    // mqttTopic [RFlink/State/Set/TriState;8554aa;1;]
    // or
    // mqttTopic [RFlink/State/Set/subTopic/TriState;8554aa;1;]
    // first remove topicStateSet from mqttTopic
    mqttTopic.replace(topicStateSet + "/", "");
        _INFO("replyMQTT2RFlink():     bare topic [");
        _INFO(mqttTopic);
        _INFOLN("]");

    // now look for last slash ..
    lastSlash       = mqttTopic.lastIndexOf('/');
        _DEBUG("replyMQTT2RFlink(): was there a last slash? lastSlash[");
        _DEBUG(lastSlash);
        _DEBUGLN("]");

    subTopic    = "";
    if (lastSlash < 255) {
        // and remove everyting from the beginning up to the last slash
        subTopic = mqttTopic.substring(0, (lastSlash +1));
        mqttTopic.remove(0, lastSlash + 1);
        _INFO("replyMQTT2RFlink():        subTopic [");
        _INFO(subTopic);
        _INFOLN("]");
    }
    
    // mqttTopic [NewKaku;007949fe;5;]
    ///////mqttTopic;
    lastRFcommand   =  String(Buffer);
    lastRFtopic     = subTopic + mqttTopic;
    
    _INFO("replyMQTT2RFlink():     lastRFtopic[");
    _INFO(lastRFtopic);
    _INFOLN("]");
    _INFO("replyMQTT2RFlink():   lastRFcommand[");
    _INFO(lastRFcommand);
    _INFOLN("]");
    
    //==== now send Buffer straight to RFlink and wait for OK reply .. ====
    _INFO("replyMQTT2RFlink(): send to RFlink [10;");
    _INFO(mqttTopic);
    _INFO(lastRFcommand);
    _INFOLN(";]");
    
    if (telnetClient.connected()) {
        telnetClient.print("toRFlink(10;");
        telnetClient.print(mqttTopic); 
        telnetClient.print(lastRFcommand); 
        telnetClient.println(";)"); 
    }
    toRFlink("10;" + mqttTopic + lastRFcommand + ";");

    //==== empty Buffer ====
    len = 0;
    Buffer[len] = '\0';
    
}   // replyMQTT2RFlink()


//============================================================
void callbackFromMQTT(char* topic, byte* payload, unsigned int len) {
String  mqttTopic;
char    cMessage[_MAX_BUFFER];
int     inQueue;
    
    for (int i = 0; i < len; i++) {
        cMessage[i] = (char)payload[i];
        
    }
    cMessage[len]   = '\0';

    mqttTopic = String(topic);

    _INFO("callbackFromMQTT(): Message arrived [");
    _INFO(cMessage);
    _INFO("] with length [");
    _INFO(len);
    _INFOLN("]");
    _INFO("callbackFromMQTT(): in topic [");
    _INFO(mqttTopic);
    _INFOLN("]");

    if (len > 1) {
        //inQueue = pushMQTTmessage(topic, cMessage);
        if (isFull()) {
            _DEBUGLN("callbackFromMQTT(): Stack is full ..");
        } else /*if (inQueue == 0) */{
            _DEBUGLN("callbackFromMQTT(): pushed to Stack!");
            inQueue = enQueue(topic, cMessage);
            _WARN("callbackFromMQTT(): topic["); _WARN(topic);
            _WARN("], cMessage:[");              _WARN(cMessage);
            _WARN("], toSlot:[");                _WARN(toQueueSlot());
            _WARNLN("]");
            mqttTopic.replace("/Set/", "/");
            mqttClient.publish(mqttTopic.c_str(), cMessage);
            _WARN("callbackFromMQTT(): publised in topic["); _WARN(mqttTopic);
            _WARN("], cMessage:[");              _WARN(cMessage);
            _WARNLN("]");
        }
    }

    //==== empty Buffer ====
    len = 0;
    cMessage[len] = '\0';
 
}   // callbackFromMQTT()


//=================================================================================
void setup() {
    pinMode(_BLINK_LED,  OUTPUT);
    //pinMode(BUILTIN_LED,  OUTPUT);
    digitalWrite(_BLINK_LED, HIGH);  
    //digitalWrite(BUILTIN_LED, HIGH);  

    Serial.begin(57600);
    Serial.println();
    Serial.println("[ESP2RFlinkTelnet v0.7] RFlink bridge to MQTT starting..\n");
    delay(100);
    Serial.flush();

    setupWiFi();

    loadCredentials();
        if (mqttConfig.serverIP[0] == 0)
          connect2MQTT = false;
    else  connect2MQTT = true;

    topicMessage    = String(mqttConfig.topTopic) + "/Message";
    topicState      = String(mqttConfig.topTopic) + "/State";
    topicStateSet   = String(mqttConfig.topTopic) + "/State/Set";
    topicStateSetX  = String(mqttConfig.topTopic) + "/State/Set/#";
    topicDebug      = String(mqttConfig.topTopic) + "/Debug";

    Serial.println("[ESP] WiFi setup complete");
    
    if (connect2MQTT)
    {
      mqttClient.setServer(mqttConfig.serverIP, 1883);
      mqttClient.setCallback(callbackFromMQTT);
      Serial.printf("[ESP] MQTT server [%s] setup complete\r\n", mqttConfig.serverIP);
      if (strlen(mqttConfig.user) == 0)
            Serial.println("[ESP] MQTT server no username/password");
      else  Serial.printf("[ESP] MQTT server user[%s]password\r\n", mqttConfig.user, mqttConfig.passwd);
    }
    
    blinker.attach(2.0, blink);
    flashTimer = millis() + 200;
  //builtinLedTimer = millis()+1000;
    keepAlive.attach(6000.0, pingPong);

    memset(mqttStack,0,sizeof(mqttStack));
    //digitalWrite(BUILTIN_LED, LOW);  

}   // setup()


//=================================================================================
void loop() {
  //if ((int32_t)(millis() - builtinLedTimer) > 0) {
  //    builtinLedTimer = millis()+1000;
  //    digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED));
  //}

    if (millis() > reconnectTimer)
    {
      reconnectTimer = millis() + 30000;   
      if (connect2MQTT && !mqttClient.connected()) {
        reconnect2MQTT();
      }
      if (mqttClient.connected())
      {
        _DEBUG("loop(): Subscribe to [");
        _DEBUG(topicStateSetX);
        _DEBUGLN("]");
        _DEBUG("loop(): Subscribe to [");
        _DEBUG(topicStateSetX);
        _DEBUGLN("]");
        
        mqttClient.subscribe(topicStateSetX.c_str());
      }
      else if (connect2MQTT)
      {
        _ERROR("Cannot connect to [");
        _ERROR(mqttConfig.serverIP);
        _ERRORLN("]");
      }
    }
    
    if (connect2MQTT)
    {
      mqttClient.loop();
    
      if (waitForOK < millis()) 
      {
        //_DEBUG("Check stack ... [");
        //_DEBUG(waitForOK);
        //_DEBUG("] millis() [");
        //_DEBUG(millis());
        //_DEBUGLN("]");
        //int len = popMQTTmessage(sTopic, sPayLoad);
        if (!isEmpty()) {
            int len = deQueue(sTopic, sPayLoad);
            _WARN("loop(): read from Queue sTopic[");  _WARN(sTopic);
            _WARN("], sPayLoad:[");                    _WARN(sPayLoad);
            _WARNLN("]");
            if (len > 0) {
                replyMQTT2RFlink(sTopic, sPayLoad, len);
            }
            
            waitForOK = millis() + 3000;
            sTopic[0]     = '\0';
            sPayLoad[0]   = '\0';
        }
      }
    }

     // look for telnet client connect trial
    if (telnetServer.hasClient()) {
        if (!telnetClient || !telnetClient.connected()) {
            if (telnetClient) {
                DebugLvl = 0;
                telnetClient.stop();
                Serial.println("Telnet Client Stoped");
            }
            telnetClient = telnetServer.available();
            Serial.println("New Telnet client");
            Dflush();  // clear input buffer, else you get strange characters 
            telnetClient.print("\nWiFi via SSID: ");
            telnetClient.println(String(WiFi.SSID()).c_str());

            telnetClient.print("\nType \"ESP;HELP;\" for info on ESP2RFlink commands"); 
            
            telnetClient.print("\nDebugLvl is ["); 
            telnetClient.print(DebugLvl);
            telnetClient.println("]\n");
            
            telnetClient.println("Please set telnet client in 'Line' mode\n");
            
            delay(100);
            Dflush();
        }
    }

    while (Serial.available() > 0) {
        outChar = Serial.read();
        if (outBufferInx >= _MAX_BUFFER) {
            parseFromRFlink(outBuffer, outBufferInx);
            outBufferInx = 0;         
            outBuffer[outBufferInx] = '\0';   
        }
        if (outChar >= ' ' && outChar <= 'z') {
            outBuffer[outBufferInx++] = outChar;
            outBuffer[outBufferInx]   = '\0';
        } else if (outBufferInx > 0) {
            outBuffer[outBufferInx] = '\0';
            parseFromRFlink(outBuffer, outBufferInx);
            outBufferInx = 0;            
            outBuffer[outBufferInx] = '\0';   
        }
        yield();
    }
    while (telnetClient.available() > 0) {
        inChar = (char)telnetClient.read();
        //Serial.print(inChar);
        if (inBufferInx >= _MAX_BUFFER) {
            parseFromTelnet(inBuffer, inBufferInx);
            inBufferInx = 0;         
            inBuffer[inBufferInx] = '\0';   
        }
        if (inChar >= ' ' && inChar <= 'z') {
            inBuffer[inBufferInx++] = inChar;
            inBuffer[inBufferInx]   = '\0';
        } else if (inBufferInx > 0) {
            inBuffer[inBufferInx] = '\0';
            parseFromTelnet(inBuffer, inBufferInx);
            inBufferInx = 0;            
            inBuffer[inBufferInx] = '\0';   
        }
        yield();
    }

}



/***************************************************************************
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to permit
* persons to whom the Software is furnished to do so, subject to the
* following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
* OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
* THE USE OR OTHER DEALINGS IN THE SOFTWARE.
* 
****************************************************************************
*/
