#include <Wire.h>
#include <U8g2lib.h>

// 針對 SSD1306 (128x64)，硬體 I2C
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE, /* clock=*/22, /* data=*/21);

// | 字型名稱                        | 字高     | 說明                |
// | ------------------------------ | ------ | ----------------- |
// | `u8g2_font_unifont_t_chinese2` | 約 16px | 支援繁簡中文、最常用        |
// | `u8g2_font_wqy12_t_chinese1`   | 約 12px | 較小一點，但只支援部分中文字    |
// | `u8g2_font_wqy16_t_chinese1`   | 約 16px | 比 unifont 清晰一點    |
// | `u8g2_font_unifont_t_symbols`  | 約 16px | 包含符號、少部分中文        |
// | `u8g2_font_b16_t_chinese2`     | 約 16px | 更粗體的中文字體（若你有手動匯入） |



void setup() {
  u8g2.begin();
  u8g2.enableUTF8Print(); // 允許顯示 UTF-8（中文字需要）
}
void loop() {
  u8g2.clearBuffer();              // 清空畫面 buffer
  u8g2.setFont(u8g2_font_unifont_t_chinese2);  // 中文字型
  u8g2.setCursor(0, 20);
  u8g2.print("你好,世界!");
  u8g2.setCursor(0, 40);
  u8g2.print("自造者設工作坊");
  u8g2.sendBuffer();               // 將 buffer 傳到 OLED
  delay(2000);
}

