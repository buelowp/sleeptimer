/*
 * Project sleeptimer
 * Description:
 * Author:
 * Date:
 */
#include <neopixel.h>
#include <MQTT.h>

#define NUM_LEDS        40
#define PIXEL_PIN       D3
#define GREEN_BTN       D11
#define YELLOW_BTN      D13
#define CROSS             1
#define SQUARE            2
#define CIRCLE            3
#define RECTANGLE         4
#define BLANK             5
#define START             0
#define CST_OFFSET      	-6
#define DST_OFFSET      	(CST_OFFSET + 1)
#define TIME_BASE_YEAR      2019
#define APP_ID              18
#define BRIGHTNESS          20

void mqttCallback(char*, byte*, unsigned int);

char g_name[] = "sleeptimer";
int g_circle[] = {2, 3, 4, 5, 9, 14, 17, 22, 26, 27, 28, 29, 99};
int g_square[] = {2, 3, 4, 5, 10, 13, 18, 21, 26, 27, 28, 29, 99};
int g_cross[] = {0, 1, 6, 7, 10, 11, 12, 13, 18, 19, 20, 21, 24, 25, 30, 31, 99};
int g_rectangle[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 15, 16, 23, 24, 25, 26, 27, 28, 29, 30, 31, 99};
const uint8_t _usDSTStart[] = {10, 8,14,13,12,10, 9, 8,14,12,11,10, 9,14,13,12,11, 9};
const uint8_t _usDSTEnd[]   = {3, 1, 7, 6, 5, 3, 2, 1, 7, 5, 4, 3, 2, 7, 6, 5, 4, 2};
String g_mqttName = g_name + System.deviceID().substring(0, 8);
char g_hostname[] = "172.24.1.13";

Adafruit_NeoPixel wing(NUM_LEDS, PIXEL_PIN, WS2812B);
bool g_timeSyncDone;
bool g_mqttConnected;
int g_displayState;
int g_bright;
int g_offset;
int g_appId;
int g_buttonCount;
system_tick_t g_debounce;
MQTT client(g_hostname, 1883, mqttCallback);

void mqttCallback(char* topic, byte* payload, unsigned int length) 
{
    char p[length + 1];
    memcpy(p, payload, length);
    p[length + 1] = '\0';
}

int currentTimeZone()
{
    g_offset = CST_OFFSET;
    Time.endDST();

    if (Time.month() > 3 && Time.month() < 11) {
        Time.beginDST();
        g_offset = DST_OFFSET;
    }
    else if (Time.month() == 3) {
        if ((Time.day() == _usDSTStart[Time.year() -  TIME_BASE_YEAR]) && Time.hour() >= 2) {
            Time.beginDST();
            g_offset = DST_OFFSET;
        }
        else if (Time.day() > _usDSTStart[Time.year() -  TIME_BASE_YEAR]) {
            Time.beginDST();
            g_offset = DST_OFFSET;
        }
    }
    else if (Time.month() == 11) {
        if ((Time.day() == _usDSTEnd[Time.year() -  TIME_BASE_YEAR]) && Time.hour() <=2) {
            Time.beginDST();
            g_offset = DST_OFFSET;
        }
        else if (Time.day() > _usDSTEnd[Time.year() -  TIME_BASE_YEAR]) {
            Time.endDST();
            g_offset = CST_OFFSET;
        }
    }

    return g_offset;
}

bool inRange(int low, int high)
{
  int x = Time.hour();
  return  ((x-low) <= (high-low)); 
}

bool isWeekday()
{
  if (Time.weekday() == 1 || Time.weekday() == 7)
    return false;

  return true;
}

void drawCircle()
{
  if (g_displayState != CIRCLE) {
    g_displayState = CIRCLE;
    int index = 0;
    for (int i = 0; i < 32; i++) {
      if (g_circle[index] == i) {
        wing.setPixelColor(i, 0, 0, g_bright);
        index++;
      }
      else {
        wing.setPixelColor(i, 0, 0, 0);
      }
    }
    wing.show();
  }
}

void drawSquare()
{
  if (g_displayState != SQUARE) {
    g_displayState = SQUARE;
    int index = 0;
    for (int i = 0; i < 32; i++) {
      if (g_square[index] == i) {
        wing.setPixelColor(i, g_bright, g_bright, 0);
        index++;
      }
      else {
        wing.setPixelColor(i, 0, 0, 0);
      }
    }
    wing.show();
  }
}

void drawRectangle()
{
  if (g_displayState != RECTANGLE) {
    g_displayState = RECTANGLE;
    int index = 0;
    for (int i = 0; i < 32; i++) {
      if (g_rectangle[index] == i) {
        wing.setPixelColor(i, 0, g_bright, 0);
        index++;
      }
      else {
        wing.setPixelColor(i, 0, 0, 0);
      }
    }
    wing.show();
  }
}

void drawCross()
{
  if (g_displayState != CROSS) {
    g_displayState = CROSS;
    int index = 0;
    for (int i = 0; i < 32; i++) {
      if (g_cross[index] == i) {
        wing.setPixelColor(i, g_bright, 0, 0);
        index++;
      }
      else {
        wing.setPixelColor(i, 0, 0, 0);
      }
    }
    wing.show();
  }
}

void drawBlank()
{
  if (g_displayState != BLANK) {
    g_displayState = BLANK;
    wing.clear();
    wing.show();
  }  
}

int setBrightness(String b)
{
    g_bright = b.toInt();
    return g_bright;
}

void setup()
{
    g_appId = APP_ID;
    
    pinMode(GREEN_BTN, INPUT);
    pinMode(YELLOW_BTN, INPUT);
    wing.begin();
    wing.show();
    g_timeSyncDone = false;
    g_displayState = BLANK;
    g_bright = BRIGHTNESS;
    g_buttonCount = 0;
    g_debounce = 0;
    Time.zone(CST_OFFSET);
    currentTimeZone();

    client.connect(g_mqttName.c_str());

    Particle.variable("timezone", g_offset);
    Particle.variable("brightness", g_bright);
    Particle.variable("appid", g_appId);
    Particle.variable("connected", g_mqttConnected);
    Particle.variable("button", g_buttonCount);
    Particle.function("setBright", setBrightness);
    g_mqttConnected = client.isConnected();
}

void loop()
{
    while (!client.isConnected()) {
        Particle.process();
        g_mqttConnected = false;
        client.connect(g_mqttName.c_str());
    }
    g_mqttConnected = true;
    client.loop();

    if (digitalRead(GREEN_BTN) == 1) {
        if ((g_debounce + 200) <= millis()) {
            g_debounce = millis();
            client.publish("heartlights/norah/on", "on");
            g_buttonCount++;
        }
    } 
    if (digitalRead(YELLOW_BTN) == 1) {
        if ((g_debounce + 1000) <= millis()) {
            g_debounce = millis();
            client.publish("heartlights/maddie/on", "on");
            g_buttonCount++;
        }
    }
    switch (Time.hour()) {
        case 0:
        case 1:
        case 2:
            drawBlank();
            break;      // Don't set time if it's not at least 3 to make sure we get the TZ changes at 2
        case 3:
            if (!g_timeSyncDone) {
                Particle.syncTime();
                currentTimeZone();
                g_timeSyncDone = true;
            }
            drawBlank();
            break;
        case 4:
        case 5:
            g_timeSyncDone = false;
            drawCross();
            break;
        case 6:
            if (Time.weekday() > 1 && Time.weekday() < 7)
                drawRectangle();
            break;
        case 7:
            if (Time.weekday() == 1 || Time.weekday() == 7)
                drawRectangle();
            break;
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
            drawBlank();
            break;
        case 19:
            drawSquare();
            break;
        case 20:
        case 21:
        case 22:
        case 23:
            drawCross();
            break;
    }
}
