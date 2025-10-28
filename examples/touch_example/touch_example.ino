#define TOUCH_PIN 4       // 觸控腳位
#define LED_PIN 2         // 內建 LED，一般 ESP32-S 是 GPIO2

// 觸控閾值
#define TOUCH_THRESHOLD 40

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  int touchValue = touchRead(TOUCH_PIN);
  Serial.println(touchValue);

  if (touchValue < TOUCH_THRESHOLD) {
    // 觸碰時，LED 亮
    digitalWrite(LED_PIN, HIGH);
  } else {
    // 未觸碰時，LED 滅
    digitalWrite(LED_PIN, LOW);
  }

  delay(50);  // 避免序列埠刷太快
}
