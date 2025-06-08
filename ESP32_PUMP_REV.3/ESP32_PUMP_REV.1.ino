#include <RTClib.h>
#include <WiFi.h>
#include <Wire.h>
#include <TimeLib.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <NetworkClient.h>
#include <Arduino.h>

const byte DNS_PORT = 53;
DNSServer dnsServer;
RTC_DS1307 rtc;
WebServer server(80);
int scheduledOnHour = -1, scheduledOnMinute = -1;
int scheduledOffHour = -1, scheduledOffMinute = -1;
unsigned long pumpStartTime = 0;
int autoOffMinutes = -1;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;
#ifndef APPSSID
#define APSSID "Pump"
#define APPSK "123456789"
#endif

#define RELAY_PIN 32
#define BUTTON_PIN 33
const char *softAP_ssid = APSSID;
const char *softAP_password = APPSK;
const char *myHostname = "Pump";

const char webpage[] PROGMEM = R"=====( 
<!DOCTYPE html>
<html>
<head>
<title>Pump Control</title>
<style>
  body { font-family: sans-serif; margin: 20px; background-color: #f4f4f4; }
  h1, h2 { color: #333; text-align: center; }
  p { text-align: center; margin-bottom: 20px; }
  form { background-color: #fff; padding: 20px; margin: 10px auto; width: 300px;
         border-radius: 8px; box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1); }
  label { display: block; margin-bottom: 5px; color: #555; }
  input, button { width: 100%; padding: 8px; margin-bottom: 10px;
                  border: 1px solid #ddd; border-radius: 4px; box-sizing: border-box; }
  button { background-color: #007bff; color: white; cursor: pointer;
           transition: background-color 0.3s ease; }
  button:hover { background-color: #0056b3; }
  #pumpStatus { font-weight: bold; color: #007bff; }
</style>
</head>
<body>
  <h1>Pump Control</h1>
  <p>Pump Status: <span id="pumpStatus">%PUMP_STATUS%</span></p>

  <form action="/pumpOn" method="post">
    <button type="submit">Turn On</button>
  </form>

  <form action="/pumpOff" method="post">
    <button type="submit">Turn Off</button>
  </form>

  <h2>Schedule</h2>
  <form action="/schedule" method="post">
    <label for="onTime">On Time:</label>
    <input type="time" id="onTime" name="onTime"><br><br>
    <label for="offTime">Off Time:</label>
    <input type="time" id="offTime" name="offTime"><br><br>
    <button type="submit">Schedule</button>
  </form>

  <h2>Auto Off</h2>
  <form action="/autoOff" method="post">
    <label for="autoOffTime">Auto Off Time (minutes):</label>
    <input type="number" id="autoOffTime" name="autoOffTime" min="1"><br><br>
    <button type="submit">Auto Off</button>
  </form>

</body>
</html>
)=====";

String pumpStatus = "OFF";  // Default status
IPAddress apIP(192,168,1,1);
IPAddress netMsk(255, 255, 255, 0);

void handleRoot() {
  String page = String(webpage);
  page.replace("%PUMP_STATUS%", pumpStatus); 
  server.send(200, "text/html", page); 
}

void handlePumpOn() {
  turnOnPump();
  pumpStartTime = millis();
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}

void handlePumpOff() {
  turnOffPump();
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}

void handleSchedule() {
    if (server.hasArg("onTime")) {
        String onTime = server.arg("onTime");  // Get time as string "hh:mm"
        scheduledOnHour = onTime.substring(0, 2).toInt();   // Extract hours
        scheduledOnMinute = onTime.substring(3, 5).toInt(); // Extract minutes
    }

    if (server.hasArg("offTime")) {
        String offTime = server.arg("offTime"); 
        scheduledOffHour = offTime.substring(0, 2).toInt();
        scheduledOffMinute = offTime.substring(3, 5).toInt();
    }

    // Redirect back to the main page
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
}


void handleAutoOff() {
  String autoOffTime = server.arg("autoOffTime");
  autoOffMinutes = server.arg("autoOffTime").toInt();
  Serial.print("Auto Off Time (minutes): ");
  Serial.println(autoOffTime);

  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}

void turnOnPump() {
  digitalWrite(RELAY_PIN, LOW);
  pumpStatus = "ON";
  Serial.println("Pump turned ON");
}

void turnOffPump() {
  digitalWrite(RELAY_PIN, HIGH);
  pumpStatus = "OFF";
  Serial.println("Pump turned OFF");
}

void checkAutoOff() {
    if (autoOffMinutes > 0 && (millis() - pumpStartTime) >= autoOffMinutes * 60000) {
        turnOffPump();
        Serial.println("Pump OFF (Auto-Off Timer)");
        autoOffMinutes = -1;  // Reset auto-off setting
    }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  rtc.begin();
  Serial.println("\nConfiguring access point...");
  
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
  pinMode(BUTTON_PIN,INPUT_PULLUP);

  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(softAP_ssid, softAP_password);
  delay(500);

  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);
  server.onNotFound([]() {
    server.sendHeader("Location","http://192.168.1.1/",true);
    server.send(302,"text/plain","");
  });
  server.on("/", handleRoot);
  server.on("/pumpOn", HTTP_POST, handlePumpOn);
  server.on("/pumpOff", HTTP_POST, handlePumpOff);
  server.on("/schedule", HTTP_POST, handleSchedule);
  server.on("/autoOff", HTTP_POST, handleAutoOff);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  DateTime now = rtc.now();
  int currentHour = now.hour();
  int currentMin = now.minute();
  
    if (currentHour == scheduledOnHour && currentMin == scheduledOnMinute) {
        turnOnPump();
        
    }

    if (currentHour == scheduledOffHour && currentMin == scheduledOffMinute) {
        turnOffPump();
        
    }
  checkAutoOff();
  int buttonState = digitalRead(BUTTON_PIN);
  if(buttonState!=lastButtonState){
    lastDebounceTime= millis();
  }
  if ((millis()-lastDebounceTime)> debounceDelay){
    if(buttonState== LOW && lastButtonState == HIGH){
      if(pumpStatus == "OFF"){
        turnOnPump();
      }
      else
      {
        turnOffPump();
      }
    }
  }
  lastButtonState=buttonState;
}