/*
 * mail-in notification via IFTTT
 * Part of the Digital Prototype Academy course "Electronics + App Development" 
 * Complete Project Details https://wikifactory.com/+dpa/project-iot
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "AnotherIFTTTWebhook.h"
#include "secrets.h"
#include "RTCVars.h"

// CPP preprocessor define to enable serial debug messages, comment out to disable them when you have things working.
#define SERIAL_DEBUG

// Define state object that allows us to interface with the ESP8266's RTC ram to store objects between the sleep(and reset cycles)
RTCVars state;

// Define global rtc saved mailmarker variable which saves the state of the mail sensor between wakeups
int mailmarker;

// set constant for GPIO 14 (D 5) where we look for our mail trigger
const short int pinD5 = 14;

// define contstant in μs for sleep duration of the ESP between the cycles
const unsigned int espsleeplength = 10e6;
 
void setup() {
#ifdef SERIAL_DEBUG
  Serial.begin(115200); // establish serial connection for debugging
  Serial.setTimeout(2000);
  Serial.println();
#endif

  // Register our marker variable with RTCVars
  state.registerVar(&mailmarker);

  // Load mailmarker from RTC if there is a valid one, or set it to false to start anew after a full reboot (poweroutage)
  if (!state.loadFromRTC()) {
    // since mailmarker is not around, we did a coldboot and thus set it to 1 (false)
    mailmarker = 1;
    state.saveToRTC(); // store the new mailmarker value in the rtc

#ifdef SERIAL_DEBUG
    Serial.println("We come from a coldboot/poweroutage, so we set mailmarker to 1 (false)");
#endif

  }

  pinMode(pinD5, INPUT_PULLUP); // configure D5 to be our input and enable the internal pullup resistor

  if (digitalRead(pinD5) == LOW) { // lets see if our sensor says we got mail
    if (mailmarker == 0) { // so the sensor is triggered but marker says we did send a notification already

#ifdef SERIAL_DEBUG
      Serial.println("Sensor is triggered, but mailmarker says we send already a note, sleep time again for " + String(espsleeplength) + " μs (microseconds)");
#endif

      ESP.deepSleep(espsleeplength); // sleep again for duration definied in espsleeplength and look again
      delay(100);
    } else { 
      mailmarker = 0; // we got mail but no notification yet ? cool, lets go by setting our marker
      state.saveToRTC(); // and save the new state so it survive the deepSleep()

#ifdef SERIAL_DEBUG
      Serial.println("Sensor is triggered, the marker says we didnt notified already, set the marker, save it and fire a notification");
#endif

    }
  } else {
    if (mailmarker == 0) { // check if our trigger is still set
      mailmarker = 1; // sensor is not triggered anymore ? reset the marker, ...
      state.saveToRTC(); // save it to the RTC, ...

#ifdef SERIAL_DEBUG
      Serial.println("Sensor is not triggered anymore, reset the marker, save the marker and go to deepsleep for " + String(espsleeplength) + " μs (microseconds) again");
#endif

      ESP.deepSleep(espsleeplength); // and go back to sleep for the duration definied in espsleeplength until we give it another look
      delay(100);
    } else { // otherwise just sleep again

#ifdef SERIAL_DEBUG
      Serial.println("Sensor is not triggered, marker is not set, wait " + String(espsleeplength) +" μs (microseconds) if it gets triggered again");
#endif

      ESP.deepSleep(espsleeplength); // since everything is in waiting position, lets go to sleep for another cycle
      delay(100);
    }
  }

  // up until now we should have figure out if we want to send a notification or not, so no more checks and just go for the notification

#ifdef SERIAL_DEBUG
  Serial.println();
  Serial.print("Connecting to access point... ");
#endif

  WiFi.persistent(false); // for now dont save wifi data into flash to minimize flash memory overwrites
  
  int i = 0; // set connections attempt counter to 0
  WiFi.begin(ssid, password); // start the connection attempt to user provided wireless lan
  while ((i < 10) && (WiFi.status() != WL_CONNECTED)) { // loop until we reached 10 attempts (which equal 5.5 secs) or WiFi.status() indicate we are connected 

#ifdef SERIAL_DEBUG
    Serial.print("Attempt ");
    Serial.print(i);
    Serial.print(" to connect to wifi network with SSID: ");
    Serial.println(ssid);
#endif    

    delay(500); // wait 0.5 sec before checking again if we are connected to the wireless lan
    i++; // increment our attempt counter
  }
  if (WiFi.status() != WL_CONNECTED) { // check if wifi is connected and react

#ifdef SERIAL_DEBUG
    Serial.println(" Couldn't establish WiFi connection after 5 sec"); // we couldn't connect, thus just go back to sleep (to wake up again we reset anyway, ram gets cleaned)
    Serial.println("Going back to DeepSleep");
#endif
    
    ESP.deepSleep(espsleeplength);
    delay(100);
  }

#ifdef SERIAL_DEBUG
  Serial.println(" Wifi connection established successful"); // we are connected (and probably in the internet)
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); 
  Serial.println("Trying to send the notification");
#endif

  if (!send_webhook(eventname,apikey,"","","")) { // here is the magic happening, we notify ifttt we got an event (aka the mailbox got probably filled with something)
    mailmarker = 1; // send_webhook did gave back an error, so we reset the marker so next wake up we try to send another notification
    state.saveToRTC(); // save our marker in the RTC ram

#ifdef SERIAL_DEBUG
    Serial.println("Encountered error while sending notification to IFTTT, will try again when the next wake up happens and the trigger is still going!");
#endif

  }

#ifdef SERIAL_DEBUG
  Serial.println(" Disconnecting and going back to deep sleep");
#endif

  WiFi.disconnect(); // we did send the event, now its time to sleep deep again thus disconnect ...
  ESP.deepSleep(espsleeplength); // and go back to sleep
  delay(100);
}

void loop() {
}
