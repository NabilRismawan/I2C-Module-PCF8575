#include <Arduino.h>
#include <I2cDiscreteIoExpander.h>

#define PIN_PCF_INTERRUPT_INPUT   3
#define PIN_GOT_INTERRUPT_INDICATOR 10

#define ADDRESS_BITS_SETTINGS   0b000

// instantiate I2cDiscreteIoExpander object
I2cDiscreteIoExpander device(ADDRESS_BITS_SETTINGS);



volatile bool haveInt = false;
void pcfInt() {
  haveInt = true;
}

void setup()
{
  // initialize i2c interface
  Wire.begin();

  // initialize serial interface
  Serial.begin(115200);

  device.disableBitwiseInversion();

  pinMode(PIN_PCF_INTERRUPT_INPUT, INPUT);
  pinMode(PIN_GOT_INTERRUPT_INDICATOR, OUTPUT);

  flashLED(5);

  device.digitalWrite(0xffff);

  uint16_t val = device.getPorts();
  Serial.print("read 0x");
  Serial.print(val, HEX);
  Serial.println(".");

  Serial.println("ATTACHING INT");
  attachInterrupt(digitalPinToInterrupt(3), pcfInt, FALLING);

  delay(1000);

  Serial.println("Clearing any int/mirroring inputs");
  mirrorInputOntoOutputs();

}


void flashLED(uint8_t n) {
  for (uint8_t i=0; i<n; i++) {
    device.digitalWrite(0x7fff);
    delay(100);
    device.digitalWrite(0xffff);
    delay(80);
  }

}

uint16_t getChangedInputValue() {

    Serial.println("Got INTerrupt");
    device.digitalRead();
    return device.getPorts();

}

void mirrorInputOntoOutputs() {

    uint16_t bothports = getChangedInputValue();

    uint16_t inputs = lowByte(bothports);

    // Mirror the inputs to the outputs
    // leave the inputs, as inputs (i.e. writing high)
    uint16_t newValue = word(inputs, 0xff) ; // (inputs << 8) | 0xff;

    Serial.print("Inputs read as 0x");
    Serial.print(inputs, HEX);
    Serial.println(" being mirrored to out");

    device.digitalWrite(newValue);
    Serial.print("Wrote 2 bytes: port0 -> 0x");
    Serial.print(lowByte(newValue), HEX);
    Serial.print(" port1 -> 0x");
    Serial.println(highByte(newValue), HEX);
}

    
void loop() {
  if (haveInt) {
    haveInt = false;
    digitalWrite(PIN_GOT_INTERRUPT_INDICATOR, HIGH);
    mirrorInputOntoOutputs();
  } else {

    digitalWrite(PIN_GOT_INTERRUPT_INDICATOR, LOW);
  }

  delay(50);

}


