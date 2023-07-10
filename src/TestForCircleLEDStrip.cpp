/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "c:/Users/Akash/IoTCamp2023/TestForCircleLEDStrip/src/TestForCircleLEDStrip.ino"
#include "neopixel.h"
#include "MQTT.h"
#include "oled-wing-adafruit.h"
#include "blynk.h"

void setup();
void loop();
void changeNeopixelColor(int pixelNum, uint8_t red, uint8_t green, uint8_t blue);
void changeAllNeopixelColors(uint8_t red, uint8_t green, uint8_t blue);
void spinNeopixelLedsColor(int startIndex, int targetIndex, int numFlashes, int flashDuration, int NUM_LEDS, uint8_t red, uint8_t green, uint8_t blue);
void updateColorValues();
#line 6 "c:/Users/Akash/IoTCamp2023/TestForCircleLEDStrip/src/TestForCircleLEDStrip.ino"
String topicName = "gestureLamp"; // has to be changed to an agreed upon topic

int numberOfLEDs = 12;
int LED_PIN = D2;

// below variables are received from MQTT
volatile int64_t receivedInt = 0;

volatile int redValue = 0;
volatile int blueValue = 1;
volatile int greenValue = 0;
int on = 0;
int brightness = 0;

int lengthOfFlashInMS = 50;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(numberOfLEDs, LED_PIN, WS2812);

SYSTEM_THREAD(ENABLED);

void callback(char *topic, byte *payload, unsigned int length);

MQTT client("lab.thewcl.com", 1883, callback);
OledWingAdafruit display;

void setup()
{
  Serial.begin(9600);
  while (!Serial.isConnected())
  {
  }
  display.setup();
  strip.begin();
  strip.show();
  client.subscribe(topicName);
}

void loop()
{
  display.loop();
  if (client.isConnected())
  {
    client.loop();
  }
  else
  {
    client.connect(System.deviceID());
    client.subscribe(topicName);
  }
  if (receivedInt > 1)
  {
    updateColorValues();
  }
  // spinNeopixelLedsColor(0, 0, 12, lengthOfFlashInMS, numberOfLEDs, redValue, greenValue, blueValue);
  changeAllNeopixelColors(redValue, greenValue, blueValue);
  display.display();
}

void callback(char *topic, byte *payload, unsigned int length)
{
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  receivedInt = atoll(p);
  Serial.print("MQTT: ");
  Serial.println(receivedInt);
}

void changeNeopixelColor(int pixelNum, uint8_t red, uint8_t green, uint8_t blue)
{
  strip.setPixelColor(pixelNum, strip.Color(red, green, blue));
  strip.show();
}

void changeAllNeopixelColors(uint8_t red, uint8_t green, uint8_t blue)
{
  for (int i = 0; i < numberOfLEDs + 1; i++)
  {
    strip.setPixelColor(i, strip.Color(red, green, blue));
    strip.show();
  }
}

void spinNeopixelLedsColor(int startIndex, int targetIndex, int numFlashes, int flashDuration, int NUM_LEDS, uint8_t red, uint8_t green, uint8_t blue)
{
  for (int i = 0; i < numFlashes; i++)
  {
    changeNeopixelColor(targetIndex, red, green, blue);
    delay(flashDuration);

    strip.setPixelColor(targetIndex, strip.Color(0, 0, 0));
    strip.show();
    delay(flashDuration);

    targetIndex = (targetIndex + 1) % NUM_LEDS;
  }

  for (int i = NUM_LEDS - 1; i > 0; i--)
  {
    strip.setPixelColor(i, strip.getPixelColor(i - 1));
  }
  strip.setPixelColor(0, strip.Color(0, 0, 0));
  strip.show();
}

void updateColorValues()
{
  int64_t temp = receivedInt;
  on = temp % 10;
  temp = temp / 10;
  brightness = temp % 1000;
  temp = temp / 1000;
  blueValue = temp % 1000;
  temp = temp / 1000;
  greenValue = temp % 1000;
  temp = temp / 1000;
  redValue = temp % 1000;

  if (on == 1)
  {
    redValue = 0;
    greenValue = 0;
    blueValue = 0;
  }
  redValue = map(brightness, 0, 255, 0, redValue);
  greenValue = map(brightness, 0, 255, 0, greenValue);
  blueValue = map(brightness, 0, 255, 0, blueValue);
}
