#include <VirtualWire.h>

const byte receiverPin = 11;
const byte receivingIndicatorPin = 8;

void setup()
{
  Serial.begin(9600);
  
  vw_set_rx_pin(receiverPin);
  vw_setup(2000);
  
  pinMode(receivingIndicatorPin, OUTPUT);
  digitalWrite(receivingIndicatorPin, HIGH);
 
 vw_rx_start(); 
}

void loop()
{
  Serial.println("Awaiting transmission");
  vw_wait_rx();
  digitalWrite(receivingIndicatorPin, LOW);
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  if (vw_get_message(buf, &buflen)) {
    int i;
    // Message with a good checksum received, dump it.
    Serial.print("Got: ");
    
    for (i = 0; i < buflen; i++)
    {
      Serial.print((char)buf[i]);
    }
    Serial.println("");
    digitalWrite(13, false);
  }
  digitalWrite(receivingIndicatorPin, HIGH);
}
