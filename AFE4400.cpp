/*
 Arduino Library for Texas Instruments AFE4400 - Pulse-Oximetry Analog Front End
 go to http://www.ti.com/product/afe4400 for datasheet
 
 See https://github.com/mogar/AFE4400 for more info
 
 
The MIT License (MIT)

Copyright (c) 2014 Morgan Redfield

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 
 */ 

#include <Arduino.h>
#include <AFE4400.h>
#include <SPI.h>

AFE4400::AFE4400() {

}

void AFE4400::sw_reset() {
   // write to control0 reg bit 3 1	
  // soft reset
  SPIWriteBit(CONTROL0, 3, true);

   // wait for bit 3 of control0 to be 0
  delay(10);
}



//
//  begin()
//  Initial SPI setup, soft reset of device
//
void AFE4400::begin(int ss,int nAFE_pwdn,int nAFE_rst) {
  chipSelectPin = ss;
  
  // set up control pins
  pinMode(nAFE_pwdn, OUTPUT);
  pinMode(nAFE_rst, OUTPUT);
  pinMode(chipSelectPin, OUTPUT);
  
  // enable control pins
  digitalWrite(nAFE_pwdn, HIGH);
  digitalWrite(nAFE_rst, HIGH);
  
  // set up SPI
  SPI.begin();
  SPI.setDataMode(SPI_MODE0);	//CPHA = CPOL = 0    MODE = 0
  sw_reset();

 }
 

 void AFE4400::setLEDCurrent(uint8_t led1_current, uint8_t led2_current) {
	
	uint32_t current_val = SPIReadReg(LEDCNTRL);

	current_val = (current_val>>16)<<16;

  current_val = current_val | (led1_current << 8);
  current_val = current_val | led2_current;
	
	SPIWriteReg(LEDCNTRL, current_val);
 }
uint32_t AFE4400::diag() {
  
  SPIWriteBit(CONTROL0, 2, true);
  delay(20);
  return SPIReadReg(DIAG);

 }
 
 void AFE4400::setDefaultTiming() {
   // set pulse repetition frequency to 500Hz, duty cycle to 25%

  // setup the timer module to default values
  writeTimingData(LED2STC, 6000);
  writeTimingData(LED2ENDC, 7999);
  writeTimingData(LED2LEDSTC, 6000);
  writeTimingData(LED2LEDENDC, 7998);
  writeTimingData(ALED2STC, 0);
  writeTimingData(ALED2ENDC, 1998);
  writeTimingData(LED1STC, 2000);
  writeTimingData(LED1ENDC, 3998);
  writeTimingData(LED1LEDSTC, 2000);
  writeTimingData(LED1LEDENDC, 3999);
  writeTimingData(ALED1STC, 4000);
  writeTimingData(ALED1ENDC, 5998);
  writeTimingData(LED2CONVST, 2);
  writeTimingData(LED2CONVEND, 1999);
  writeTimingData(ALED2CONVST, 2002);
  writeTimingData(ALED2CONVEND, 3999);
  writeTimingData(LED1CONVST, 4002);
  writeTimingData(LED1CONVEND, 5999);
  writeTimingData(ALED1CONVST, 6002);
  writeTimingData(ALED1CONVEND, 6002);
  writeTimingData(ALED1CONVEND, 7999);
  writeTimingData(ADCRSTSTCT0, 0);
  writeTimingData(ADCRSTENDCT0, 2);
  writeTimingData(ADCRSTSTCT1, 2000);
  writeTimingData(ADCRSTENDCT1, 2002);
  writeTimingData(ADCRSTSTCT2, 4000);
  writeTimingData(ADCRSTENDCT2, 4002);
  writeTimingData(ADCRSTSTCT3, 6000);
  writeTimingData(ADCRSTENDCT3, 6002);
  writeTimingData(PRPCOUNT, 7999);
 }
 
//
//  beginMeasure()
//  turn on Measurement mode - required after reset
//  be sure to set LED current before calling
//  be sure to set timing parameters before calling (e.g. with setDefaultTiming)
// 
void AFE4400::beginMeasure(bool Push_Pull) {
  // setup the LED driver (bit 11 of CONTROL2)

  SPIWriteBit(CONTROL2, 11, Push_Pull); // set it for H-Bridge mode

  // turn on LED current (bit 17 of LEDCNTRL)
  SPIWriteBit(LEDCNTRL, 17, true);

  // tri-state
  //SPIWriteBit(CONTROL2, 10, true);  //not sure why it cause reading error

  // enable the timer module (bit D8 of CONTROL1)
  SPIWriteBit(CONTROL1, 8, true);
}

void AFE4400::setGain(uint32_t ambdac,bool stage2,uint16_t stage2_gain,byte Cf,byte Rf) {
  uint32_t data = 0;
  data  |=  ambdac<<16;
  //Serial.println(data,BIN);
  if (stage2){
   bitWrite(data,14,true);
   data |=stage2_gain<<8;
  }
 
  data |=Cf<<3;

  data |=Rf;

  SPIWriteReg(TIA_AMB_GAIN,data);
}

int AFE4400::readPulseData(){
  return pulse;
}

int AFE4400::readOxData(){
  return oximetry;
}

// Basic SPI routines to simplify code
// read and write one register
void AFE4400::SPIWriteBit(byte regAddress, uint8_t bit, bool bit_high) {
	// read the reg
	uint32_t current_val = SPIReadReg(regAddress);
	bitWrite(current_val,bit,bit_high);
	
	SPIWriteReg(regAddress, current_val);

	
}


uint32_t AFE4400::SPIReadReg(byte regAddress){
  uint32_t temp_byte = 0;
  uint32_t reg_value = 0;
  
  
  // enable reading from registers
  SPIEnableRead();
  
  digitalWrite(chipSelectPin, LOW);
  
  // set address
  SPI.transfer(regAddress);
  // get first byte
  temp_byte = SPI.transfer(0x00);
  reg_value |= temp_byte << 16;

  // get second byte
  temp_byte = SPI.transfer(0x00);
  reg_value |= temp_byte << 8;

  // get last byte
  temp_byte = SPI.transfer(0x00);
  reg_value |= temp_byte;

   reg_value = (reg_value<<8)>>8;
  digitalWrite(chipSelectPin, HIGH);
  
  // disable reading from registers
  SPIDisableRead();

  return reg_value;
}

void AFE4400::SPIEnableRead() {
	SPIWriteReg(CONTROL0, 1);
	//delay(1);
}

void AFE4400::SPIDisableRead() {
	SPIWriteReg(CONTROL0, 0);
//	delay(1);
}

void AFE4400::SPIWriteReg(byte regAddress, uint32_t regValue){
  byte temp_byte = 0;
  
  digitalWrite(chipSelectPin, LOW);
  
  SPI.transfer(regAddress);  // write instruction
  temp_byte = regValue >> 16;
  SPI.transfer(temp_byte);
  temp_byte = regValue >> 8;
  SPI.transfer(temp_byte);
  temp_byte = regValue;
  SPI.transfer(temp_byte);
  
  digitalWrite(chipSelectPin, HIGH);
}

void AFE4400::writeTimingData(byte regAddress, uint16_t timing_value) {
	if (regAddress < LED2STC || regAddress > PRPCOUNT) {
		if (debugSerial) {Serial.println("Error: timing address out of range"); }
		return;
	}
	uint32_t reg_val = timing_value;
	
	// write the value
	SPIWriteReg(regAddress, reg_val);
}

