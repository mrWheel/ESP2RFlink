/*
***************************************************************************
**  Program  : queueFiFo  (part of ESP2RFlink)
**
**  Copyright (c) 2020 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.
***************************************************************************
*/

/*
** First-In First-Out queue
*/

#include "macros.h"

//#define _MAX_QUEUE 5

static int front = 0, rear = 0, noElements = 0;
stackStruct  mqttStack[_MAX_QUEUE+1];


//=================================================================================
int enQueue(char *Topic, char *payLoad)
{
  int iX;

  //if (telnetClient.connected()) {
  //    telnetClient.print("\nenQueue(): front["); telnetClient.print(front);      telnetClient.print("] ");
  //    telnetClient.print("rear[");             telnetClient.print(rear);       telnetClient.print("] ");
  //    telnetClient.print("in queue[");         telnetClient.print(noElements); telnetClient.println("]");
  //}

  rear = (rear+1)%_MAX_QUEUE;

  iX = 0;
  while(Topic[iX]  >= ' ' && Topic[iX] <= 'z')
  {
    mqttStack[rear].topic[iX] = Topic[iX];
    iX++;
  }
  mqttStack[rear].topic[iX] = '\0';
  yield();
  iX = 0;
  while(payLoad[iX]  >= ' ' && payLoad[iX] <= 'z')
  {
    mqttStack[rear].payLoad[iX] = payLoad[iX];
    iX++;
  }
  mqttStack[rear].payLoad[iX] = '\0';
  mqttStack[rear].pushTime = millis();

  noElements = noElements+1;

  //if (telnetClient.connected()) {
  //    telnetClient.print("enQueue():   Topic["); telnetClient.print(mqttStack[rear].topic);   telnetClient.println("]");
  //    telnetClient.print("enQueue(): payLoad["); telnetClient.print(mqttStack[rear].payLoad); telnetClient.println("]");
  //
  //    telnetClient.print("enQueue(): front["); telnetClient.print(front);      telnetClient.print("] ");
  //    telnetClient.print("rear[");             telnetClient.print(rear);       telnetClient.print("] ");
  //    telnetClient.print("in queue[");         telnetClient.print(noElements); telnetClient.println("]");
  //}

  return noElements;

}   // enQueue()


//=================================================================================
int deQueue(char *Topic, char *payLoad)
{
  int iX;

  //if (telnetClient.connected()) {
  //    telnetClient.print("\ndeQueue(): front["); telnetClient.print(front);       telnetClient.print("] ");
  //    telnetClient.print("rear[");             telnetClient.print(rear);        telnetClient.print("] ");
  //    telnetClient.print("in queue[");         telnetClient.print(noElements);  telnetClient.println("]");
  //}

  front = (front+1)%_MAX_QUEUE;

  iX = 0;
  while (mqttStack[front].topic[iX] != '\0')
  {
    Topic[iX] = mqttStack[front].topic[iX];
    iX++;
  }
  Topic[iX] = '\0';
  yield();
  iX = 0;
  while (mqttStack[front].payLoad[iX] != '\0')
  {
    payLoad[iX] = mqttStack[front].payLoad[iX];
    iX++;
  }
  payLoad[iX] = '\0';

  yield();
  //if (telnetClient.connected()) {
  //    telnetClient.print("deQueue(): Popped from queue [");             telnetClient.print(front);
  //    telnetClient.print("], waited["); telnetClient.print((millis() - mqttStack[front].pushTime));
  //    telnetClient.println("] milli-seconds");
  //
  //    telnetClient.print("deQueue():   Topic[");  telnetClient.print(Topic);    telnetClient.println("]");
  //    telnetClient.print("deQueue(): payLoad[");  telnetClient.print(payLoad);  telnetClient.println("]");
  //
  //    telnetClient.print("deQueue(): ==> now[");   telnetClient.print((noElements -1));
  //    telnetClient.println("] messages on Queue\n");
  //}

  memset(mqttStack[front].topic, 0, sizeof(mqttStack[front].topic));
  memset(mqttStack[front].payLoad, 0, sizeof(mqttStack[front].payLoad));

  noElements = noElements-1;

  return iX;  // lengte van payLoad

}   // deQueue()

//------ helpers ------
//=================================================================================
int isFull()
{
  return noElements == _MAX_QUEUE;

}   // isFull()

//=================================================================================
int isEmpty()
{
  return noElements == 0;

}   // isEmpty()

//=================================================================================
int inQueue()
{
  return noElements;

}   // isQueue()

//=================================================================================
int toQueueSlot()
{
  return rear;

}   // toQueueSlot()

//=================================================================================
int fromQueueSlot()
{
  return front;

}   // fromQueueSlot()



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
