/* rawSend.ino Example sketch for IRLib2
 *  adapted to control old LEGO IR devices
 *  contains signals calculated by 'ir_generate.py'
 */
 
#include <Arduino.h>
#include "IRLibSendBase.h"    //We need the base code
#include "IRLib_HashRaw.h"    //Only use raw sender
#include "IR_patterns.h"
#include "IR_functions.h"
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/interrupt.h>

//The TX and RX LEDs are available if we aren't using USB
// TXLED0  PORTD |= (1<<5)   // TX off
// TXLED1  PORTD &= ~(1<<5)  // TX on
// RXLED0  PORTB |= (1<<0)   // RX off
// RXLED1  PORTB &= ~(1<<0)  // RX on

#define PROGRAMMING_PIN 5 // pull down this pin to disable power saving modes and keep USB alive

#define LEFT_REV_PB  3   // INT0 / PD0
#define LEFT_FWD_PB  2   // INT1 / PD1
#define RIGHT_REV_PB 0   // INT2 / PD2
#define RIGHT_FWD_PB 1   // INT3 / PD3

// PD0=LEFT_REV, PD1=LEFT_FWD, PD2=RIGHT_REV, PD3=RIGHT_FWD
#define BUTTON_MASK 0x0F  // bits 0-3

bool programming_mode = false;

#define SLEEP_TIMEOUT 500

// ── ISRs: disable themselves immediately, just record which button ──
ISR(INT0_vect) { EIMSK &= ~(1 << INT0); }
ISR(INT1_vect) { EIMSK &= ~(1 << INT1); }
ISR(INT2_vect) { EIMSK &= ~(1 << INT2); }
ISR(INT3_vect) { EIMSK &= ~(1 << INT3); }

// enable the interrupts to prepare for sleep
void enableWakeInterrupts() {
    // Trigger on LOW level
    EICRA = 0x00;   // INT0–INT3 all set to LOW level (00 = low level)
    // Clear any stale flags first
    EIFR = (1 << INTF0) | (1 << INTF1) | (1 << INTF2) | (1 << INTF3);
    // Enable all four
    EIMSK |= (1 << INT0) | (1 << INT1) | (1 << INT2) | (1 << INT3);
}

void enterSleep() {
    // Power down peripherals you don't need
    TXLED0;
    power_usart0_disable();
    power_spi_disable();
    power_twi_disable();
    power_timer0_disable();
    power_timer1_disable();
    power_timer2_disable();
    power_timer3_disable();

    enableWakeInterrupts();

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();

    sei();              // interrupts must be on to wake
    sleep_cpu();        // ← CPU halts here

    // ── resumes here after ISR runs ──
    sleep_disable();

    // Re-enable peripherals as needed
    power_timer0_enable();  // millis() needs this
    //power_usart0_enable();
    //power_spi_enable();
    //power_twi_enable();

    power_timer1_enable();
    power_timer2_enable();
    power_timer3_enable();

    TXLED1; // turn on the tx led
}
 
//returns true if a button was pressed, false otherwise
// reads the buttons on port D and packs them into 4 bits
// Buttons are active LOW (INPUT_PULLUP), so invert with ~
// PD0=LEFT_REV, PD1=LEFT_FWD, PD2=RIGHT_REV, PD3=RIGHT_FWD
bool handleButtons(void) {
  uint8_t button_bits = (~PIND) & BUTTON_MASK;

  if (programming_mode){
    Serial.println(button_bits);
  }

  send_all(button_bits);

  if (button_bits){
    return true;
  }
  return false;
}

void setup() {
  pinMode(PROGRAMMING_PIN, INPUT_PULLUP);
  pinMode(LEFT_FWD_PB, INPUT_PULLUP);
  pinMode(LEFT_REV_PB, INPUT_PULLUP);
  pinMode(RIGHT_FWD_PB, INPUT_PULLUP);
  pinMode(RIGHT_REV_PB, INPUT_PULLUP);

  if (!digitalRead(PROGRAMMING_PIN)){
    programming_mode = true;
    Serial.begin(9600);
    delay(200); 
    while (!Serial); //delay for Leonardo
    Serial.println("Programming mode is active, CPU will not sleep.");
  }else{
    // set up to use usb leds
    // Disable USB to save power and free the LED pins
    USBCON |= (1 << FRZCLK);   // freeze USB clock
    PLLCSR &= ~(1 << PLLE);    // stop PLL
    USBCON &= ~(1 << USBE);    // disable USB

    // Now take full control of both LED pins
    DDRB |= (1 << PB0);
    DDRD |= (1 << PD5);

    // Start with both off (active LOW)
    PORTB |= (1 << PB0);
    PORTD |= (1 << PD5); 
  }
  TXLED1; // turn on the tx led
}

void loop() {
  static uint32_t timeout = 0;
  timeout = millis();
  
  while ((timeout + SLEEP_TIMEOUT) > millis()){
    if (handleButtons()){
      timeout = millis(); //reset timeout
    }
    delay(25); // dwell time between transmissions
  }

  if (!programming_mode){ // dont sleep in programming mode
    enterSleep();
  }

}
