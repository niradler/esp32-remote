//set ssid and password, the pins is for m5stack atom lite and the ir code is for lg tv. (turn on and off.)

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include "M5Atom.h"

AsyncWebServer server(80);

const char* ssid = "***";
const char* password = "****";

const char* PARAM_MESSAGE = "message";

const uint16_t kIrLed = 12;  // ESP8266 GPIO pin to use. Recommended: 4 (D2).
uint16_t rawDataA[67] = {8936, 4442,  508, 540,  518, 532,  516, 1686,  488, 538,  516, 530,  522, 528,  518, 532,  458, 562,  516, 1688,  516, 1688,  520, 530,  490, 1692,  488, 1714,  484, 1718,  520, 1684,  520, 1684,  518, 530,  520, 506,  482, 564,  522, 1682,  522, 528,  522, 504,  506, 538,  520, 530,  518, 1686,  522, 1682,  490, 1692,  506, 540,  516, 1686,  520, 1684,  516, 1688,  520, 1684,  522};  // NEC 20DF10EF
uint16_t rawDataB[53] = {430, 1278,  232, 136,  364, 196,  178, 172,  512, 158,  256, 2168,  1438, 140,  146, 1448,  150, 896,  1572, 1416,  162, 448,  146, 1032,  180, 148,  454, 2700,  588, 1202,  134, 2168,  236, 1964,  206, 2710,  298, 136,  158, 242,  542, 136,  396, 142,  208, 918,  380, 200,  182, 138,  150, 1738,  384};  // UNKNOWN 131ED6AE
uint16_t rawDataC[39] = {350, 552,  416, 618,  468, 538,  268, 644,  106, 1150,  334, 136,  104, 1276,  454, 128,  970, 222,  254, 2186,  208, 750,  206, 132,  506, 132,  196, 2032,  624, 1120,  108, 444,  422, 398,  106, 864,  930, 210,  880};  // UNKNOWN 8C7D6A2B
uint16_t rawDataD[25] = {254, 140,  374, 1102,  288, 3496,  724, 630,  356, 224,  510, 232,  150, 4552,  498, 278,  1152, 840,  150, 152,  390, 710,  250, 510,  308};  // UNKNOWN BE903067

IRsend irsend(kIrLed);  // Set the GPIO to be used to sending the message.

uint8_t DisBuff[2 + 5 * 5 * 3];

void setBuff(uint8_t Rdata, uint8_t Gdata, uint8_t Bdata)
{
  DisBuff[0] = 0x05;
  DisBuff[1] = 0x05;
  for (int i = 0; i < 25; i++)
  {
    DisBuff[2 + i * 3 + 0] = Rdata;
    DisBuff[2 + i * 3 + 1] = Gdata;
    DisBuff[2 + i * 3 + 2] = Bdata;
  }
}

void toggle() {
  Serial.println("Power Toggle");
  // Power On

  irsend.sendRaw(rawDataA, 67, 38);  // Send a raw data capture at 38kHz.
  delay(1);
  irsend.sendRaw(rawDataB, 53, 38);  // Send a raw data capture at 38kHz.
  delay(1);
  irsend.sendRaw(rawDataC, 39, 38);  // Send a raw data capture at 38kHz.
  delay(1);
  irsend.sendRaw(rawDataD, 25, 38);  // Send a raw data capture at 38kHz.
  delay(1000);
  Serial.println("Toggle Complete!");
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void handleRoot(AsyncWebServerRequest *request) {
  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf(temp, 400,

           "<html>\
  <head>\
    <meta http-equiv='refresh' content='60'/>\
    <title>Tv Remote</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Tv Remote:</h1>\
    <a style=\"margin-left: 50%;font-size: 400%;\" href=\"/toggle\">Toggle</a>\
    <p>Uptime: %02d:%02d:%02d</p>\
  </body>\
</html>",

           hr, min % 60, sec % 60
          );
  request->send(200, "text/html", temp);
}

void setup() {
  M5.begin(true, false, true);
  delay(10);
  setBuff(0x00, 0x40, 0x00);
  M5.dis.displaybuff(DisBuff);
  irsend.begin();
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("WiFi Failed!\n");
    return;
  }

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, handleRoot);

  // Send a GET request to <IP>/get?message=<message>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String message;
    if (request->hasParam(PARAM_MESSAGE)) {
      message = request->getParam(PARAM_MESSAGE)->value();
    } else {
      message = "No message sent";
    }
    request->send(200, "text/plain", "Hello, GET: " + message);
  });


  server.on("/toggle", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("GET /toggle");
    toggle();
    Serial.println("Redirect /");
    request->redirect("/");
  });

  server.onNotFound(notFound);

  server.begin();
}

void loop() {
    if (M5.Btn.wasPressed())
  {
    toggle();

  }

  delay(50);
  M5.update();
}
