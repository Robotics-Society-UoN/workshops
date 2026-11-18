// Activity 3: press the button to take a photo.
// The LED flashes for every photo, and the Serial Monitor shows how red it was.
//
// How to run it
// 1. Unplug USB. Button across the breadboard gap: one leg to GPIO 33,
//    the opposite corner to GND.
// 2. LED long leg -> 220 ohm resistor -> GPIO 13. LED short leg -> GND.
// 3. Plug in. Tools: Board = ESP32 Wrover Module, PSRAM = Enabled, pick the Port.
// 4. Upload (hold BOOT if it sticks on "Connecting..."), then open the
//    Serial Monitor at 115200 and wait for "Camera ready!".
// 5. Press the button: the LED flashes and "Snap! red pixels: ..." prints.
//    Hold up the red card and press again: the count goes up.

#include "esp_camera.h"

const int BUTTON_PIN = 33;   // other leg of the button goes to GND
const int LED_PIN    = 13;   // LED long leg -> 220 ohm -> 13, short leg -> GND

int redMin    = 150;   // red at least this
int otherMax  = 100;   // green and blue below this

void setupCamera() {
  camera_config_t c = {};
  c.pin_pwdn = -1;  c.pin_reset = -1;  c.pin_xclk = 21;
  c.pin_sccb_sda = 26;  c.pin_sccb_scl = 27;
  c.pin_d7 = 35;  c.pin_d6 = 34;  c.pin_d5 = 39;  c.pin_d4 = 36;
  c.pin_d3 = 19;  c.pin_d2 = 18;  c.pin_d1 = 5;   c.pin_d0 = 4;
  c.pin_vsync = 25;  c.pin_href = 23;  c.pin_pclk = 22;
  c.xclk_freq_hz = 20000000;
  c.ledc_timer = LEDC_TIMER_0;  c.ledc_channel = LEDC_CHANNEL_0;
  c.pixel_format = PIXFORMAT_RGB565;
  c.frame_size = FRAMESIZE_QQVGA;   // 160 x 120
  c.fb_count = 1;
  c.fb_location = CAMERA_FB_IN_PSRAM;
  c.grab_mode = CAMERA_GRAB_LATEST;

  if (esp_camera_init(&c) != ESP_OK) {
    Serial.println("Camera failed. Check the ribbon.");
    while (true) delay(1000);
  }
  Serial.println("Camera ready!");
}

void readPixel(camera_fb_t *photo, int x, int y, int &r, int &g, int &b) {
  int i = (y * photo->width + x) * 2;
  uint16_t p = (photo->buf[i] << 8) | photo->buf[i + 1];
  r = ((p >> 11) & 0x1F) << 3;
  g = ((p >> 5) & 0x3F) << 2;
  b = (p & 0x1F) << 3;
}

bool isRed(int r, int g, int b) {
  return r > redMin && g < otherMax && b < otherMax;
}

int countRed(camera_fb_t *photo) {
  int redCount = 0;
  for (int y = 0; y < photo->height; y++) {
    for (int x = 0; x < photo->width; x++) {
      int r, g, b;
      readPixel(photo, x, y, r, g, b);
      if (isRed(r, g, b)) {
        redCount = redCount + 1;
      }
    }
  }
  return redCount;
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);   // reads LOW when pressed
  pinMode(LED_PIN, OUTPUT);
  setupCamera();
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    digitalWrite(LED_PIN, HIGH);   // flash on

    camera_fb_t *old = esp_camera_fb_get();   // throw away the old photo
    if (old) esp_camera_fb_return(old);

    camera_fb_t *photo = esp_camera_fb_get();  // snap!
    if (!photo) {
      Serial.println("Photo failed, try again");
      digitalWrite(LED_PIN, LOW);
      return;
    }
    int redCount = countRed(photo);
    esp_camera_fb_return(photo);

    delay(200);                    // keep the flash on long enough to see
    digitalWrite(LED_PIN, LOW);    // flash off

    Serial.print("Snap! red pixels: ");
    Serial.println(redCount);

    while (digitalRead(BUTTON_PIN) == LOW) delay(10);   // wait for let go
    delay(50);
  }
}
