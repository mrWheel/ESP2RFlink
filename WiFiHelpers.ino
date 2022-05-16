/*
***************************************************************************
**  Program  : WiFiHelpers  (part of ESP2RFlink)
**
**  Copyright (c) 2020 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.
***************************************************************************
*/

void resetESP()
{
  WiFiManager wifiManager;
  delay(5000);
  ESP.reset();
  delay(5000);

}   // blink()

//=================================================================================
//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager)
{
  //blinker.attach(5.0, blink);

  _INFO("configModeCallback(): Entered config mode @");
  _INFO(WiFi.softAPIP());
  _INFOLN(" with SSID: ");
  //if you used auto generated SSID, print it
  _INFOLN(myWiFiManager->getConfigPortalSSID());

}   // configModeCallback()


//=================================================================================
void setupWiFi()
{
  digitalWrite(_BLINK_LED, HIGH);

  loadCredentials();

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  if (!strcasecmp("none", mqttConfig.topTopic) == 0)
  {
    if (mqttConfig.serverIP[0] == 0 || mqttConfig.user[0] == 0 || mqttConfig.passwd[0] == 0)
    {
      wifiManager.resetSettings();
    }
  }
  wifiManager.setConfigPortalTimeout(360);
  WiFiManagerParameter custom_mqtt_server("server", "mqtt serverIP", mqttConfig.serverIP, 30);
  wifiManager.addParameter(&custom_mqtt_server);
  WiFiManagerParameter custom_mqtt_user("user", "mqtt User", mqttConfig.user, 16);
  wifiManager.addParameter(&custom_mqtt_user);
  WiFiManagerParameter custom_mqtt_passwd("password", "mqtt password", mqttConfig.passwd, 16);
  wifiManager.addParameter(&custom_mqtt_passwd);
  WiFiManagerParameter custom_mqtt_topTopic("topTopic", "mqtt topTopic ('none' for no MQTT)", mqttConfig.topTopic, 16);
  wifiManager.addParameter(&custom_mqtt_topTopic);
  wifiManager.setRemoveDuplicateAPs(true);
  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);
  Dflush();

  //==== if wifiManager.autoConnect takes longer then 5 minuts,   ====
  //==== reset the ESP and hope for the best. Else, reset after   ====
  //==== 5 minuts again                                           ====
  autoConnectTimeOut.attach(300, resetESP);

  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("ESP2RFlink");

  autoConnectTimeOut.detach();

  //if you get here you have connected to the WiFi
  _INFOLN("connected...yeey :)");
  Dflush();
  digitalWrite(_BLINK_LED, LOW);

  //read updated parameters
  strcpy(mqttConfig.serverIP, custom_mqtt_server.getValue());
  strcpy(mqttConfig.user, custom_mqtt_user.getValue());
  strcpy(mqttConfig.passwd, custom_mqtt_passwd.getValue());
  strcpy(mqttConfig.topTopic, custom_mqtt_topTopic.getValue());

  saveCredentials();

  topicMessage    = String(mqttConfig.topTopic) + "/Message";
  topicState      = String(mqttConfig.topTopic) + "/State";
  topicStateSet   = String(mqttConfig.topTopic) + "/State/Set";
  topicJSON       = String(mqttConfig.topTopic) + "/JSON";
  topicDebug      = String(mqttConfig.topTopic) + "/Debug";

  //Serial.print(F("setupWiFi(): Free Heap[B]: "));
  //Serial.println(ESP.getFreeHeap());
  //Serial.print(F("setupWiFi(): Connected to SSID: "));
  //Serial.println(WiFi.SSID());
  //Serial.print(F("setupWiFi(): IP address: "));
  //Serial.println(WiFi.localIP());

  _INFO(    "setupWiFi(): Free Heap[B]: ");
  _INFOLN(ESP.getFreeHeap());
  _ALWAYS(  "setupWiFi(): Connected to SSID: ");
  _ALWAYSLN(WiFi.SSID());
  _ALWAYS(  "setupWiFi(): IP address: ");
  _ALWAYSLN(WiFi.localIP());
  Dflush();
  // Generate client name based on topTopic and 16 bits of microsecond counter
  clientName  = String(mqttConfig.topTopic);
  clientName += String(micros() & 0xff, 16);

  if (!strcasecmp("none", mqttConfig.topTopic) == 0)
  {
    Dprint(  "setupWiFi(): Connected to ");
    Dprint(  mqttConfig.serverIP);
    Dprint(  " as ");
    Dprint(  clientName);
    Dprint(  " with [");
    Dprint(  mqttConfig.user);
    Dprint(  "]/[");
    Dprint(  mqttConfig.passwd);
    Dprintln("] ");
  }
  else
  {
    Dprintln("no MQTT server configured!");
  }
  Dflush();
  delay(100);

  telnetServer.begin();
  telnetServer.setNoDelay(false);
  Serial.println("Please connect Telnet Client, exit with ^] and 'quit'");

}   // setupWiFi()


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
