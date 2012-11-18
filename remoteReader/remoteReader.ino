#include <SL018.h>
#include <VirtualWire.h>
#include <Wire.h>

const byte transmitterPin = 10;
const byte transmittingIndicatorPin = 11;

SL018 rfid;

void setup()
{
  Wire.begin();
  
  Serial.begin(9600);
  
  vw_set_tx_pin(transmitterPin);
  vw_setup(2400);
  
  pinMode(transmittingIndicatorPin, OUTPUT);
  digitalWrite(transmittingIndicatorPin, HIGH);  
}

void transmitMessage(const char *msg)
{
  // Send the tag's id.
  digitalWrite(transmittingIndicatorPin, LOW);
  vw_send((uint8_t *)msg, strlen(msg));
  vw_wait_tx();
  digitalWrite(transmittingIndicatorPin, HIGH);
}

void loop()
{
  // start seek mode
  Serial.println("Show me your tag");
  rfid.seekTag();
  // wait until tag detected
  while(!rfid.available());
  Serial.println("Tag detected");
  Serial.println(rfid.getTagString());
  // transmit tag id
  transmitMessage(rfid.getTagString());
}
