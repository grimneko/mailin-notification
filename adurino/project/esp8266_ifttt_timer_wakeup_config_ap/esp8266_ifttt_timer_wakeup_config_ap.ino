/*
 * mail-in notification via IFTTT
 * Part of the Digital Prototype Academy course "Electronics + App Development" 
 * Complete Project Details https://wikifactory.com/+dpa/project-iot
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "AnotherIFTTTWebhook.h"
#include "secrets.h"
#include "RTCVars.h"
#include <Ticker.h>
#include <LittleFS.h>

/*
 * -------------- Variables, objects and constant section --------------  
 *
 * Here we define all our data objects of any kind to be used globally
 * 
 */

// Define a Ticker object to enable the blinking led during setup
Ticker ledBlink;

// Define state object that allows us to interface with the ESP8266's RTC ram to store objects between the sleep(and reset cycles)
RTCVars state;

// Define global rtc saved mailmarker variable which saves the state of the mail sensor between wakeups
int mailmarker;

// set constant for GPIO 14 (D 5) where we look for our mail trigger
const short int pinD5 = 14;

// set constant for GPIO 12 (D6) where our setup switch is connected
const short int pinD6 = 12;

// set constant for the board led of the ESP8266 we let blink during setup mode
const short int boardLED = 2;

// define contstant in μs for sleep duration of the ESP between the cycles
const unsigned int espsleeplength = 10e6;

// define name/pass for our setup ap
const char *setup_ssid = "Mailin_AP";
const char *setup_password = "mailiswhatwedo"; // leave blank if you want it open for all

// our webserver for setup
ESP8266WebServer setupweb(80);

/* 
 *  -------------- functions section -------------- 
 *  
 *  Here we have all our functions we need in this sketch
 *  
 */

void blinkLedSetup()
{
  digitalWrite(boardLED, !(digitalRead(boardLED))); // invert the LED to switch it over the opposite state and make it blink
}

void handleRoot() // handle the webrequests
{
  // setupweb.send(200, "text/html", "<form action=\"/ENDSETUP\" method=\"POST\"><input type=\"submit\" value=\"End setup mode\"></form>");

}

void endSetup() // end setup mode and restart the esp
{
  Serial.println("Ending Setup mode and restarting");
  ESP.restart();
}

 /*
  * -------------- adurino procedure area -------------- 
  * 
  * Here we got the default functions for the adurino programs
  * 
  */
void setup() {
  Serial.begin(115200); // establish serial connection for debugging
  Serial.setTimeout(2000);
  Serial.println();

  pinMode(pinD6, INPUT_PULLUP); // configure D6 to be an input with the internal pullup enabled

  if (digitalRead(pinD6) == LOW) { // check if the user is pushing the setup button 
    // we enter setup mode
    if (!LittleFS.begin()){ // check if we can load our flash fs properly
      Serial.println("Error initializing flash fs, going back to sleep");
      ESP.deepSleep(espsleeplength);
      delay(100);
    } else {
      Serial.println("Flash FS successful mounted");
    }
    Serial.println("Entering Setup mode...");
    pinMode(boardLED,OUTPUT); // setup the on-board LED for blinking
    ledBlink.attach(0.5, blinkLedSetup); // initalize ticker to call our blink routine every 0.5s
    WiFi.persistent(false);
    WiFi.setOutputPower(1); // lower the TX rf power to 5 dbm to lower the range of the setup wifi
    WiFi.softAP(setup_ssid, setup_password); // start our setup ap
    Serial.print("Connect to ");
    Serial.println(WiFi.softAPIP());
    setupweb.serveStatic("/", LittleFS,"/index.html");
    //setupweb.on("/", handleRoot);
    //setupweb.on("/ENDSETUP", endSetup);
    setupweb.begin();
    Serial.println("Setup webpage started");
    return; // end the setup() routine early and hop over to loop() where we deal with the setup 
  }
  
  // Register our marker variable with RTCVars
  state.registerVar(&mailmarker);

  // Load mailmarker from RTC if there is a valid one, or set it to false to start anew after a full reboot (poweroutage)
  if (!state.loadFromRTC()) {
    // since mailmarker is not around, we did a coldboot and thus set it to 1 (false)
    mailmarker = 1;
    state.saveToRTC(); // store the new mailmarker value in the rtc
    Serial.println("We come from a coldboot/poweroutage, so we set mailmarker to 1 (false)");
  }

  pinMode(pinD5, INPUT_PULLUP); // configure D5 to be our input and enable the internal pullup resistor

  if (digitalRead(pinD5) == LOW) { // lets see if our sensor says we got mail
    if (mailmarker == 0) { // so the sensor is triggered but marker says we did send a notification already
      Serial.println("Sensor is triggered, but mailmarker says we send already a note, sleep time again for " + String(espsleeplength) + " μs (microseconds)");
      ESP.deepSleep(espsleeplength); // sleep again for duration definied in espsleeplength and look again
      delay(100);
    } else { 
      mailmarker = 0; // we got mail but no notification yet ? cool, lets go by setting our marker
      state.saveToRTC(); // and save the new state so it survive the deepSleep()
      Serial.println("Sensor is triggered, the marker says we didnt notified already, set the marker, save it and fire a notification");
    }
  } else {
    if (mailmarker == 0) { // check if our trigger is still set
      mailmarker = 1; // sensor is not triggered anymore ? reset the marker, ...
      state.saveToRTC(); // save it to the RTC, ...
      Serial.println("Sensor is not triggered anymore, reset the marker, save the marker and go to deepsleep for " + String(espsleeplength) + " μs (microseconds) again");
      ESP.deepSleep(espsleeplength); // and go back to sleep for the duration definied in espsleeplength until we give it another look
      delay(100);
    } else { // otherwise just sleep again
      Serial.println("Sensor is not triggered, marker is not set, wait " + String(espsleeplength) +" μs (microseconds) if it gets triggered again");
      ESP.deepSleep(espsleeplength); // since everything is in waiting position, lets go to sleep for another cycle
      delay(100);
    }
  }

  // up until now we should have figure out if we want to send a notification or not, so no more checks and just go for the notification
    
  Serial.println();
  Serial.print("Connecting to access point... ");
  WiFi.persistent(false); // for now dont save wifi data into flash to minimize flash memory overwrites
  
  int i = 0; // set connections attempt counter to 0
  WiFi.begin(ssid, password); // start the connection attempt to user provided wireless lan
  while ((i < 10) && (WiFi.status() != WL_CONNECTED)) { // loop until we reached 3 attempts or WiFi.status() indicate we are connected 
    Serial.print("Attempt ");
    Serial.print(i);
    Serial.print(" to connect to wifi network with SSID: ");
    Serial.println(ssid);
    delay(500); // wait 10 sec to allow the esp to connect sucessful
    i++; // increment our attempt counter
  }
  if (WiFi.status() != WL_CONNECTED) { // check if wifi is connected and react
    Serial.println(" Couldn't establish WiFi connection after 5 sec"); // we couldn't connect, thus just go back to sleep (to wake up again we reset anyway, ram gets cleaned)
    Serial.println("Going back to DeepSleep");
    ESP.deepSleep(espsleeplength);
    delay(100);
  }
  
  Serial.println(" Wifi connection established successful"); // we are connected (and probably in the internet)
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); 
  Serial.println("Trying to send the notification");
  send_webhook(eventname,apikey,"","",""); // here is the magic happening, we notify ifttt we got an event (aka the mailbox got probably filled with something)
  Serial.println(" Disconnecting and going back to deep sleep");
  WiFi.disconnect(); // we did send the event, now its time to sleep deep again thus disconnect ...
  ESP.deepSleep(espsleeplength); // and go back to sleep
  delay(100);
}

void loop() {
  setupweb.handleClient();
  yield();
}
