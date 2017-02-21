
// Pocket Operator signal...
#define POCKET_OPERATOR_SYNC_SIGNAL 4
#define POCKET_OPERATOR_SYNC_SIGNAL_TIME 5

// button constants...
#define BUTTON 1
#define BUTTON_DEBOUNCE_TIME 10

#define TICKS_TO_RESET_COUNTER 1000

// for OLED display - library located here: https://bitbucket.org/tinusaur/ssd1306xled
// changed files from .c to .cpp to build with Arduino...
#define SSD1306_SCL   PB2 // SCL, Pin 3 on SSD1306 Board
#define SSD1306_SDA   PB0 // SDA, Pin 4 on SSD1306 Board
#define SSD1306_SA    0x78  // Slave address

#include "ssd1306xled.h"
#include "ssd1306xled8x16.h"

volatile uint32_t tick = 0;
volatile uint32_t lastButtonPressTick = 0;
volatile uint32_t firstTapTick = 0;
volatile uint8_t consecutiveButtonTaps = 0;
volatile uint16_t pocketOperatorTimeBetweenPulses = 120;

void setup() {
  cli(); // turn off interupts...
  
  // setup timer0
  bitSet(TCCR0A, WGM01); // CTC mode. set WGM01
  // divide by 64
  bitSet(TCCR0B, CS00);
  bitSet(TCCR0B, CS01);
  bitClear(TCCR0B, CS02);
  OCR0A = 125; // timer 0 value - 8,000,000 / 64 / 125 ≈ 1ms
  bitSet(TIMSK, OCIE0A); // set timer to interrupt
  
  // setup button....
  pinMode(BUTTON, INPUT); // button is input... (button on 1 is the same as led on trinket - normally low - pressed = high)
  
  // setup sync signal...
  pinMode(POCKET_OPERATOR_SYNC_SIGNAL, OUTPUT);
  
  sei(); // turn on interupts...
  
  
  // setup OLED...
  delay(40);
  ssd1306_init();
  ssd1306_clear();
  delay(20);
  updateDisplay();
}

uint8_t buttonPressed = 0;
uint32_t totalTapTicks = 0;
uint8_t pocketOperatorPulse;

void loop() {

  // if tick is a multiple of time between pulses start a sync signal...
  if(tick % pocketOperatorTimeBetweenPulses == 0) {
    digitalWrite(POCKET_OPERATOR_SYNC_SIGNAL, HIGH);
    pocketOperatorPulse = POCKET_OPERATOR_SYNC_SIGNAL_TIME;
  }
  else {
    pocketOperatorPulse--;
    if(pocketOperatorPulse == 0) {
      digitalWrite(POCKET_OPERATOR_SYNC_SIGNAL, LOW);
    }
  }
  
  // tap button handling...
  if(buttonPressed) {
    buttonPressed = 0;
    if (consecutiveButtonTaps > 0) {
      totalTapTicks = tick - firstTapTick;
      pocketOperatorTimeBetweenPulses = (totalTapTicks / consecutiveButtonTaps) / 2; // Pocket Operator seems to sync on 2 beats per measure...
      
      // 
    }
    else {
      firstTapTick = tick;
    }
    consecutiveButtonTaps++;
    lastButtonPressTick = tick;
    updateDisplay();
  }
  else {
    if ((consecutiveButtonTaps > 0) && (tick > lastButtonPressTick + TICKS_TO_RESET_COUNTER)) {
      consecutiveButtonTaps = 0;
      updateDisplay();
    }
  }
  
  
}

volatile uint8_t buttonDown = 0;

uint8_t checkButton() {
  uint8_t returnValue = 0;
  if(digitalRead(BUTTON)) {
    buttonDown++;
  }
  else {
    if(buttonDown > 10) {
      returnValue = 1; // set return value on release....
    }
    buttonDown = 0;
  }
  return returnValue;
}

void updateDisplay() {
  ssd1306_clear();
  ssd1306_setpos(5, 0);
  ssd1306_string_font6x8("TAP-POCKET-OPERATOR");
  ssd1306_setpos(5, 2);
  ssd1306_string_font6x8("BPM: ");
  ssd1306_numdec_font6x8(60000 / (pocketOperatorTimeBetweenPulses * 4));
  ssd1306_setpos(5, 4);
  ssd1306_string_font6x8("TAPS: ");
  ssd1306_numdec_font6x8(consecutiveButtonTaps);
  // ssd1306_setpos(5, 6);
  // ssd1306_numdec_font6x8(tick);
  // ssd1306_string_font6x8(" - ");
  // ssd1306_numdec_font6x8(millis());
}


// timer interupt...
ISR (TIMER0_COMPA_vect) {
  if(checkButton()) {
    buttonPressed = 1;
  }
  else {
    buttonPressed = 0;
  }
  tick++;
}







