#include <EEPROM.h>

///////////////////////////////////
// Boost Display
///////////////////////////////////
//Pin connected to ST_CP of 74HC595 (RCLK)
int latchPin = 3;
//Pin connected to SH_CP of 74HC595 (SCLK)
int clockPin = 5;
////Pin connected to DS of 74HC595 (DIO)
int dataPin = 4;

///////////////////////////////////
// Pressure Sensor
///////////////////////////////////
///Connected to vout of the pressure sensor
int sensorPin = A0;
int sensorValue = 0;
//Current sensor pressure reading
float currentPressure = 0.0;
int formattedPressure = 0;
// The pressure offset is set up to be calibrated but
// honestly it could just be set statically to 310 and it would be pretty accurate.
int pressureOffset = 0;

//////////////////////////////////
// Pressure Calibration Button
//////////////////////////////////
//Calibration button pin
int calButtonPin = 2;


byte lcdMapping[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};
byte digitMapping[] = {0x01,0x02,0x04,0x08};

void setup() {
  //set pins to output so you can control the shift register
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  //set up the calibration button
  pinMode(calButtonPin, INPUT);
  //set internal pullup resistor
  digitalWrite(calButtonPin, HIGH);
  //retrieve pressure offset from ROM
  retrievePressureOffset();
}

///////////////////////////////////////////
// Read pressure and update boost display
///////////////////////////////////////////
void readAndUpdatePressure() {
  sensorValue = analogRead(sensorPin) - pressureOffset;
  //process raw value into pressure
  currentPressure = (0.035445 * sensorValue);
  formattedPressure = (int)(currentPressure*10);
  //display the pressure
  if(formattedPressure >= 0){
    displayDigit(formattedPressure, false);
  } else {
    displayDigit(-1*formattedPressure, true);
  }
}

/////////////////////////////////////
// Display digit on boost gauge
/////////////////////////////////////
void displayDigit(int inDigit, boolean negative) {
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, lcdMapping[inDigit % 10]);
  shiftOut(dataPin, clockPin, MSBFIRST, digitMapping[0]);
  digitalWrite(latchPin, HIGH);
  inDigit /= 10;
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, lcdMapping[inDigit % 10] + 128);
  shiftOut(dataPin, clockPin, MSBFIRST, digitMapping[1]);
  digitalWrite(latchPin, HIGH);
  inDigit /= 10;
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, lcdMapping[inDigit % 10]);  //place the decimal point
  shiftOut(dataPin, clockPin, MSBFIRST, digitMapping[2]);
  digitalWrite(latchPin, HIGH);
  if (negative) {
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, 0xbf);  //place the negative sign
    shiftOut(dataPin, clockPin, MSBFIRST, digitMapping[3]);
    digitalWrite(latchPin, HIGH);
  }
}

////////////////////////////////////////////////////////////
//Read the stored pressure offset in from the ROM
////////////////////////////////////////////////////////////
void retrievePressureOffset() {
  byte lower = EEPROM.read(0);
  byte upper = EEPROM.read(1);
  pressureOffset = upper;
  pressureOffset = pressureOffset << 8;
  pressureOffset |= lower;
}

////////////////////////////////////////////////////////////////
//Read the calibration button and recalibrate zero if necessary
////////////////////////////////////////////////////////////////
void readCalibrateButton() {
  //Read button
  if(digitalRead(calButtonPin) == LOW){
    //Re-zero pressure
    int tempValue = analogRead(sensorPin);
    //update pressure zero value in ROM
    EEPROM.write(0, lowByte(tempValue));
    EEPROM.write(1, highByte(tempValue));
    retrievePressureOffset();
  }
}


/////////////////////////////////////
// Main Loop
/////////////////////////////////////
void loop() {
  // Read pressure voltage
  readAndUpdatePressure();
  readCalibrateButton();

}
