/**
 * 
 * @todo
 *  - move strings to flash (less RAM consumption)
 *  - fix deprecated convertation form string to char* startAsAnchor
 *  - give example description
 */
#include <SPI.h>
#include <math.h>
#include "DW1000Ranging.h"
const float Z_COORD = 0.4;
const int MEAN_RANGE = 6;
#define LEN_DATA 16
byte data[LEN_DATA];

// connection pins
const uint8_t PIN_RST = 9; // reset pin
const uint8_t PIN_IRQ = 2; // irq pin
const uint8_t PIN_SS = SS; // spi select pin
float ranges [5];
int sizeOfRanges = 0;

bool fpsTest = true; //enabling fpsTest on the anchors will affect the accuracy of ranging results
unsigned long overhead;
unsigned long currTime = 0;
uint8_t frameCount = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  //init the configuration
  DW1000Ranging.initCommunication(PIN_RST, PIN_SS, PIN_IRQ); //Reset, CS, IRQ pin
  //define the sketch as anchor. It will be great to dynamically change the type of module
  DW1000Ranging.attachNewRange(newRange);
  DW1000Ranging.attachBlinkDevice(newBlink);
  DW1000Ranging.attachInactiveDevice(inactiveDevice);
  //Enable the filter to smooth the distance
  //DW1000Ranging.useRangeFilter(true);
  
  //we start the module as an anchor
  DW1000Ranging.startAsAnchor("82:17:5B:D5:A9:9A:E2:9C", DW1000.MODE_LONGDATA_RANGE_ACCURACY, true);
  DW1000Ranging.getDistantDevice()->setShortAddress(2);
  if(fpsTest) {overhead = millis();}
}

void loop() {
  DW1000Ranging.loop();
  if(sizeOfRanges > 5 | sizeOfRanges < 0){
    sizeOfRanges = 0;  
  }
  if(fpsTest){
    currTime = millis();
    if(currTime - overhead >= 5000){
      Serial.print("Average fps in the last five seconds is: ");
      Serial.print((float) frameCount/5);Serial.print("\n"); 
      overhead = currTime;
      frameCount = 0;
      }
    }
}

void newRange() {
  /*
    double absRange = (double) DW1000Ranging.getDistantDevice()->getRange();
    double projRange = sqrt(sq(absRange) - sq(Z_COORD));
    if(sizeOfRanges < 5){
      sizeOfRanges++;
      //float value = DW1000Ranging.getDistantDevice()->getRange();
      ranges[sizeOfRanges] = projRange;
      //ranges[sizeOfRanges] = DW1000Ranging.getDistantDevice()->getRange();
    }
    else {
    float mean = 0;
    for(int a = 0; a < 5; a++){
      mean += ranges[a]; 
    }
    mean /= 5;
    sizeOfRanges = 0;
    short index = 2;
    //TODO: transmit mean range to TAG in a MEAN_RANGE message
    
    DW1000.newTransmit();
    DW1000.setDefaults();
    data[0] = MEAN_RANGE;
    data[1] = 2;
    data[2] = mean;
    DW1000.setData(data, LEN_DATA);   
    DW1000.startTransmit();
    
    Serial.print("mean range:"); Serial.print(mean); Serial.print("m\n"); 
    }
  */
  Serial.print("from: "); Serial.print(DW1000Ranging.getDistantDevice()->getShortAddress(), HEX);
  Serial.print("\t Range: "); Serial.print(DW1000Ranging.getDistantDevice()->getRange()); Serial.print(" m");
  Serial.print("\t RX power: "); Serial.print(DW1000Ranging.getDistantDevice()->getRXPower()); Serial.println(" dBm");
  if(fpsTest) {frameCount++;}
}

void newBlink(DW1000Device* device) {
  Serial.print("blink; 1 device added ! -> ");
  Serial.print(" short:");
  Serial.println(device->getShortAddress(), HEX);
}

void inactiveDevice(DW1000Device* device) {
  Serial.print("delete inactive device: ");
  Serial.println(device->getShortAddress(), HEX);
}
