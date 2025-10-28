#include <Wire.h>
#include <U8x8lib.h>

// 初始化 U8x8，這裡使用 SSD1306 I2C 介面
// 如果你是 SSD1315，也可以用 U8X8_SSD1315_128X64_NONAME_HW_I2C
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE, /* clock=*/ 22, /* data=*/ 21);

// 暫存輸入的字串
String inputText = "";

void setup() {
  Serial.begin(115200);
  u8x8.begin();        // 初始化 OLED
  u8x8.setPowerSave(0); // 開啟螢幕
  u8x8.setFont(u8x8_font_chroma48medium8_r); // 設定字型

// | 字型名稱                            | 說明                        | 每字寬×高 |
// | ---------------------------------- | -----------------           | ----- |
// | `u8x8_font_chroma48medium8_r`      | 預設字型（中等大小、清晰）    | 8×8   |
// | `u8x8_font_profont29_2x3_r`        | 超大字型（2x3格，可顯示大數字） | 16×24 |
// | `u8x8_font_5x7_f`                  | 經典小字型（輕巧）         | 5×7   |
// | `u8x8_font_px437wyse700b_2x2_r`    | 大一點的粗體字           | 16×16 |
// | `u8x8_font_amstrad_cpc_extended_f` | 復古電腦風格字型          | 8×8   |
// | `u8x8_font_artossans8_r`           | 細體英文字型            | 8×8   |
// | `u8x8_font_open_iconic_all_1x1`    | 小圖示字型（不是文字，是符號）   | 8×8   |
// | `u8x8_font_pxplusibmcgathin_f`     | IBM CGA 字型，細長型    | 8×8   |
// | `u8x8_font_7x14_1x2_r`             | 比一般大一點的兩倍高字       | 7×14  |
// | `u8x8_font_8x13B_1x2_r`            | 粗體兩倍高字            | 8×16  |
// | `u8x8_font_pxplustandynewtv_f`     | 類似早期 CRT 螢幕風格     | 8×8   |


  u8x8.clearDisplay();
  u8x8.drawString(0, 0, "Type via Serial");
  Serial.println("請在序列埠輸入文字並按 Enter");
}

void loop() {
  if (Serial.available()) {
    inputText = Serial.readStringUntil('\n'); // 讀取一行
    inputText.trim(); // 去掉前後空白字元

    // 清空畫面並顯示輸入
    u8x8.clearDisplay();
    u8x8.drawString(0, 0, "You typed:");

    // 逐行顯示（U8x8 每行最多 16 字）
    int maxCharsPerLine = 16;
    for (int i = 0; i < inputText.length(); i += maxCharsPerLine) {
      String line = inputText.substring(i, i + maxCharsPerLine);
      u8x8.drawString(0, 2 + (i / maxCharsPerLine), line.c_str());
    }

    Serial.print("顯示中：");
    Serial.println(inputText);
  }
}
