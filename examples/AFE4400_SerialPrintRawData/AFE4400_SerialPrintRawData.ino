
 
#include <SPI.h>
#include "AFE4400.h"

AFE4400 afe;//AFE_PWN,ADC_RDY,AFE_RST  //3,4,5
int32_t dataout[6] = {0};
#define ave 10
int value;

unsigned long data[6]={0};
int count = 0;

void setup() {
	Serial.begin(115200);
	afe.begin(10,3,4);   //SS,AFE_PWN,AFE_RST
        
        Serial.println(afe.diag(),BIN);  
        afe.setDefaultTiming();
        afe.setLEDCurrent(75, 75); 
        afe.setGain(0,0,0,0,0x02);
	// begin measuring

	afe.beginMeasure(false);
        Serial.println("==========REG_SRT========");
	dumpreg();
        Serial.println("==========REG_END========");
        attachInterrupt(0,readadc,RISING);
}


void loop() {

   if(count>=ave){
     detachInterrupt(0);
     for(int i=0;i<=4;i++){
     Serial.print(dataout[i]/(float)count,8);
     Serial.print(",");
      }
     Serial.println(dataout[5]/(float)count,8);
     count = 0;
     for(int i=0;i<=5;i++){
      dataout[i] =0;
    }
     attachInterrupt(0,readadc,RISING);
   }
}
void readadc(){

  afe.SPIEnableRead();
  for(int j=0;j<=5; j++){
   data[j] =  readmuti(0x2a+j);
  }
  afe.SPIDisableRead();

  for(int i=0;i<=5;i++){
    long temp = data[i]<<10;
    dataout[i] += temp/1024;
}

  count+=1;
}
void dumpreg(){
  
    for(int i=0x01;i<=0x1E; i++){
  uint32_t data = afe.SPIReadReg(i);
  Serial.print(i,HEX);
  Serial.print("==>");
  Serial.println(data,BIN);
  }
      for(int i=0x20;i<=0x23; i++){
  uint32_t data = afe.SPIReadReg(i);
  Serial.print(i,HEX);
  Serial.print("==>");
  Serial.println(data,BIN);
  }
  
}

uint32_t readmuti(byte regAddress){
   byte temp_byte = 0;
  uint32_t reg_value = 0;
  
  digitalWrite( afe.chipSelectPin, LOW);
  
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
  digitalWrite( afe.chipSelectPin, HIGH);
  return reg_value;
  
}
