#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "config.h"  // 引用圖片設定檔

// 初始化 OLED
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);


void setup() {
	u8g2.begin();
	u8g2.clearBuffer();
	// 顯示圖片
	u8g2.drawXBMP(0, 0, LOGO_WIDTH, LOGO_HEIGHT, picAllArray[0]);
	u8g2.sendBuffer();
	delay(1000);
}

void loop() {
	for (int i = 0; i <10 ; i++) {
		u8g2.clearBuffer();
		u8g2.drawXBMP(0, 0, LOGO_WIDTH, LOGO_HEIGHT, picAllArray[i]);
		u8g2.sendBuffer();
		delay(500);
	}
	delay(5000);
}
