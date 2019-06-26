/*
 * Copyright (c) 2015 by Thomas Trojer <thomas@trojer.net>
 * Decawave DW1000 library for arduino.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @file BasicSender.ino
 * Use this to test simple sender/receiver functionality with two
 * DW1000. Complements the "BasicReceiver" example sketch. 
 * 
 * @todo
 *  - move strings to flash (less RAM consumption)
 *  
 */
#include <SPI.h>
#include <DW1000.h>

// connection pins
const uint8_t PIN_RST = 9; // reset pin
const uint8_t PIN_IRQ = 2; // irq pin
const uint8_t PIN_SS = SS; // spi select pin

// DEBUG packet sent status and count
boolean sent = false;
volatile boolean sentAck = false;
volatile unsigned long delaySent = 0;
int16_t sentNum = 0; // todo check int type
DW1000Time sentTime;

void setup() {
  // DEBUG monitoring
  Serial.begin(9600);
  Serial.println(F("### DW1000-arduino-sender-test ###"));//set up serial monitor for debug
  // initialize the driver
  DW1000.begin(PIN_IRQ, PIN_RST);
  DW1000.select(PIN_SS);
  Serial.println(F("DW1000 initialized ..."));
  // general configuration
  DW1000.newConfiguration();//read from register
  DW1000.setDefaults();//choose to enable or disable various features here
  DW1000.setDeviceAddress(5);
  DW1000.setNetworkId(10);
  DW1000.enableMode(DW1000.MODE_LONGDATA_RANGE_LOWPOWER);//sets data rate, pulse frequency and preamble length
  DW1000.commitConfiguration();//write to DW1000 register
  Serial.println(F("Committed configuration ..."));
  // DEBUG chip info and registers pretty printed
  // check if everything is set correctly
  char msg[128];
  DW1000.getPrintableDeviceIdentifier(msg);
  Serial.print("Device ID: "); Serial.println(msg);
  DW1000.getPrintableExtendedUniqueIdentifier(msg);
  Serial.print("Unique ID: "); Serial.println(msg);
  DW1000.getPrintableNetworkIdAndShortAddress(msg);
  Serial.print("Network ID & Device Address: "); Serial.println(msg);
  DW1000.getPrintableDeviceMode(msg);
  Serial.print("Device mode: "); Serial.println(msg);
  // attach callback for (successfully) sent messages
  DW1000.attachSentHandler(handleSent);//DW1000 has a data member which is a pointer to a function which is called once 
                                       //a message is sent. This line sets that data member to handleSent() function.
  // start a transmission
  transmitter();
}

void handleSent() {
  // status change on sent success
  sentAck = true;
}

void transmitter() {
  // transmit some data
  Serial.print("Transmitting packet ... #"); Serial.println(sentNum);
  DW1000.newTransmit();//upsets DW1000 from idle mode to TX mode, which requires current of 140mA
                       //if we use Arduino to power up DW1000 directly, this requirement cannot be met
                       //and DW1000 can't enter TX mode.
  DW1000.setDefaults();//set a lot of options as default values. We can change these values in DW1000.cpp
  String msg = "Hello DW1000, it's #"; msg += sentNum;
  DW1000.setData(msg);//encode msg into register of DW1000 and transmit 
  // delay sending the message for the given amount
  DW1000Time deltaTime = DW1000Time(10, DW1000Time::MILLISECONDS);
  DW1000.setDelay(deltaTime);
  DW1000.startTransmit();//update DW1000 registers, set driver to idle mode if appropreate
  delaySent = millis();//update delaySent, which used for debug in this sketch
}

void loop() {
  if (!sentAck) {
    return; //if previous message is not sent successfully, return
  }
  // continue on success confirmation
  // (we are here after the given amount of send delay time has passed)
  sentAck = false;
  // update and print some information about the sent message
  Serial.print("ARDUINO delay sent [ms] ... "); Serial.println(millis() - delaySent);
  DW1000Time newSentTime;
  DW1000.getTransmitTimestamp(newSentTime);
  Serial.print("Processed packet ... #"); Serial.println(sentNum);
  Serial.print("Sent timestamp ... "); Serial.println(newSentTime.getAsMicroSeconds());
  // note: delta is just for simple demo as not correct on system time counter wrap-around
  Serial.print("DW1000 delta send time [ms] ... "); Serial.println((newSentTime.getAsMicroSeconds() - sentTime.getAsMicroSeconds()) * 1.0e-3);
  sentTime = newSentTime;//upeate sentTime and sentNum, which is used for dubugging in this sketch
  sentNum++; 
  // again, transmit some data
  transmitter();
}
