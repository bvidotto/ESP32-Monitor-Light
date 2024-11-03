// Import required libraries
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"

// Replace with your network credentials
const char* ssid = "";
const char* password = "";

// Pin definitions
const int warmLedPin = 2;
const int coldLedPin = 3;

// Start-up Settings
float colorLevel = 25.0;
float brightnessLevel = 50.0;
boolean toggleState = true;

// Web variable names
const char* PARAM_INPUT_COLOR = "color";
const char* PARAM_INPUT_BRIGHTNESS = "brightness";
const char* PARAM_INPUT_VALUE = "value";

// used in loop to smooth out transitions
int currentWarm = 255;
int currentCold = 255;
const int animationLength = 500;
const int delayIncrementations = 5;
int nextChangeMillis = millis();
int nextCold = 255;
int nextWarm = 255;
int coldIncrement = 0;
int warmIncrement= 0;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// To initialize values in loaded HTML
String processor(const String& var){
  Serial.println(var);
  if (var == "INIT_BRIGHTNESS_VALUE"){
  Serial.println(String(brightnessLevel));
      return String(brightnessLevel);
  }
  if (var == "INIT_COLOR_VALUE"){
  Serial.println(String(colorLevel));
      return String(colorLevel);
  }
  if (var == "TOGGLE_VALUE"){
  Serial.println(String(toggleState));
      return String(toggleState);
  }
  return String();
}

void setChangeValues(int warm, int cold){
  if (warm != currentWarm){
    warmIncrement = warm > currentWarm ? 1 : -1;
  }
  if (cold != currentCold){
    coldIncrement = cold > currentCold ? 1 : -1;
  }

  nextWarm = warm;
  nextCold = cold;
  nextChangeMillis = millis() + delayIncrementations;
}

void computeNextValues(float brightness, float color){
  brightness = max(float(0), min(float(100), brightness));
  color = max(float(0), min(float(100), color));
  toggleState = brightness > float(0) ? true : false;

  float maxBrightness = 255 * brightness/100;
  int warm = round(255 - maxBrightness * color/100);
  int cold = round(255 - maxBrightness * (1-color/100));

  brightnessLevel = brightness;
  colorLevel = color;

  setChangeValues(warm, cold);
}

void defineApiEndpoints(){
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", String(), false, processor);
  });
  
  // For style.css and script.js
  server.serveStatic("/", LittleFS, "/");

  server.on("/blink", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if (toggleState){
      analogWrite(coldLedPin, 255);
      analogWrite(warmLedPin, 255);
      delay(250);
      analogWrite(coldLedPin, currentCold);
      analogWrite(warmLedPin, currentWarm);
    }
    else{
      analogWrite(coldLedPin, 128);
      analogWrite(warmLedPin, 128);
      delay(250);
      analogWrite(coldLedPin, 255);
      analogWrite(warmLedPin, 255);
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/brightnessAdd", HTTP_PUT, [] (AsyncWebServerRequest *request) {
    if (request->hasParam(PARAM_INPUT_VALUE)) {
      float inputBrightness = brightnessLevel + (request->getParam(PARAM_INPUT_VALUE)->value()).toFloat();
      computeNextValues(inputBrightness, colorLevel);
    }
    request->send(200, "text/plain", String(brightnessLevel));
  });

  server.on("/colorAdd", HTTP_PUT, [] (AsyncWebServerRequest *request) {
    if (request->hasParam(PARAM_INPUT_VALUE)) {
      float inputColor = colorLevel + (request->getParam(PARAM_INPUT_VALUE)->value()).toFloat();
      computeNextValues(brightnessLevel, inputColor);
    }
    request->send(200, "text/plain", String(colorLevel));
  });

  server.on("/getSettings", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String results_json = "{ \"toggleState\": " + String(toggleState) + 
                          ", " + "\"colorLevel\": " + String(colorLevel) +
                          ", " + "\"brightnessLevel\": " + String(brightnessLevel) + " }";

    request->send(200, "application/json", results_json);
  });

  // Uses the max values for the warm and cold LEDs
  server.on("/max", HTTP_PUT, [] (AsyncWebServerRequest *request) {
    setChangeValues(0, 0);
    request->send(200, "text/plain", "OK");
  });

  server.on("/setSettings", HTTP_PUT, [] (AsyncWebServerRequest *request) {
    if (request->hasParam(PARAM_INPUT_COLOR) && request->hasParam(PARAM_INPUT_BRIGHTNESS)) {
      float inputColor = (request->getParam(PARAM_INPUT_COLOR)->value()).toFloat() ;
      float inputBrightness = (request->getParam(PARAM_INPUT_BRIGHTNESS)->value()).toFloat();
      computeNextValues(inputBrightness, inputColor);
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/toggle", HTTP_PUT, [] (AsyncWebServerRequest *request) {
    toggleState = !toggleState;
    if (toggleState){
      computeNextValues(brightnessLevel, colorLevel);
    }
    else{
      setChangeValues(255, 255);
    }
    request->send(200, "text/plain", String(toggleState));
  });
}

void setup(){
  analogWrite(coldLedPin, 255);
  analogWrite(warmLedPin, 255);
  computeNextValues(brightnessLevel, colorLevel);
  // Serial port for debugging purposes
  Serial.begin(115200);

  // Initialize LittleFS
  if(!LittleFS.begin()){
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  defineApiEndpoints();
  
  // Start server
  server.begin();
}

void loop() {
  // Control light using Serial
  if (Serial.available()){
    String cold = Serial.readStringUntil('|');
    String warm = Serial.readString();
    analogWrite(coldLedPin, cold.toInt());
    analogWrite(warmLedPin, warm.toInt());
  }

  // To smooth out the transition between two settings
  if (millis() >= nextChangeMillis){
    if (currentCold != nextCold && 0 <= currentCold && currentCold <= 255){
      currentCold += coldIncrement;
      analogWrite(coldLedPin, currentCold);
      nextChangeMillis += delayIncrementations;
    }
    if (currentWarm != nextWarm && 0 <= currentWarm && currentWarm <= 255){
      currentWarm += warmIncrement;
      analogWrite(warmLedPin, currentWarm);
      nextChangeMillis += delayIncrementations;
    }
  }
}