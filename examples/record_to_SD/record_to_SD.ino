// for SD card
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#define SD_CS   5

//for inmp441 mic
#include "driver/i2s.h"
#define I2S_WS  15     // LRCL
#define I2S_SCK 14     // BCLK
#define I2S_SD  32     // DOUT
#define I2S_PORT I2S_NUM_0
#define SAMPLE_RATE 16000
#define BITS_PER_SAMPLE 16
#define RECORD_TIME 5   // 錄音秒數
#define BUFFER_SIZE 1024

void setupI2S() {
  const i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = (i2s_bits_per_sample_t)BITS_PER_SAMPLE,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 1024,
    .use_apll = false
  };

  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
  i2s_zero_dma_buffer(I2S_PORT);
}

void writeWavHeader(File file, int sampleRate, int bitsPerSample, int numSamples) {
  int byteRate = sampleRate * bitsPerSample / 8;
  int dataSize = numSamples * bitsPerSample / 8;

  file.write((const uint8_t*)"RIFF", 4);
  int chunkSize = dataSize + 36;
  file.write((const uint8_t*)&chunkSize, 4);
  file.write((const uint8_t*)"WAVEfmt ", 8);

  int subchunk1Size = 16;
  short audioFormat = 1;
  short numChannels = 1;
  file.write((const uint8_t*)&subchunk1Size, 4);
  file.write((const uint8_t*)&audioFormat, 2);
  file.write((const uint8_t*)&numChannels, 2);
  file.write((const uint8_t*)&sampleRate, 4);
  file.write((const uint8_t*)&byteRate, 4);

  short blockAlign = bitsPerSample / 8;
  file.write((const uint8_t*)&blockAlign, 2);
  file.write((const uint8_t*)&bitsPerSample, 2);

  file.write((const uint8_t*)"data", 4);
  file.write((const uint8_t*)&dataSize, 4);
}


void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Initializing SD card...");
  if (!SD.begin(SD_CS)) {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card ready.");

  setupI2S();

  File file = SD.open("/record.wav", FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing!");
    return;
  }

  Serial.println("Recording...");
  int numSamples = SAMPLE_RATE * RECORD_TIME;
  writeWavHeader(file, SAMPLE_RATE, BITS_PER_SAMPLE, numSamples);

  uint8_t buffer[BUFFER_SIZE];
  size_t bytesRead;

  for (int i = 0; i < (numSamples * 2) / BUFFER_SIZE; i++) {
    i2s_read(I2S_PORT, buffer, BUFFER_SIZE, &bytesRead, portMAX_DELAY);
    file.write(buffer, bytesRead);
  }

  file.close();
  Serial.println("Saved as /record.wav");
}

void loop() {}
