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
}

void loop() {
  // read the sensor input
  sensorSTATE = digitalRead(sensorPIN);

  // check if the sensor picked up anything
  if (sensorSTATE = HIGH) {
    // turn internal led on
    digitalWrite(ledPIN, HIGH);
  } else {
    // turn internal led off
    digitalWrite(ledPIN, LOW);
  }
// return to the beginning
}
