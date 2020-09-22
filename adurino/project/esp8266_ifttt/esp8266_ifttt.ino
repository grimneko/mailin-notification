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
  Serial.begin(115200); // establish serial connection for debugging
  Serial.setTimeout(2000);

  Serial.println();
  Serial.print("Connecting to access point... ");
  WiFi.persistent(false); // for now dont save wifi data into flash to minimize flash memory overwrites
  
  int i = 0; // set connections attempt counter to 0
  while ((i < 2) && (WiFi.status() != WL_CONNECTED)) { // loop until we reached 3 attempts or WiFi.status() indicate we are connected
    WiFi.begin(ssid, password); // start the connection attempt to user provided wireless lan
    Serial.print("Attempt ");
    Serial.print(i);
    Serial.print(" to connect to wifi network with SSID: ");
    Serial.println(ssid);
    delay(10000); // wait 10 sec to allow the esp to connect sucessful
    i++; // increment our attempt counter
  }
  if (WiFi.status() != WL_CONNECTED) { // check if wifi is connected and react
    Serial.println(" Couldn't establish WiFi connection after 3 attempts"); // we couldn't connect, thus just go back to sleep (to wake up again we reset anyway, ram gets cleaned)
    Serial.println("Going back to DeepSleep");
    ESP.deepSleep(0);
  }
  
  Serial.println(" Wifi connection established successful"); // we are connected (and probably in the internet)
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); 
  send_webhook(eventname,apikey,"","",""); // here is the magic happening, we notify ifttt we got an event (aka the mailbox got probably filled with something)
  Serial.println(" Disconnecting and going back to deep sleep");
  WiFi.disconnect(); // we did send the event, now its time to sleep deep again thus disconnect ...
  ESP.deepSleep(0); // and go back to sleep
}

void loop() {
}
