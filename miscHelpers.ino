/*
***************************************************************************
**  Program  : miscHelpers  (part of ESP2RFlink)
**
**  Copyright (c) 2020 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.
***************************************************************************
*/



//=================================================================================
void blink()
{
  //toggle state
  digitalWrite(_BLINK_LED, !digitalRead(_BLINK_LED));   // toggle pin to the opposite state
  flashTimer = millis() + 100;
}   // blink()

//=================================================================================
void pingPong()
{
  Serial.println("10;PING;");
}   // pingPong()


//=================================================================================
float hextofloat(char *hexchars)
{
  return float(strtol(hexchars, NULL, 16));
}
//float hextofloat(String hexchars) {return float(strtol(hexchars.c_str()(),NULL,16));}
int hextoint(char *hexchars)
{
  return strtol(hexchars, NULL, 16);
}


//=================================================================================
uint8_t splitBuffer(char *Buffer, int len)
{
  int     fieldNum, tmpInt, ix;

  //  _DEBUG("splitBuffer(): Buffer[");
  //  _DEBUG(Buffer);
  //  _DEBUG("] len[");
  //  _DEBUG(len);
  //  _DEBUGLN("]");
  fieldNum = 0;
  field[fieldNum] = "";
  for (int i=0; i< len; i++)
  {
    if (Buffer[i] == ';' || Buffer[i] == '=' || Buffer[i] == '\n' || Buffer[i] == '\0')
    {
      fieldNum++;
      field[fieldNum] = "";
    }
    else
    {
      field[fieldNum] += Buffer[i];
    }
    yield();
  }

  _DEBUG("splitBuffer(): found [");
  _DEBUG(fieldNum);
  _DEBUGLN("] fields.");

  for (int i=0; i<fieldNum; i++)
  {
    _DEBUG("[");
    _DEBUG(i);
    _DEBUG("] \"");
    _DEBUG(field[i]);
    _DEBUGLN("\"");
  }

  return fieldNum;

}   // splitBuffer()


//=================================================================================
String macToStr(const uint8_t *mac)
{
  String result;
  for (int i = 0; i < 6; ++i)
  {
    result += String(mac[i], 16);
    if (i < 5) result += ':';
  }
  return result;
}   // macToStr()



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
