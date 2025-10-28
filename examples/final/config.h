#ifndef CONFIG_H
#define CONFIG_H
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "driver/i2s.h"


// -------------------- SD Card --------------------
#define SD_CS 5

// -------------------- Touch --------------------
#define TOUCH_PIN 4

// -------------------- INMP441 I2S 麥克風 --------------------
#define I2S_WS 15
#define I2S_SCK 14
#define I2S_SD 32
#define I2S_PORT_RX I2S_NUM_0
#define SAMPLE_RATE 16000
#define BITS_PER_SAMPLE 16
#define BUFFER_SIZE 1024
extern float Gain;

// -------------------- MAX98357A I2S DAC --------------------
#define I2S_DOUT 25
#define I2S_LRC 26
#define I2S_BCLK 27
#define DAC_SD 33
#define I2S_PORT_TX I2S_NUM_1

// -------------------- OLED --------------------
#define OLED_CLOCK 22
#define OLED_DATA 21
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

// -------------------- WIFI --------------------
extern const char* ssid;
extern const char* password;
extern const char* host;  // 去掉 https://
extern const int httpsPort;
extern const char* urlPath;
extern const char* urlPath2;
#define BOUNDARY "ESP32Boundary"
#define WAV_FILE "/speech.wav"
extern const String apiKey;  // 你的 Gemini API Key



extern File file;
extern uint8_t buffer[BUFFER_SIZE];
extern size_t bytesRead;
extern int touchValue;
extern String tmp;


// 函式宣告（如果 main.ino 會呼叫這些）
void setupI2SInput();
void setupI2SOutput();
void writeWavHeader(File &file, int sampleRate, int bitsPerSample, uint32_t numSamples);
void recording();
void playAudio();
void showWords(const char *str);
void send_wav();
void send_to_llm();
void downloadTTS();


#endif
