#include "neopixel.h"
#include "MQTT.h"
#include "oled-wing-adafruit.h"
#include "blynk.h"
#include "wire.h"
#include "SparkFun_VCNL4040_Arduino_Library.h"

SYSTEM_THREAD(ENABLED);

String subscribeTopic = "gestureLamp";
String publishTopic = "gestureLamp2";
#define VCNL 0x60

int numberOfLEDs = 12;
int LED_PIN = D8;

// below variables are received from MQTT
volatile int64_t receivedInt = 2552552552550;

volatile int redValue = 0;
volatile int blueValue = 1;
volatile int greenValue = 0;
int onAndSpin = 0;
int brightness = 0;

bool readyToUpdate = true;

void readyToUpdateFunc();

Timer readyToUpdateTimer(1000, readyToUpdateFunc, true);

int lengthOfFlashInMS = 50;
int setProximityValue = 10;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(numberOfLEDs, LED_PIN, WS2812);
VCNL4040 proximitySensor;

void callback(char *topic, byte *payload, unsigned int length);

MQTT client("lab.thewcl.com", 1883, callback);
OledWingAdafruit display;

void setup()
{
  Wire.begin();
  Serial.begin(9600);
  display.setup();
  strip.begin();
  strip.show();
  client.subscribe(subscribeTopic);
  proximitySensor.begin();
  proximitySensor.powerOnAmbient();
  proximitySensor.powerOnProximity();
  setProximityValue = proximitySensor.getProximity();
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
    client.subscribe(subscribeTopic);
  }
  uint16_t proximityToObject = proximitySensor.getProximity();
  if (proximityToObject < setProximityValue && readyToUpdate)
  {
    readyToUpdate = false;
    readyToUpdateTimer.start();
    client.publish(publishTopic, "1");
    if (receivedInt > 1)
    {
      updateColorValues();
    }
    if (onAndSpin == 2)
    {
      spinNeopixelLedsColor(0, 0, 12, lengthOfFlashInMS, numberOfLEDs);
    }
    else if (onAndSpin == 0)
    {
      changeAllNeopixelColors(redValue, greenValue, blueValue);
    }
    else
    {
      changeAllNeopixelColors(0, 0, 0);
    }
  }
  else if (readyToUpdate)
  {
    readyToUpdate = false;
    readyToUpdateTimer.start();
    client.publish(publishTopic, "0");
  }
  Serial.println(proximityToObject);
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
  updateColorValues();
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

void spinNeopixelLedsColor(int startIndex, int targetIndex, int numFlashes, int flashDuration, int NUM_LEDS)
{
  for (int i = 0; i < numFlashes; i++)
  {
    changeNeopixelColor(targetIndex, redValue, greenValue, blueValue);
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
  onAndSpin = temp % 10; // 0 is on and spin, 1 is off and spin, 2 is on and lock, 3 is off and lock
  temp = temp / 10;
  brightness = temp % 1000;
  temp = temp / 1000;
  blueValue = temp % 1000;
  temp = temp / 1000;
  greenValue = temp % 1000;
  temp = temp / 1000;
  redValue = temp % 1000;
  if (onAndSpin == 1 || onAndSpin == 3)
  {
    redValue = 0;
    greenValue = 0;
    blueValue = 0;
  }
  redValue = map(brightness, 0, 255, 0, redValue);
  greenValue = map(brightness, 0, 255, 0, greenValue);
  blueValue = map(brightness, 0, 255, 0, blueValue);
}

void readyToUpdateFunc()
{
  readyToUpdate = true;
}