#include "DFrobot_MSM261.h"

#define SAMPLE_RATE     (44100)
#define I2S_SCK_IO      (16)
#define I2S_WS_IO       (15)
#define I2S_DI_IO       (17)
#define DATA_BIT        (16)
#define MODE_PIN        (7) //Move SEL to ground
DFRobot_Microphone microphone(I2S_SCK_IO, I2S_WS_IO, I2S_DI_IO);
char i2sReadrawBuff[100];
void setup() {
  Serial.begin(115200);//Serial rate 115200
  pinMode(MODE_PIN,OUTPUT);
  digitalWrite(MODE_PIN,LOW);//Configure the microphone as receiving data of left channel
  while(microphone.begin(SAMPLE_RATE, DATA_BIT) != 0){
      Serial.println(" I2S init failed");//Init failed
  }
  Serial.println("I2S init success");//Init succeeded
}

void loop() {
  microphone.read(i2sReadrawBuff,100);
  //Output data of left channel
  Serial.println((int16_t)(i2sReadrawBuff[0]|i2sReadrawBuff[1]<<8));
  delay(100);
}