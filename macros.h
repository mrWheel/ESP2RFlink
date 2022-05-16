/*
***************************************************************************
**  Program  : macros.h  (part of ESP2RFlink)
**
**  Copyright (c) 2020 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.
***************************************************************************
*/

#ifndef _MACROS_H
  #define _MACROS_H
  uint8_t     DebugLvl        = 1;           // only Errors
  const char *errorPrompt     = "ERROR: ";
  const char *alwaysPrompt    = "       ";
  const char *warnPrompt      = "WARN : ";
  const char *infoPrompt      = "INFO : ";
  const char *debugPrompt     = "DEBUG: ";
#endif

#define _MAX_QUEUE 5

#define _ERROR(...)         Debug(0, __VA_ARGS__)
#define _ERRORLN(...)       Debugln(0, __VA_ARGS__)
#define _ALWAYS(...)        Debug(1, __VA_ARGS__)
#define _ALWAYSLN(...)      Debugln(1, __VA_ARGS__)
#define _WARN(...)          Debug(2, __VA_ARGS__)
#define _WARNLN(...)        Debugln(2, __VA_ARGS__)
#define _INFO(...)          Debug(3, __VA_ARGS__)
#define _INFOLN(...)        Debugln(3, __VA_ARGS__)
#define _DEBUG(...)         Debug(4, __VA_ARGS__)
#define _DEBUGLN(...)       Debugln(4, __VA_ARGS__)

#define Debug(Lvl, ...)     {                                               \
    if (Lvl <= DebugLvl) {                              \
      switch(Lvl) {                                   \
        case 0:  Dprint(errorPrompt);  break;       \
        case 1:  Dprint(alwaysPrompt); break;       \
        case 2:  Dprint(warnPrompt);   break;       \
        case 3:  Dprint(infoPrompt);   break;       \
        default: Dprint(debugPrompt);  break;       \
      }                                               \
      if (telnetClient.connected()) {                 \
        telnetClient.print(__VA_ARGS__);            \
      } else if (Lvl < 2) {                           \
        Serial.print(__VA_ARGS__);                  \
      }                                               \
      switch(Lvl) {                                   \
        case 0:  errorPrompt  = ""; break;          \
        case 1:  alwaysPrompt = ""; break;          \
        case 2:  warnPrompt   = ""; break;          \
        case 3:  infoPrompt   = ""; break;          \
        default: debugPrompt  = ""; break;          \
      }                                               \
    }       /* end if() */                              \
  }   /* Debug(Lvl) */

#define Debugln(Lvl, ...)   {                                               \
    if (Lvl <= DebugLvl) {                              \
      switch(Lvl) {                                   \
        case 0:  Dprint(errorPrompt);  break;       \
        case 1:  Dprint(alwaysPrompt); break;       \
        case 2:  Dprint(warnPrompt);   break;       \
        case 3:  Dprint(infoPrompt);   break;       \
        default: Dprint(debugPrompt);  break;       \
      }                                               \
      if (telnetClient.connected()) {                 \
        telnetClient.println(__VA_ARGS__);          \
      } else if (Lvl < 2) {                           \
        Serial.println(__VA_ARGS__);                \
      }                                               \
      switch(Lvl) {                                   \
        case 0:  errorPrompt  = "ERROR: "; break;   \
        case 1:  alwaysPrompt = "       "; break;   \
        case 2:  warnPrompt   = "WARN : "; break;   \
        case 3:  infoPrompt   = "INFO : "; break;   \
        default: debugPrompt  = "DEBUG: "; break;   \
      }                                               \
    }       /* end if() */                              \
  }   /* Debugln(Lvl) */

#define Dprint(...)         {                                       \
    if (telnetClient.connected()) {                 \
      telnetClient.print(__VA_ARGS__);            \
    } else {                                        \
      Serial.print(__VA_ARGS__);                  \
    }                                               \
  }
#define Dprintln(...)   {                                           \
    if (telnetClient.connected()) {                 \
      telnetClient.println(__VA_ARGS__);         \
    } else {                                        \
      Serial.println(__VA_ARGS__);               \
    }                                               \
  }
#define Dflush(...)     {                                           \
    if (telnetClient.connected()) {                 \
      telnetClient.flush(__VA_ARGS__);            \
    } else {                                        \
      Serial.flush(__VA_ARGS__);                  \
    }                                               \
  }
#define toRFlink(...)   {                                           \
    Serial.println(__VA_ARGS__);                    \
    Serial.flush();                                 \
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
