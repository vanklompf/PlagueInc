#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266mDNS.h>
#include <TFT_eSPI.h>
#include <SPI.h>

#include <Ticker.h>

#include "InetWeather.h"


static const int GPIO_DHT_DATA = 5;
static const int GPIO_DHT_3V = 16;
static const int GPIO_DHT_GND = 4;

static const int GPIO_LED_GREEN = 10;
static const int GPIO_LED_RED = 12;
//static const int GPIO_LED_BLUE = 0;

Ticker myTimer;

static const int TEMP_ERROR = 0xFFFF;

static MDNSResponder mdns;

static InetWeather inetWeather(Serial);
static Print& logger = Serial;

static TFT_eSPI tft;
static ESP8266WiFiMulti wifiMulti;


static void initWifi();
static void checkWifi();
static void initLcd();
static void initLed();

static void drawSplashscreen();
static void showCountry(int countryCode);

static void animateProgressBar();
static void flipLed();


void setup(void)
{
  Serial.begin(115200);
  Serial.println("");

  initLcd();
  initLed();
  initWifi();
  inetWeather.init();
  drawSplashscreen();
}

void loop(void)
{
  uint8_t rot = 0;
  for (int i=0; i<100; i++)
  {
    checkWifi();

    showCountry(0);
    animateProgressBar();
    showCountry(1);
    animateProgressBar();
  }
}

void initLcd()
{
  logger.printf("initLcd()");
  tft.init();
  tft.fillScreen(TFT_LIGHTGREY);
  tft.setRotation(1);
  tft.setTextSize(1);

  logger.printf("TFT_WIDTH: %d\n", TFT_WIDTH);
  logger.printf("TFT_HEIGHT: %d\n", TFT_HEIGHT);
#ifdef ILI9163_DRIVER
  logger.println("Using ILI9163_DRIVER");
#endif
}

void initLed()
{
  logger.println(__PRETTY_FUNCTION__);
  pinMode(GPIO_LED_GREEN, OUTPUT);
  digitalWrite(GPIO_LED_GREEN, 1);

  pinMode(GPIO_LED_RED, OUTPUT);
  digitalWrite(GPIO_LED_RED, 1);

  //pinMode(GPIO_LED_BLUE, OUTPUT);
  //digitalWrite(GPIO_LED_BLUE, 1);
}

void drawSplashscreen()
{
  logger.println(__PRETTY_FUNCTION__);
  for(int32_t x=0; x<128; x++)
  {
      tft.drawFastVLine(x, 0, 128, TFT_BLUE);
      delay(10);
  }

  for(int32_t x=0; x<128; x++)
  {
      tft.drawFastHLine(0, x, 128, TFT_RED);
      delay(10);
  }
}

void initWifi()
{
  logger.println(__PRETTY_FUNCTION__);
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("gacoperek", "fedjp5cwcaZ6");
}

void checkWifi()
{
  logger.println(__PRETTY_FUNCTION__);
  static bool checked = false;

  if (!checked)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      checked = true;

      Serial.print("Connected to ");
      Serial.println(WiFi.SSID().c_str());
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());

      if (mdns.begin("esp8266", WiFi.localIP())) 
      {
        Serial.println("MDNS responder started");
      }        
    }
  }
}

void flipLed()
{
  static int greenState = 1;
  static int redState = 0;
  static int blueState = 1;

  greenState = 1 - greenState;
  redState = 1 - redState;
  blueState = 1 - blueState;
  digitalWrite(GPIO_LED_GREEN, greenState);
  digitalWrite(GPIO_LED_RED, redState);
  //digitalWrite(GPIO_LED_BLUE, blueState);
}


void animateProgressBar()
{
  for (uint32_t color : {TFT_RED, TFT_GREEN, TFT_BLUE, TFT_NAVY})
  {
    for(int32_t i = 0; i < 128; i++)
    {
      tft.fillRect(0, 75, i, 5, color);
      delay(10);
    }
    flipLed();
  }
}

static void showCountryScreen(uint32_t bgColor, const char* countryName, int infected, int cured)
{
  tft.fillScreen(bgColor);
  tft.setTextColor(TFT_BLACK, bgColor);  
  tft.setCursor(0, 0, 4);
  tft.println(countryName);

  tft.setTextColor(TFT_BLACK, bgColor);  
  tft.setTextFont(infected > 9999 ? 4 : 7);

  if (infected < 0)
    tft.print("...");
  else
    tft.print(infected);

  tft.setTextFont(6);
  tft.setTextColor(TFT_DARKGREEN, bgColor);
  tft.setCursor(0, 82, 4);
  if (infected < 0)
    tft.print("...");
  else
    tft.print(cured);
}

static void showCountry(int country)
{
  if (country == 1)
    showCountryScreen(TFT_WHITE, "Poland", 238, 13);
  else
    showCountryScreen(TFT_WHITE, "Italy", 31506, 2941);
}
