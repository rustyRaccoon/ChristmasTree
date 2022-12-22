/*
Author:	rustyRaccoon
Date:		2022-12-22
Version:	1.0
*/

/*
D12 D9 D7

Randomly cycles through all 24 LEDs on the tree.
Keeps each LED on for 700ms.
Uses https://forum.arduino.cc/t/random-option-from-array-without-repetition/484510/6
*/
// define attiny pins
#define DATA PB0
#define CLK PB1
#define LATCH PB2

// changing variables
uint32_t data;      // 32-bits for 24 LEDs
uint8_t counter;    // just to count from 0 to 23

// Prep and setup stuff goes here...
void setup() {
  cli();  // disable interrupts during setup

  pinMode(DATA, OUTPUT);   // set serial data as output
  pinMode(CLK, OUTPUT);    // set shift register clock as output
  pinMode(LATCH, OUTPUT);  // set output register (latch) clock as output

  // default conditions
  data = 0;
  counter = 0;

  sei();  // enable interrupts after setup
}

// Main loop
void loop() {
  data = 0;
  data |= (1ul << counter); 

  //Shift the data out
  digitalWrite(LATCH, 0);          // set latch pin LOW so nothing gets shifted out
  shiftOutData(DATA, CLK, data);  // shift out LED states in that layer
  digitalWrite(LATCH, 1);          // sent everything out in parallel
  
  if(counter > 23){
    counter = 0;
  }
  else{
    counter++;
  }

  delay(1000);
}

// This shifts 24 bits out MSB first, on the rising edge of the clock.
void shiftOutData(uint8_t dataPin, uint8_t clockPin, uint32_t toBeSent) {
  int i = 0;
  uint8_t pinState = 0;

  // clear everything out just in case
  digitalWrite(dataPin, 0);
  digitalWrite(clockPin, 0);

  // loop through bits in the data bytes counting DOWN in the for loop so that %00000001 or "1"
  // will go through such that it will be pin Q0 that lights.
  for (i = 23; i >= 0; i--) {
    digitalWrite(clockPin, 0);
    // if the value passed AND a bitmask result is true then set pinState to 1
    if (toBeSent & (1ul << i)) {
      pinState = 1;
    } else {
      pinState = 0;
    }

    digitalWrite(dataPin, pinState);  // sets the pin to HIGH or LOW depending on pinState
    digitalWrite(clockPin, 1);        // shifts bits on upstroke of clock pin
    digitalWrite(dataPin, 0);         // zero the data pin after shift to prevent bleed through
  }
  digitalWrite(clockPin, 0);  // stop shifting
}