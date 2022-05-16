/*
***************************************************************************
**  Program  : connect2MQTT  (part of ESP2RFlink)
**
**  Copyright (c) 2020 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.
***************************************************************************
*/


//=================================================================================
void reconnect2MQTT()
{
  uint16_t ncCount = 0;
  boolean connectState = false;

  blinker.attach(0.5, blink);

  // Loop until we're reconnected
  while (!mqttClient.connected() && (ncCount < 5))    // max try 4 times
  {
    if (strlen(mqttConfig.user) == 0)
    {
      _INFO("reconnect2MQTT(): Attempting MQTT connection (no username) ... ");
      connectState = mqttClient.connect((char *)mqttConfig.topTopic);
    }
    else
    {
      _INFO("reconnect2MQTT(): Attempting MQTT connection with user/password ... ");
      connectState = mqttClient.connect((char *)mqttConfig.topTopic, (char *)mqttConfig.user, (char *)mqttConfig.passwd);
    }

    if (!connectState)
    {
      _INFOLN("Connected!!");
      Dflush();
      // Once connected, publish an announcement...

      mqttClient.publish(topicMessage.c_str(), new byte[0], 0, true);
      mqttClient.publish(topicState.c_str(), new byte[0], 0, true);
      mqttClient.publish(topicStateSet.c_str(), new byte[0], 0, true);
      mqttClient.publish(topicDebug.c_str(), new byte[0], 0, true);

      mqttClient.publish(topicMessage.c_str(), "I'm reconnected again ... ;-)");
      // ... and resubscribe
      _INFO("reconnect2MQTT(): Subscribe to [");
      _INFO(topicStateSetX);
      _INFOLN("]");
      mqttClient.subscribe(topicStateSetX.c_str());
      ncCount = 0;
      return;
    }
    else
    {
      _DEBUG("mqtt username [");
      _DEBUG((char *)mqttConfig.user);
      _DEBUG("], ");
      _DEBUG("mqtt password [");
      _DEBUG((char *)mqttConfig.passwd);
      _DEBUGLN("]");
      _INFO("failed, rc=");
      _INFO(mqttClient.state());
      _INFOLN("reconnect2MQTT(): try again in 5 seconds");
      Dflush();
      // Wait 1 second1 before retrying
      delay(1000);
      pingNum   = 0;
      ncCount++;
    }
    // it will never gome so far!
    if (ncCount > 360)    // to long (30 min) no connection to MQTTserver
    {
      WiFiManager wifiManager;
      wifiManager.resetSettings();
      _ERRORLN("reconnect2MQTT(): Lost connection to MQTTserver. Reset ESP module");
      Dflush();
      delay(30000);
      ESP.reset();
      delay(5000);
    }
  }

  blinker.detach();
  blinker.attach(2.0, blink);
  flashTimer = millis() + 500;

}   // reconnect2MQTT()



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
