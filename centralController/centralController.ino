#include <VirtualWire.h>

const byte toggleDoorPin = 2;
const byte doorSensorPin = 3;
const byte receiverPin = 11;
const byte receivingIndicatorPin = 8;

const char *closeRfids[] = { NULL };
const char *openRfids[] = { NULL };
const char *toggleRfids[] = {
  // Sticker in wallet
  "E2791DC4",
  // Small tag sewn into left cap-touch bike glove
  "D27DE539",
  NULL
};

// Track when we last attempted to change the state of the door, to prevent thrashing it.
unsigned long lastCommandSent;

void setup()
{
  Serial.begin(9600);
  
  vw_set_rx_pin(receiverPin);
  vw_setup(2000);
  
  pinMode(receivingIndicatorPin, OUTPUT);
  digitalWrite(receivingIndicatorPin, HIGH);
  
  pinMode(toggleDoorPin, OUTPUT);
  pinMode(doorSensorPin, INPUT);
 
  lastCommandSent = 0;
 
  vw_rx_start();
  
  Serial.println("Send me your tag");
}

boolean rfidMatches(char *rfid, const char **options)
{
  int optionIndex = 0;
  while (true) {
    const char *candidate = options[optionIndex++];
    if (candidate == NULL) {
      return false;
    }
    Serial.println("Comparing rfids");
    Serial.println(rfid);
    Serial.println(candidate);
    if (strcmp(candidate, rfid) == 0) {
      Serial.println("Matched one");
      return true;
    }
  }
}

boolean isDoorClosed()
{
  // Magnetic sensor being used is wired to +5V on one end.
  // When the door is OPEN, the circuit is CLOSED -> +5V
  // When the door is CLOSED, the magnet causes the reed switch
  // to pull away, causing the circuit to pop OPEN -> 0V
  // However, the reed flutters a bit, causing the reading
  // to fluctuate.  Therefore, we can't take a single +5V
  // reading as authoritative; however, a single 0V reading
  // is generally good enough.
  // Demand at least 20 samples over at least 500 ms to decide
  // that the door is actually open.
  unsigned long samplingStart = millis();
  int sampleCount = 0;
  while (true) {
    boolean doorAppearsClosed = (digitalRead(doorSensorPin) == LOW);
    Serial.println("Door sensor? " + String(doorAppearsClosed ? "LOW" : "HIGH"));
    Serial.println("Door? " + String(doorAppearsClosed ? "CLOSED" : "OPEN"));
    if (doorAppearsClosed) {
      return true;
    }
    if (++sampleCount >= 20 && (millis() - samplingStart) >= 500) {
      return false;
    }
  }
}

void toggleDoor()
{
  // Flip active for 1 second
  unsigned long chargeDelay = millis();
  digitalWrite(toggleDoorPin, HIGH);
  while (millis() - chargeDelay < 1000);
  lastCommandSent = millis();
  // Flip inactive for 1 second
  unsigned long drainDelay = millis();
  digitalWrite(toggleDoorPin, LOW);
  while (millis() - drainDelay < 1000);
}

void openDoor()
{
  if (isDoorClosed()) {
    Serial.println("Door is closed, will be opened");
    toggleDoor();
  }
}

void closeDoor()
{
  if (!isDoorClosed()) {
    Serial.println("Door is open, will be closed");
    toggleDoor();
  }
}

void loop()
{
  char buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  if (vw_get_message((uint8_t *)buf, &buflen)) {
    // Flash LED to indicate receipt of valid message.
    unsigned long ledDelay = millis();
    digitalWrite(receivingIndicatorPin, LOW);
    
    // Message with a good checksum received, print it.
    Serial.print("Got: ");
    buf[buflen] = '\0';
    Serial.println(buf);
    
    // When sufficient LED time has elapsed, turn off LED.
    while (millis() - ledDelay < 200);
    digitalWrite(receivingIndicatorPin, HIGH);
    
    // Determine whether we must open, close, or toggle the door.
    if (millis() - lastCommandSent > 5000) {
      if (rfidMatches(buf, toggleRfids)) {
        toggleDoor();
      } else if (rfidMatches(buf, openRfids)) {
        openDoor();
      } else if (rfidMatches(buf, closeRfids)) {
        closeDoor();
      }
    }
  }
}
