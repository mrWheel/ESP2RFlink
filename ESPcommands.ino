/*
***************************************************************************
**  Program  : ESPcommands  (part of ESP2RFlink)
**
**  Copyright (c) 2020 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.
***************************************************************************
*/


//=================================================================================
void parseESPcommand(char *Buffer, int len)
{
  int     tmpInt;
  char    tmpChar[10];

  Dprintln("\n==================================================");
  Dprint("[ESP] command found [");
  Dprint(Buffer);
  Dprintln("]\n");
  if (len >= 11 && (strncasecmp("ESP;DEBUG=", Buffer, 10) == 0 ))
  {
    memcpy(tmpChar, Buffer + 10 /* Offset */, 1 /* Length */);
    tmpChar[1] = '\0'; /* Add terminator */
    if (tmpChar[0] == '?')
    {

    }
    else
    {
      tmpInt = atoi(tmpChar);
      DebugLvl = tmpInt;
    }
    if (DebugLvl < 0) DebugLvl = 0;
    if (DebugLvl > 4) DebugLvl = 4;
    Dprint("[ESP] DebugLvl is [");
    Dprint(DebugLvl);
    Dprint("] - ");
    switch(DebugLvl)
    {
      case 0:
        Dprintln("     only Errors ");
        break;
      case 1:
        Dprintln("     Errors,  and Always ");
        break;
      case 2:
        Dprintln("     Errors, Always and Warnings ");
        break;
      case 3:
        Dprintln("     Errors, Always, Warnings and Info ");
        break;
      default:
        Dprintln("     Errors, Always, Warnings, Info and Debug ");
    }
  }
  else if (len >= 9 && (strncasecmp("ESP;INFO;", Buffer, 9) == 0 ))
  {
    Dprintln("[ESP] Info:");
    Dprint("    Hostname : [");
    Dprint(_HOSTNAME);
    Dprintln("]");
    Dprint("  FW Version : [");
    Dprint(_FW_VERSION);
    Dprintln("]\r\n");
    Dprintln("[ESP] Recovered credentials:");
    Dprint(" MQTT server IP : [");
    Dprint(mqttConfig.serverIP);
    Dprintln("]");
    Dprint("      MQTT user : [");
    Dprint(mqttConfig.user);
    Dprintln("]");
    Dprint("  MQTT password : [");
    Dprint(mqttConfig.passwd);
    Dprintln("]");
    Dprint(" MQTT top topic : [");
    Dprint(mqttConfig.topTopic);
    Dprintln("]");
    Dprint("\nWiFi via SSID: [");
    Dprint(String(WiFi.SSID()).c_str());
    Dprintln("]");
    Dprint("  WiFi via IP: [");
    Dprint(WiFi.localIP());
    Dprintln("]");
    Dprintln("\nMQTT Topics:");
    Dprint("    topicState [");
    Dprint(topicState);
    Dprintln("]");
    Dprint(" topicStateSet [");
    Dprint(topicStateSet);
    Dprintln("]");
    Dprint("topicStateSetX [");
    Dprint(topicStateSetX);
    Dprintln("]");
    Dprint("     topicJSON [");
    Dprint(topicJSON);
    Dprintln("]");
    Dprint("  topicMessage [");
    Dprint(topicMessage);
    Dprintln("]");
    Dprint("    topicDebug [");
    Dprint(topicDebug);
    Dprintln("]");
    tmpInt = DebugLvl;
    DebugLvl = 0;
    Serial.println("10;VERSION;");
    DebugLvl = tmpInt;
    Dprintln("==================================================\n");
  }
  else if (len >= 10 && (strncasecmp("ESP;RESET;", Buffer, 10) == 0 ))
  {
    WiFiManager wifiManager;
    wifiManager.resetSettings();
    Dprintln("[ESP] Reseting stored WiFi credentials.");
    Dprintln(" ===> ESP2RFlink will reboot ...");
    Dprintln("==================================================\n");
    Dflush();
    delay(5000);
    ESP.reset();
    delay(5000);
  }
  else
  {
    Dprintln("[ESP] valid commands are:");
    Dprintln("      ESP;DEBUG={?,0,1,2,3,4};");
    Dprintln("      ESP;INFO;");
    Dprintln("      ESP;RESET;");
    Dprintln("==================================================\n");
  }

}   // parseESPcommand()


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
