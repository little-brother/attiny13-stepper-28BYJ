// 28BYJ
#include <avr/io.h>
#include <avr/pgmspace.h>

//                        +-\/-+
//      --- A0 (D5) PB5  1|°   |8  Vcc
// CTRL --- A3 (D3) PB3  2|    |7  PB2 (D2) A1 --- INT3
// INT4 --- A2 (D4) PB4  3|    |6  PB1 (D1) ------ INT2
//                  GND  4|    |5  PB0 (D0) ------ INT1
//                        +----+  

const byte STEPPER_PINS[] = {PB0, PB1, PB2, PB4};
#define CONTROL_PIN  A3

#define CONTROL_STEP_COUNT 8
const uint8_t CONTROL_STEPS[] PROGMEM = {
  0b00000001,
  0b00000011,
  0b00000010,  
  0b00000110,
  0b00000100,
  0b00001100,
  0b00001000,  
  0b00001001
};

volatile byte isForward = 1;
volatile uint16_t counter = 0;
volatile uint16_t remainSteps = 1;

int main (void) {
  TCCR0B = _BV(CS01); // clock frequency / 8 - http://reefwingrobotics.blogspot.com/2019/09/programming-atmega328p-registers-and.html
  OCR0B = 0x00;  // Output compare
  TCNT0 = 0; // Set counter 0 to zero
  TIMSK0 = _BV(TOIE0); // Enable overflow interrupt
  sei();

  for (byte pinNo = 0; pinNo < 4; pinNo++)
    pinMode(STEPPER_PINS[pinNo], OUTPUT);

  pinMode(CONTROL_PIN, INPUT); 
  
  while(1);
}

ISR(TIM0_OVF_vect) {
  counter++;
  if (counter < 5)
    return;

  counter = 0;
  remainSteps--;
  if (remainSteps == 0) {
    isForward = !isForward;
    uint16_t val = analogRead(CONTROL_PIN);
    remainSteps = map(val, 0, 1024, 10, 4096); // 4096 is one turn 
  } else {
    uint16_t stepNo = remainSteps % CONTROL_STEP_COUNT;
    if (!isForward)
      stepNo = CONTROL_STEP_COUNT - stepNo - 1;

    uint8_t step = pgm_read_byte(&CONTROL_STEPS[stepNo]);  
    for (byte pinNo = 0; pinNo < 4; pinNo++) 
      digitalWrite(STEPPER_PINS[pinNo], bitRead(step, pinNo));    
  }  
}