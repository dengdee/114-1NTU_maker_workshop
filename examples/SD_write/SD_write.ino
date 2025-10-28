#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define SD_CS 5  // 根據實際接線修改

void setup() {
  Serial.begin(115200);
  if(!SD.begin(SD_CS)){
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialized.");

  File file = SD.open("/test.txt", FILE_WRITE);
  if(file){
    file.println("Hello ESP32 SD card!");
    file.close();
    Serial.println("File written.");
  } else {
    Serial.println("Failed to open file for writing.");
  }

  // 讀回來看看
  file = SD.open("/test.txt");
  if(file){
    Serial.println("File content:");
    while(file.available()){
      Serial.write(file.read());
    }
    file.close();
  } else {
    Serial.println("Failed to open file for reading.");
  }
}

void loop() {}
