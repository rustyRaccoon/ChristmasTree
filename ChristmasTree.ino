/*
Author:	rustyRaccoon
Date:		2022-12-22
Version:	1.0
*/

/*
Randomly cycles through all 24 LEDs on the tree.
Keeps each LED on for 700ms.
Uses https://forum.arduino.cc/t/random-option-from-array-without-repetition/484510/6
*/
// define attiny pins
#define DATA PB0
#define CLK PB1
#define LATCH PB2
#define SEED PB3

// changing variables
uint16_t sleepTime;    // wait time between LEDs [ms]
uint32_t lastTime;     // last timestamp for delay
uint32_t data;         // 32-bits for 24 LEDs
uint8_t numBits;       // tracks number of bits active in current data value
uint8_t bitArray[24];  // keeps all bits we can light up
uint8_t usedBits;      // tracks how many bits were already lit up

// Prep and setup stuff goes here...
void setup() {
  cli();  // disable interrupts during setup

  pinMode(DATA, OUTPUT);   // set serial data as output
  pinMode(CLK, OUTPUT);    // set shift register clock as output
  pinMode(LATCH, OUTPUT);  // set output register (latch) clock as output
  pinMode(SEED, INPUT);

  // default conditions
  data = 0;
  sleepTime = 250;
  lastTime = 0;
  randomSeed(getSeed());
  resetArray();  //default fill array with all values

  // Serial.begin(19200);

  // for (int i = 0; i <= 23; i++){
  //   Serial.print(i);
  //   Serial.print(": ");
  //   Serial.println(bitArray[i]);
  // }
  // Serial.println("");

  sei();  // enable interrupts after setup
}

// Main loop
void loop() {
  if ((millis() - lastTime) >= sleepTime) {  // if delay time is over...
    lastTime = millis();    // update timestamp
    randomSeed(getSeed());  // get new random seed
    data = 0;
    numBits = 0;

    // fetch me four bits (and set them, obviously)
    while (numBits < 4) {
      data |= (1ul << selectRandomBit());
      //Serial.println(data,BIN);
      numBits++;
    }

    // Serial.print(data,BIN);
    // Serial.println(",");
    // Serial.println("Done with output bits. Shifting out.");

    //Shift the data out
    digitalWrite(LATCH, 0);          // set latch pin LOW so nothing gets shifted out
    shiftOutShort(DATA, CLK, data);  // shift out LED states in that layer
    digitalWrite(LATCH, 1);          // sent everything out in parallel

    // Serial.println("Shifting out done. Sleeping for " + sleepTime);
  }
}

// Select random entry from array, from remaining choices
uint8_t selectRandomBit() {
  //check if we've run out of values
  if (usedBits > 23) {
    resetArray();
  }

  // Select random value (from values not yet used) and shift value used to the beginning
  // of the array to skip it next time (by incrementing a counter where to start the random
  // range). This effectively skips all values already used.
  uint8_t selection = random(usedBits, 23);  // select random index from not yet used indices
  uint8_t temp = bitArray[usedBits];         // save value at current position to temporary variable
  bitArray[usedBits] = bitArray[selection];  // overwrite current array position with value at random position from before
  bitArray[selection] = temp;                // overwrite random position from before with previously saved current position

  // Serial.print("Chosen bit: ");
  // Serial.println(bitArray[usedBits]);
  return bitArray[usedBits++];  // return value at current position + 1
}

// Refills the array with all values
void resetArray() {
  // refill choices in ascending order
  for (int i = 0; i <= 23; i++) {
    bitArray[i] = i;
  }

  // scramble the array of available choices to generate a random order
  for (int i = 0; i <= 23; i++) {
    int index = random(i, 23);      // generate random number between i and max value
    int temp = bitArray[i];         // save i-th value to temporary variable
    bitArray[i] = bitArray[index];  // overwrite i-th value with random position from before
    bitArray[index] = temp;         // overwrite random position from before with previously i-th value
  }

  usedBits = 0;  // reset counter for how many values were picked already
  // Serial.println(";");
}

// Generates a random see from reading the three LSBs of an input pin and combining them into a 12 bit number
uint16_t getSeed(void) {
  uint16_t seedValue = 0;

  seedValue |= (analogRead(SEED) & 0b0111);
  seedValue |= (analogRead(SEED) & 0b0111) << 3;
  seedValue |= (analogRead(SEED) & 0b0111) << 6;
  seedValue |= (analogRead(SEED) & 0b0111) << 9;
}

// This shifts 24 bits out MSB first, on the rising edge of the clock.
void shiftOutShort(uint8_t dataPin, uint8_t clockPin, uint32_t toBeSent) {
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
    if (toBeSent & (1UL << i)) {
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