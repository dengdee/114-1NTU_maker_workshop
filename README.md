# 114-1 NTU_Maker_Workshop

---

## 簡介

本專案為 NTU Maker Workshop 練習範例，示範如何使用 ESP32(NodeMCU-32S)錄音、觸控輸入、及OLED顯示。

## 重要連結

* AI Studio API 金鑰申請(Google AI Studio):[https://aistudio.google.com/api-keys](https://aistudio.google.com/api-keys)
* Arduino ESP32 core 套件來源（(ackage index):[https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json](https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json)
* image2cpp(將影像轉為 MCU 用點陣資料):[https://javl.github.io/image2cpp/](https://javl.github.io/image2cpp/)

## 安裝 ESP32 板子(Arduino IDE 範例)

1. 打開 Arduino IDE，選擇 `File` → `Preferences`。
2. 在 "Additional Boards Manager URLs" 加入：
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
3. 打開 `Tools` → `Board` → `Boards Manager`，搜尋 `esp32` 並安裝 `esp32 by Espressif Systems`。

