#include <Arduino.h>
#include <Adafruit_GFX.h>         // Core graphics library
#include <Adafruit_ST7789.h>      // Hardware-specific library for ST7789 TFT
#include <Adafruit_LIS3DH.h>      // library for LIS3DH accelerometer
#include <Fonts/FreeSans18pt7b.h> // Font for display
#include <SPI.h>

// Pins for display
#define TFT_CS         16
#define TFT_RST        -1 // connected to board reset pin
#define TFT_DC         2
#define TFT_LITE       0

// Pins for accel
#define ACCEL_CS       15

// FIXME: This is not actually the right way to do this (should look at accel value more closely)
#define DEBOUNCE       250 // time in millis to wait after flipping screen state to do it again

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
Adafruit_LIS3DH lis = Adafruit_LIS3DH(ACCEL_CS);
sensors_event_t event;
bool cur_state;  // true if face up,
bool prev_state; // false if face down

char* get_fortune();

void setup(void) {
  Serial.begin(9600);
  pinMode(TFT_LITE, OUTPUT);
  digitalWrite(TFT_LITE, 0);
  prev_state = false; // assume we start face down

  randomSeed(analogRead(0));
  Serial.print("Setting up SPI devices...");

  tft.init(240, 240);           // Init ST7789 240x240
  lis.begin();

  Serial.println(F(" done."));

  // Add a fortune in case it's powered on face up
  tft.setTextWrap(false);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 30);
  tft.setFont(&FreeSans18pt7b);
  tft.setTextColor(ST77XX_BLUE);
  tft.println("OH NO\nNOT AGAIN");

  uint16_t time = millis();
  time = millis() - time;

  Serial.print("Ready at: ");
  Serial.println(time, DEC);

  Serial.println(get_fortune());
}

void loop() {
  static float accel_z;
  static uint16_t cur_time, prev_time = 0;
  uint16_t elapsed_time;

  cur_time = millis();
  elapsed_time = cur_time - prev_time;

  lis.getEvent(&event);
  accel_z = event.acceleration.z;
  /*
  Serial.print("X: "); Serial.println(event.acceleration.x);
  Serial.print("Y: "); Serial.println(event.acceleration.y);
  Serial.print("Z: "); Serial.println(event.acceleration.z);
  Serial.print("Heading: "); Serial.println(event.acceleration.heading);
  Serial.print("Pitch:"); Serial.println(event.acceleration.pitch);
  Serial.print("Roll: "); Serial.println(event.acceleration.roll);
  Serial.print("Status: "); Serial.println(event.acceleration.status);
  Serial.println("");
  */

  if (accel_z < 0)
  {
    cur_state = true;
    if ((prev_state == false) && (elapsed_time > DEBOUNCE))
    {
      digitalWrite(TFT_LITE, 1);
      prev_time = cur_time;
    }
  }
  else
  {
    cur_state = false;
    if ((prev_state == true) && (elapsed_time > DEBOUNCE))
    {
      // Set the fortune when the light is off
      // to avoid any flicker
      digitalWrite(TFT_LITE, 0);
      tft.setTextWrap(false);
      tft.fillScreen(ST77XX_BLACK);
      tft.drawTriangle(0, 0, 239, 0, 120, 239, ST77XX_BLUE);
      tft.setCursor(0, 30);
      tft.setTextColor(ST77XX_BLUE);
      tft.println(get_fortune());
      prev_time = cur_time;
    }
  }

  prev_state = cur_state;

  // TODO: Consider a fade-in/fade-out for the backlight using PWM
}

char* get_fortune()
{
  static char* fortunes[] = {
    "      MY EYES\n       ARE UP\n         HERE",
    "          OH\n       COME\n          ON",
    "      SCREW\n        YOU\n       HIPPY",
    "        FUCK\n       RIGHT\n         OFF",
    "         PUT\n         ME\n       DOWN",
    "       OH  NO\n         NOT\n        AGAIN",
    "             I\n        HATE\n        THIS"
  };

  static int count = sizeof(fortunes) / sizeof(char*);

  return fortunes[random(count)];
}