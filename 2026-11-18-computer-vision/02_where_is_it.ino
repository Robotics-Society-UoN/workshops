// ============================================================
//  RoboSoc Workshop 6 - Activity 2: Where is it?
// ============================================================
//  Same as Activity 1, but now we also work out WHERE the red
//  card is: on the LEFT, in the CENTRE or on the RIGHT.
//  Left and right are from the camera's point of view, the same
//  way a robot would see it.
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
bool flipLeftRight = false;  // LEFT and RIGHT the wrong way round? set to true

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
  sensor_t *camera = esp_camera_sensor_get();
  camera->set_hmirror(camera, flipLeftRight ? 1 : 0);
  Serial.println("Camera ready!");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  startCamera();
}

// Draws where the card is, like this:  |      ##        |
void printBar(int avgX, int width) {
  int spot = avgX * 16 / width;   // 0 to 15
  Serial.print("|");
  for (int i = 0; i < 16; i++) {
    if (i == spot) Serial.print("##");
    else Serial.print(" ");
  }
  Serial.print("|  ");
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
  long sumX = 0;   // all the x positions of red pixels added up

  for (int y = 0; y < photo->height; y++) {
    for (int x = 0; x < photo->width; x++) {
      readPixel(photo, x, y, r, g, b);
      if (isRed(r, g, b)) {
        redCount = redCount + 1;
        sumX = sumX + x;
      }
    }
  }

  int width = photo->width;
  esp_camera_fb_return(photo);   // give the photo memory back

  if (redCount < minPixels) {
    Serial.println("no red");
    delay(300);
    return;
  }

  int avgX = sumX / redCount;    // the middle of the red card

  printBar(avgX, width);

  if (avgX < width / 3) {
    Serial.print("LEFT");
  } else if (avgX < width * 2 / 3) {
    Serial.print("CENTRE");
  } else {
    Serial.print("RIGHT");
  }

  Serial.print("   (x = ");
  Serial.print(avgX);
  Serial.print(", red pixels: ");
  Serial.print(redCount);
  Serial.println(")");

  delay(300);
}

// ------------------------------------------------------------
//  CHALLENGES
//  1. Split the picture into 5 zones instead of 3:
//     FAR LEFT, LEFT, CENTRE, RIGHT, FAR RIGHT.
//  2. Also work out UP, MIDDLE or DOWN.
//     Hint: add up the y positions too, just like sumX.
//  3. Print "CLOSE" when the card is near the camera.
//     Hint: the closer the card, the more red pixels there are.
//  4. Track the green or blue card instead of the red one.
//  5. Wire 3 LEDs (with 220 ohm resistors) to pins 13, 14 and 32
//     and light up the one for LEFT, CENTRE or RIGHT.
// ------------------------------------------------------------
