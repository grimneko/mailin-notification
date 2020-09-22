/*
 * mail-in notification via IFTTT
 * Part of the Digital Prototype Academy course "Electronics + App Development" 
 * Complete Project Details https://wikifactory.com/+dpa/project-iot
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "AnotherIFTTTWebhook.h"
#include "secrets.h"

 
void setup() {
  Serial.begin(115200);
  Serial.setTimeout(2000);

  Serial.println();
  Serial.print("Connecting to access point... ");
  WiFi.persistent(false);
  
  int i = 0;
  while ((i < 2) && (WiFi.status() != WL_CONNECTED)) { // loop until we reached 3 attempts or WiFi.status() indicate we are connected
    WiFi.begin(ssid, password);
    Serial.print("Attempt ");
    Serial.print(i);
    Serial.print(" to connect to wifi network with SSID: ");
    Serial.println(ssid);
    delay(10000); // wait 10 sec to allow the esp to connect sucessful
    i++; // increment our attempt counter
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(" Couldn't establish WiFi connection after 3 attempts");
    Serial.println("Going back to DeepSleep");
    ESP.deepSleep(0);
  }
  send_webhook(eventname,apikey,"","","");
  Serial.println(" Wifi connection established successful");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  Serial.println(" Disconnecting and going back to deep sleep");
  WiFi.disconnect();
  ESP.deepSleep(0);
}

void loop() {
}
