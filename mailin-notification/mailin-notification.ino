// define the pins to be used and give them a name

const int sensorPIN = 2 ; // here our sensor (reed contact, photo diode, etc) is connected
const int ledPIN = 13 ; // internal LED pin to signal the sensor picked up something

// variable to read the sensor state into
int sensorSTATE = 0;

void setup() {
  // initialize the internal LED as output pin
  pinMode(ledPIN, OUTPUT);
  // initalize the sensor pin as input
  pinMode(sensorPIN, INPUT);
  // initialise serial monitor for debugging with 9600 baud
  Serial.begin(9600);
  // send an initial display text to the serial console to show we started
  Serial.println(" --- Serial Monitor started ---");
  Serial.println(" ------------------------------");
  Serial.println();
}

void loop() {
  // read the sensor input
  sensorSTATE = digitalRead(sensorPIN);

  // check if the sensor picked up anything
  if (sensorSTATE == HIGH) {
    // turn internal led on
    digitalWrite(ledPIN, HIGH);
    // send status message to the serial monitor
    Serial.println(" Button is pushed, pin 13 LED is HIGH (on)");
  } else {
    // turn internal led off
    digitalWrite(ledPIN, LOW);
    // send status message to the serial monitor
    Serial.println(" Button is not pushed, pin 13 LED is LOW (off)");
  }
// return to the beginning
}
