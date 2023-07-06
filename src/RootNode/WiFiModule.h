#ifndef WIFI_MODULE
#define WIFI_MODULE

#ifndef ESP8266WiFi_LIB
#define ESP8266WiFi_LIB
#include <ESP8266WiFi.h>
#endif

class WiFiManager {

  String wifiSSID;
  String password;

  int maxRetries;

public:

  WiFiManager(String _wifiSSID, String _password, int _maxRetries = 20) {
    this->wifiSSID = _wifiSSID;
    this->password = _password;
    this->maxRetries = _maxRetries;
  }

  void connect(bool withOutputLogs = true) {
    if (withOutputLogs) {
      Serial.print("Connecting to ");
      Serial.print(this->wifiSSID);
    }

    WiFi.begin(this->wifiSSID, this->password);

    int retries(0);

    while (WiFi.status() != WL_CONNECTED && retries < this->maxRetries) {
      if (withOutputLogs) { Serial.print("."); }

      retries++;

      delay(1000);
    }

    if (withOutputLogs) {
      if (this->isConnected()) {
        Serial.print("\nSuccessfully connected to ");
        Serial.println(this->wifiSSID);
        Serial.print("IP Address: ");
        Serial.println(this->getlocalIP());
      }
    } else {
      Serial.println("Error connect");
    }
  }

  static bool isConnected() {
    return WiFi.status() == WL_CONNECTED;
  }

  IPAddress getlocalIP() {
    return WiFi.localIP();
  }
};


namespace WIFI_AP_NAMESPACE {

IPAddress local_IP(192, 168, 4, 22);
IPAddress gateway(192, 168, 4, 9);
IPAddress subnet(255, 255, 255, 0);

String wifiSSID = "ESP_" + WiFi.macAddress().substring(9);
String password = "01234567";

int max_connections = 8;
int channel = 1;
bool hidden = false;

int start(bool withOutputLogs = true) {

  int res = 0;

  // -- softAPConfig --

  if (withOutputLogs) { Serial.print("WiFi.softAPConfig -> "); }

  bool retValue = false;
  retValue = WiFi.softAPConfig(local_IP, gateway, subnet);

  if (retValue) {
    if (withOutputLogs) {
      Serial.println("Ready");
    } else {
      if (withOutputLogs) {
        Serial.println("Failed!");
      }
      res = -1;
    }
  }

  // -- softAP --

  if (withOutputLogs) { Serial.print("WiFi.softAP -> "); }

  retValue = false;
  retValue = WiFi.softAP(wifiSSID, password, 1, false, 8);

  if (retValue) {
    if (withOutputLogs) {
      Serial.println("Ready");
    } else {
      if (withOutputLogs) {
        Serial.println("Failed!");
      }
      res = -2;
    }
  }

  return res;
}
}

#endif