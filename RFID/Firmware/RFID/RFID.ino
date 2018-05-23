#include <FS.h>
#include <ESP.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
// wifi manager
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
// rfid reader
#include "RDM6300.h"
// lcd display
#include <LiquidCrystal_I2C.h>
// json
#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>

#define SSID "RFID"
#define PASS "12345678"
#define HOST "192.168.2.4"
#define PORT 82

#define STATE_PIN D6
#define LCD_I2C 0x27
#define COLS 16
#define ROWS 2
#define SDA D2
#define SCL D1
#define DELAY 1000

RDM6300<HardwareSerial> *rfid;
LiquidCrystal_I2C *lcd;

void setup() {
  WiFiManager wifiManager;
  wifiManager.setDebugOutput(false);
  wifiManager.autoConnect(SSID, PASS);

  pinMode(STATE_PIN, INPUT);
  rfid = new RDM6300<HardwareSerial>(&Serial);
  lcd = new LiquidCrystal_I2C(LCD_I2C, COLS, ROWS);
  lcd->begin(SDA, SCL);
  lcd->backlight();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  lcd->clear();
  unsigned long long id = rfid->read();
  int state = digitalRead(STATE_PIN);
  rfid->print_int64(id, (Stream*)lcd);
  sendRequest("/rfid", payload(id, state));

  delay(DELAY);
}

String payload(unsigned long long id, int state) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["rfid"] = string_int64(id);
  json["state"] = state;
  String result;
  json.printTo(result);
  return result;
}

void sendRequest(String url, String payload) {
  HTTPClient http;
  http.begin(HOST, PORT, url);
  http.addHeader("Content-Type", "application/json");
  http.POST(payload);
  http.end();
}

String string_int64(unsigned long long data) {
    union {
      unsigned long long ull;
      unsigned long ul[2];
    } tmp;
    String output;
    tmp.ull = data;
    output += String(tmp.ul[1], HEX);
    unsigned long beacon = 0x10000000;
    while(beacon > 0) {
      if(tmp.ul[0] < beacon)
        output += "0";
      else
        break;
      beacon >>= 4;
    }
    output += String(tmp.ul[0], HEX);
    return output;
}
