// Please read Bounce2.h for information about the liscence and authors

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include "Bounce2.h"

#define DEBOUNCED_STATE 0
#define UNSTABLE_STATE  1
#define STATE_CHANGED   3


Bounce::Bounce()
    : previous_millis(0)
    , interval_millis(10)
    , state(0)
    , pin(0)
{}

void Bounce::attach(int pin) {
    this->pin = pin;
    bool read = digitalRead(pin);
    state = 0;
    if (digitalRead(pin)) {
        state = _BV(DEBOUNCED_STATE) | _BV(UNSTABLE_STATE);
    }
#ifdef BOUNCE_LOCK_OUT
    previous_millis = 0;
#else
    previous_millis = millis();
#endif
}

void Bounce::attach(int pin, int mode){
  pinMode(pin, mode);
  
  this->attach(pin);
}

void Bounce::interval(uint16_t interval_millis)
{
    this->interval_millis = interval_millis;
}

bool Bounce::update()
{
#ifdef BOUNCE_LOCK_OUT
    state &= ~_BV(STATE_CHANGED);
    // Ignore everything if we are locked out
    if (millis() - previous_millis >= interval_millis) {
        bool currentState = digitalRead(pin);
        if ((bool)(state & _BV(DEBOUNCED_STATE)) != currentState) {
            previous_millis = millis();
            state ^= _BV(DEBOUNCED_STATE);
            state |= _BV(STATE_CHANGED);
        }
    }
    return state & _BV(STATE_CHANGED);
#else
    // Read the state of the switch in a temporary variable.
    bool currentState = digitalRead(pin);
    state &= ~_BV(STATE_CHANGED);

    // If the reading is different from last reading, reset the debounce counter
    if ( currentState != (bool)(state & _BV(UNSTABLE_STATE)) ) {
        previous_millis = millis();
        state ^= _BV(UNSTABLE_STATE);
    } else
        if ( millis() - previous_millis >= interval_millis ) {
            // We have passed the threshold time, so the input is now stable
            // If it is different from last state, set the STATE_CHANGED flag
            if ((bool)(state & _BV(DEBOUNCED_STATE)) != currentState) {
                previous_millis = millis();
                state ^= _BV(DEBOUNCED_STATE);
                state |= _BV(STATE_CHANGED);
            }
        }

    return state & _BV(STATE_CHANGED);
#endif
}

bool Bounce::read()
{
    return state & _BV(DEBOUNCED_STATE);
}

bool Bounce::rose()
{
    return ( state & _BV(DEBOUNCED_STATE) ) && ( state & _BV(STATE_CHANGED));
}

bool Bounce::fell()
{
    return !( state & _BV(DEBOUNCED_STATE) ) && ( state & _BV(STATE_CHANGED));
}
