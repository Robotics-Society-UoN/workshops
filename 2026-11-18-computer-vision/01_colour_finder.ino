// ============================================================
//  RoboSoc Workshop 6 - Activity 1: Find a colour
// ============================================================
//  The ESP32 takes a small photo (160 x 120 pixels), checks
//  every pixel, and counts how many of them are red.
//
//  Board:     Tools > Board > esp32 > "ESP32 Wrover Module"
//  Serial Monitor: 115200 baud
// ============================================================

#include "esp_camera.h"

// ---------- Camera pins for the Freenove ESP32-WROVER CAM ----------
// (you don't need to change these)
#define PWDN_GPIO_NUM   -1
#define RESET_GPIO_NUM  -1
#define XCLK_GPIO_NUM   21
#define SIOD_GPIO_NUM   26
#define SIOC_GPIO_NUM   27
#define Y9_GPIO_NUM     35
#define Y8_GPIO_NUM     34
#define Y7_GPIO_NUM     39
#define Y6_GPIO_NUM     36
#define Y5_GPIO_NUM     19
#define Y4_GPIO_NUM     18
#define Y3_GPIO_NUM      5
#define Y2_GPIO_NUM      4
#define VSYNC_GPIO_NUM  25
#define HREF_GPIO_NUM   23
#define PCLK_GPIO_NUM   22

// ---------- Numbers you can change ----------
int redMin    = 150;   // red has to be at least this high (0 to 255)
int otherMax  = 100;   // green and blue have to be below this
int minPixels = 300;   // this many red pixels = "I can see red"

// Is this pixel red?
bool isRed(int r, int g, int b) {
  return r > redMin && g < otherMax && b < otherMax;
}

// Reads the colour of one pixel and gives back r, g and b (0 to 255)
void readPixel(camera_fb_t *photo, int x, int y, int &r, int &g, int &b) {
  int i = (y * photo->width + x) * 2;               // each pixel is 2 bytes
  uint16_t p = (photo->buf[i] << 8) | photo->buf[i + 1];
  r = ((p >> 11) & 0x1F) * 255 / 31;                // 5 bits of red
  g = ((p >> 5)  & 0x3F) * 255 / 63;                // 6 bits of green
  b = ( p        & 0x1F) * 255 / 31;                // 5 bits of blue
}

void startCamera() {
  camera_config_t config = {};
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk  = XCLK_GPIO_NUM;
  config.pin_pclk  = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href  = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn  = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_RGB565;   // raw colours, not a JPEG
  config.frame_size   = FRAMESIZE_QQVGA;    // 160 x 120 pixels
  config.jpeg_quality = 12;
  config.fb_count     = 2;
  config.fb_location  = CAMERA_FB_IN_PSRAM;
  config.grab_mode    = CAMERA_GRAB_LATEST; // always use the newest photo

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera did not start. Check the board type and camera cable.");
    while (true) delay(1000);
  }
  Serial.println("Camera ready!");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  startCamera();
}

void loop() {
  camera_fb_t *photo = esp_camera_fb_get();   // take a photo
  if (!photo) {
    Serial.println("No photo, trying again");
    delay(500);
    return;
  }

  int r, g, b;
  int redCount = 0;

  // check every pixel in the photo
  for (int y = 0; y < photo->height; y++) {
    for (int x = 0; x < photo->width; x++) {
      readPixel(photo, x, y, r, g, b);
      if (isRed(r, g, b)) {
        redCount = redCount + 1;
      }
    }
  }

  // the colour of the pixel in the very middle of the photo
  readPixel(photo, photo->width / 2, photo->height / 2, r, g, b);

  Serial.print("Middle pixel  R=");
  Serial.print(r);
  Serial.print(" G=");
  Serial.print(g);
  Serial.print(" B=");
  Serial.print(b);
  Serial.print("   red pixels: ");
  Serial.print(redCount);

  if (redCount > minPixels) {
    Serial.println("   RED SEEN");
  } else {
    Serial.println("   no red");
  }

  esp_camera_fb_return(photo);   // give the photo memory back
  delay(300);
}

// ------------------------------------------------------------
//  CHALLENGES
//  1. Hold up the green card in the middle. Write down the
//     R, G and B numbers. Then hold up the blue card.
//  2. Turn the lights off or cover the camera with your hand.
//     What happens to the numbers?
//  3. Change the code so it finds the green card instead.
//     Hint: make an isGreen() function where green is high
//     and red and blue are low.
//  4. Count red AND blue pixels at the same time and print
//     both numbers.
// ------------------------------------------------------------
