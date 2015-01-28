
 
#include <SPI.h>
#include "AFE4400.h"

AFE4400 afe;//AFE_PWN,ADC_RDY,AFE_RST  //3,4,5
int32_t dataout[6] = {0};
#define ave 10
int value;
char* diag_string[]={"INPSCLED","INNSCLED","INPSCGND","PDSC","PDOC","OUTNSHGND","OUTPSHGND","LEDSC","LED2OPEN","LED1OPEN","LED_ALM","PD_ALM"};
unsigned long data[6]={0};
int count = 0;

void setup() {
	Serial.begin(115200);
	afe.begin(10,3,4);   //SS,AFE_PWN,AFE_RST
Serial.println("AFE4400 Start");
        uint32_t diag_data = afe.diag();
        Serial.print("AFE4400 Diag:");
        Serial.println(diag_data,BIN);  
        int error = 0;
  for (int i = 0; i < 15; ++i)
  {
    if (bitRead(diag_data,i)==1)
    {
      Serial.println(diag_string[i]);
      error = 1;

    }

  }
  while(error){
    Serial.println("AFE4400 LED or PD connection error");
    Serial.println("Program Stopped");
   while(1){
   }
  }
  Serial.println("AFE4400 PD and LED Connection Checked");
        afe.setDefaultTiming();
        afe.setLEDCurrent(30,30); 
        afe.setGain(0x09,1,4,0,2);//AMDAC,STAGE2,GAIN,CF,RF
        /*RF 0 500K    GAIN 0  0dB      AMDAC  0 0uA ~ 10 10uA
             1 250K         1  3.5dB    CF  0 5PF
             2 100K         2  6dB          2 15PF
             3 50K          3  9.5dB        4 25PF
             4 25K          4  12dB         8 50PF  
             5 10K                          16 150PF
             6 1M                           can be added muti PF
         */
        
	afe.beginMeasure(false);//NOT PUSH-PULL
        Serial.println("==========REG_SRT========");
       	dumpreg();
        Serial.println("==========REG_END========");
        attachInterrupt(0,readadc,RISING);
  

}


void loop() {
   // readadc();
  //  delay(1000);

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
   // Serial.println(j);
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
   uint32_t temp_byte = 0;
  uint32_t reg_value = 0;
 // Serial.println("Read_START");
  digitalWrite( afe.chipSelectPin, LOW);
  
  // set address
  SPI.transfer(regAddress);
  // get first byte
  temp_byte = SPI.transfer(0x00);
  reg_value |= temp_byte << 16;
  //Serial.println(reg_value,BIN);
  // get second byte
  temp_byte = SPI.transfer(0x00);
  reg_value |= temp_byte << 8;
//Serial.println(reg_value,BIN);
  // get last byte
  temp_byte = SPI.transfer(0x00);
  reg_value |= temp_byte;
//Serial.println(reg_value,BIN);
  reg_value = (reg_value<<8)>>8;
  digitalWrite( afe.chipSelectPin, HIGH);
  return reg_value;
  
}
