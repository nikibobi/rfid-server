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
typedef unsigned long long id_t;

#define SSID "RFID"
#define PASS "12345678"
#define HOST "192.168.2.4"
#define PORT 82

#define ACCESS_FILE "/cards.txt"
#define RELAY_PIN LED_BUILTIN
#define ADMIN_PIN D7
#define RELAY_DELAY 5000
#define ADDED_PREFIX "+"
#define MSG_OPEN "open"

#define ERROR_SPIFFS_FAILED "spiffs failed"
#define ERROR_FILE_OPEN "open failed"
#define ERROR_CACHE_FULL "cache full"
#define ERROR_ACCESS_DENIED "access denied"

#define STATE_PIN D6
// lcd display
#define LCD_I2C 0x27
#define COLS 16
#define ROWS 2
#define SDA D2
#define SCL D1
#define LOG_DELAY 500

#define DELAY 1000

RDM6300<HardwareSerial> *rfid;
LiquidCrystal_I2C *lcd;
DynamicJsonBuffer jsonBuffer;
JsonVariant json;

void setup() {
  lcd = new LiquidCrystal_I2C(LCD_I2C, COLS, ROWS);
  lcd->begin(SDA, SCL);
  lcd->backlight();
  stage();
  
  WiFiManager wifiManager;
  wifiManager.setDebugOutput(false);
  wifiManager.autoConnect(SSID, PASS);
  stage();
  
  pinMode(ADMIN_PIN, INPUT);
  pinMode(STATE_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
  stage();

  rfid = new RDM6300<HardwareSerial>(&Serial);
  stage();

  clear_cache();
  stage();

  if (SPIFFS.begin()) {
    stage();
  } else {
    log(ERROR_SPIFFS_FAILED);
  }
}

void loop() {
  lcd->clear();
  id_t id = rfid->read();

  if (digitalRead(ADMIN_PIN) && !has_access(id) && grant_access(id)) {
    lcd->print(ADDED_PREFIX);
  }
  
  rfid->print_int64(id, (Stream*)lcd);

  if (has_access(id)) {
    trigger_relay();
  } else {
    log(ERROR_ACCESS_DENIED);
  }

  int state = digitalRead(STATE_PIN);

  cache(id, state);

  if (WiFi.status() == WL_CONNECTED) {
    String payload;
    json.printTo(payload);
    sendRequest("/rfid", payload);
    clear_cache();
  }

  delay(DELAY);
}

void log(String message) {
  lcd->clear();
  lcd->print(message);
  delay(LOG_DELAY);
  lcd->clear();
}

void stage() {
  static int i = 0;
  log("stage " + String(++i));
}

bool grant_access(id_t id) {
  File file = SPIFFS.open(ACCESS_FILE, "a+");
  if (!file) {
    log(ERROR_FILE_OPEN);
    return false;
  }
  file.println(string_int64(id));
  file.close();
  return true;
}

bool has_access(id_t id) {
  File file = SPIFFS.open(ACCESS_FILE, "r");
  if (!file) {
    log(ERROR_FILE_OPEN);
    return false;
  }
  String asked_id = string_int64(id);
  while (file.available()) {
    String current_id = file.readStringUntil('\r');
    if (asked_id == current_id) {
      file.close();
      return true;
    }
  }
  file.close();
  return false;
}

void trigger_relay() {
  digitalWrite(RELAY_PIN, LOW);
  delay(RELAY_DELAY);
  digitalWrite(RELAY_PIN, HIGH);
}

void cache(id_t id, int state) {
  JsonObject& object = json.as<JsonArray>().createNestedObject();
  if (object.success()) {
    object["rfid"] = string_int64(id);
    object["state"] = state;
  } else {
    log(ERROR_CACHE_FULL);
    clear_cache();
  }
}

void clear_cache() {
  jsonBuffer.clear();
  json = jsonBuffer.createArray();
}

void sendRequest(String url, String payload) {
  HTTPClient http;
  http.begin(HOST, PORT, url);
  http.addHeader("Content-Type", "application/json");
  http.POST(payload);
  http.end();
}

String string_int64(id_t id) {
  union {
    unsigned long long ull;
    unsigned long ul[2];
  } tmp;
  String output;
  tmp.ull = id;
  output += String(tmp.ul[1], HEX);
  unsigned long beacon = 0x10000000;
  while (beacon > 0) {
    if (tmp.ul[0] < beacon)
      output += "0";
    else
      break;
    beacon >>= 4;
  }
  output += String(tmp.ul[0], HEX);
  return output;
}