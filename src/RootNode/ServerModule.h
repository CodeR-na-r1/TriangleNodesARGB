#include <cstddef>
#ifndef SERVER_MODULE
#define SERVER_MODULE


#ifndef WIFI_MODULE
#include "esp_wifi_module.hpp"
#endif

#ifndef Arduino_LIB
#define Arduino_LIB
#include <Arduino.h>
#endif

#ifndef ESP8266WiFi_LIB
#define ESP8266WiFi_LIB
#include <ESP8266WiFi.h>
#endif

#ifndef AESPAsyncTCP_LIB
#define ESPAsyncTCP_LIB
#include <ESPAsyncTCP.h>
#endif

#ifndef ESPAsyncWebSrv_LIB
#define ESPAsyncWebSrv_LIB
#include <ESPAsyncWebSrv.h>
#endif

namespace SERVER_NAMESPACE {

AsyncWebServer server(80);

void (*userCahngeColorCallback)(String, String, String) = nullptr;
void (*userCahngeBrightnessCallback)(String) = nullptr;
void (*userCahngeModeCallback)(String) = nullptr;

// uint8_t* mode = nullptr;
// uint8_t* brightness = nullptr;
// uint8_t* rColor = nullptr;
// uint8_t* gColor = nullptr;
// uint8_t* bColor = nullptr;

String html = "";

int max_attempts = 20;

void setuserCahngeColorCallback(void (*_userCallback)(String, String, String)) {
  userCahngeColorCallback = _userCallback;

  return;
}

void setuserCahngeBrightnessCallback(void (*_userCallback)(String)) {
  userCahngeBrightnessCallback = _userCallback;

  return;
}

void setuserCahngeModeCallback(void (*_userCallback)(String)) {
  userCahngeModeCallback = _userCallback;

  return;
}

// void setData(uint8_t& mode_, uint8_t& brightness_, uint8_t& rColor_, uint8_t& gColor_, uint8_t& bColor_) {
//   mode = mode_;
//   brightness = brightness_;
//   rColor = rColor_;
//   gColor = gColor_;
//   bColor = bColor_;
// }

void initHTML() {
  html = "<!DOCTYPE html> <html>\n";

  html += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  html += "<title>TreeNode ARGB</title>\n";
  html += "</head>\n";

  html += "<body>\n";

  html += "<h1>Customize your light</h1>\n";

  html += "<form method=\"POST\" action=\"commit\">\n";

  html += "<p>\n";
  html += "<label for=\"colorChoise\">colorChoise:</label>\n";
  html += "<input type=\"color\" id=\"color\" name=\"colorChoise\" required>\n";
  html += "</p>\n";

  html += "<p>\n";
  html += "<label for=\"brightnessChoise\">brightnessChoise:</label>\n";
  html += "<input type=\"range\" id=\"brightness\" name=\"brightnessChoise\" min=\"0\" max=\"255\" required>\n";
  html += "</p>\n";

  html += "<p>\n";
  html += "<label for=\"modeChoise\">modeChoise:</label>\n";
  html += "<select id=\"mode\" name=\"modeChoise\">\n";
  html += "<option value=\"Static\">Static color</option>";
  html += "<option value=\"StaticAnim\">Static color animation</option>";
  html += "<option value=\"Rainbow\">Rainbow animation</option>";
  html += "</select>\n";
  html += "</p>\n";
  // html += "<button type=\"submit\">Change color</button>";
  html += "</form>\n";

  html += "</body>\n";

  // Внимание, опасная зона, ниже может присутствовать код на JAVASCRIPT!!!!!!!!!!!!

  // Для обработки цвета

  html += "<script>";

  html += "let elem = document.getElementById(\"color\");";
  html += "elem.addEventListener(\"change\", updateColor);";
  html += "console.log(\"script work!\");";

  html += "function updateColor(event) {";
  html += "let value = event.target.value;";
  html += "console.log(value);";

  html += "let r = (parseInt(value[1] + value[2], 16)).toString();";
  html += "console.log(r);";
  html += "let g = parseInt(value[3] + value[4], 16);";
  html += "console.log(g);";
  html += "let b = parseInt(value[5] + value[6], 16);";
  html += "console.log(b);";
  // html += "let req = new XMLHttpRequest(); req.open( \"GET\", window.location.origin + \"/commitData?\" + r + \"&\" + g + \"&\" + b, false); req.send( null );";
  html += "fetch(window.location.origin + \"/commitColor?r=\" + r + \"&g=\" + g + \"&b=\" + b);";
  html += "console.log(window.location.origin + \"/commitColor?r=\" + r + \"&g=\" + g + \"&b=\" + b);";
  html += "}";

  html += "</script>";

  // Для обработки яркости

  html += "<script>";

  html += "let elemBrightness = document.getElementById(\"brightness\");";
  html += "elemBrightness.addEventListener(\"change\", updateBrightness);";
  html += "console.log(\"script2 work!\");";

  html += "function updateBrightness(event) {";
  html += "let valueBrightness = event.target.value;";
  html += "console.log(valueBrightness);";

  html += "fetch(window.location.origin + \"/commitBrightness?brightness=\" + valueBrightness);";
  html += "console.log(window.location.origin + \"/commitBrightness?brightness=\" + valueBrightness);";
  html += "}";

  html += "</script>";

  // Для обработки режима

  html += "<script>";

  html += "let elemMode = document.getElementById(\"mode\");";
  html += "elemMode.addEventListener(\"change\", updateMode);";
  html += "console.log(\"script3 work!\");";

  html += "function updateMode(event) {";
  html += "let valueMode = event.target.value;";
  html += "console.log(valueMode);";

  html += "fetch(window.location.origin + \"/commitMode?mode=\" + valueMode);";
  html += "console.log(window.location.origin + \"/commitMode?mode=\" + valueMode);";
  html += "}";

  html += "</script>";
}

void notFound(AsyncWebServerRequest* request) {
  request->send(404, "text/plain", "Not found");
}

void mainPage(AsyncWebServerRequest* request) {
  request->send(200, "text/html", html);
}

void processsingColorGetReq(AsyncWebServerRequest* request) {

  Serial.println("get color req!");
  String r, g, b;

  if (request->hasParam("r")) {
    r = request->getParam("r")->value();
    Serial.println("red color get!");
  } else {
    Serial.println("red color FAIL!");
    request->send(200, "text/plain", "Error r field!");
    return;
  }

  if (request->hasParam("g")) {
    g = request->getParam("g")->value();
    Serial.println("green color get!");
  } else {
    Serial.println("green color FAIL!");
    request->send(200, "text/plain", "Error g field!");
    return;
  }

  if (request->hasParam("b")) {
    b = request->getParam("b")->value();
    Serial.println("blue color get!");
  } else {
    Serial.println("blue color FAIL!");
    request->send(200, "text/plain", "Error b field!");
    return;
  }

  if (userCahngeColorCallback != nullptr) {
    userCahngeColorCallback(r, g, b);
    Serial.println("_userCallback called");
  } else {
    Serial.println("_userCallback is nullpointer");
  }

  request->send(200, "text/html", html);
}

void processsingBrightnessGetReq(AsyncWebServerRequest* request) {

  Serial.println("get Brightness req!");
  String brightness;

  if (request->hasParam("brightness")) {
    brightness = request->getParam("brightness")->value();
    Serial.println("brightness get!");
  } else {
    Serial.println("brightness FAIL!");
    request->send(200, "text/plain", "Error brightness field!");
    return;
  }

  if (userCahngeBrightnessCallback != nullptr) {
    userCahngeBrightnessCallback(brightness);
    Serial.println("_userCallback called");
  } else {
    Serial.println("_userCallback is nullpointer");
  }

  request->send(200, "text/html", html);
}

void processsingModeGetReq(AsyncWebServerRequest* request) {

  Serial.println("get mode req!");
  String mode;

  if (request->hasParam("mode")) {
    mode = request->getParam("mode")->value();
    Serial.println("mode get!");
  } else {
    Serial.println("mode FAIL!");
    request->send(200, "text/plain", "Error mode field!");
    return;
  }

  if (userCahngeModeCallback != nullptr) {
    userCahngeModeCallback(mode);
    Serial.println("_userCallback called");
  } else {
    Serial.println("_userCallback is nullpointer");
  }

  request->send(200, "text/html", html);
}

int start() {
  Serial.println("Starting server...");

  int retValue = WIFI_AP_NAMESPACE::start();

  initHTML();
  server.onNotFound(notFound);
  server.on("/", HTTP_GET, mainPage);
  server.on("/commitColor", HTTP_GET, processsingColorGetReq);
  server.on("/commitBrightness", HTTP_GET, processsingBrightnessGetReq);
  server.on("/commitMode", HTTP_GET, processsingModeGetReq);

  server.begin();

  Serial.println("Server started!");

  return retValue;
}

}

#endif