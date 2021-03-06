/* 

  Simple Teensy DIY USB-MIDI controller.

  Created by Rodolfo Palacios, based on a script by Liam Lacey 
  https://ask.audio/articles/how-to-build-a-simple-diy-usb-midi-controller-using-teensy

   Contains 10 push buttons for sending MIDI/Keyboard messages,
   and a toggle switch for setting whether the buttons
   send note messages, CC messages or keyboard keys.

   You must select MIDI from the "Tools > USB Type" menu for this code to compile.

   To change the name of the USB-MIDI device, edit the STR_PRODUCT define
   in the /Applications/Arduino.app/Contents/Java/hardware/teensy/avr/cores/usb_midi/usb_private.h
   file. You may need to clear your computers cache of MIDI devices for the name change to be applied.

   See https://www.pjrc.com/teensy/td_midi.html for the Teensy MIDI library documentation.

   D0  - D9  : MIDI buttons.
   D10 - D11 : MIDI bank buttons. Up/Down.
   D12 - D18 : 7-seg display.
   D19 - D21 : Mode switch.
   A8        : Expression pedal.

*/

#include <Bounce.h>

// The number of push buttons
const int MIDI_BUTTON_COUNT = 10;

// The number of bank buttons (up/down);
const int BANK_BUTTON_COUNT = 2;

// The number of mode switches.
const int SWITCHES_COUNT = 3;

const int ALL_BUTTONS_COUNT = MIDI_BUTTON_COUNT + BANK_BUTTON_COUNT;

// Create Bounce objects for each button and switch. The Bounce object
// automatically deals with contact chatter or "bounce", and
// it makes detecting changes very simple.
// 5 = 5 ms debounce time which is appropriate for good quality mechanical push buttons.
// If a button is too "sensitive" to rapid touch, you can increase this time.

//button debounce time
const int DEBOUNCE_TIME = 5;

Bounce buttons[ALL_BUTTONS_COUNT] = {
  Bounce(0, DEBOUNCE_TIME),
  Bounce(1, DEBOUNCE_TIME),
  Bounce(2, DEBOUNCE_TIME),
  Bounce(3, DEBOUNCE_TIME),
  Bounce(4, DEBOUNCE_TIME),
  Bounce(5, DEBOUNCE_TIME),
  Bounce(6, DEBOUNCE_TIME),
  Bounce(7, DEBOUNCE_TIME),
  Bounce(8, DEBOUNCE_TIME),
  Bounce(9, DEBOUNCE_TIME),
  Bounce(10, DEBOUNCE_TIME),
  Bounce(11, DEBOUNCE_TIME)
};

Bounce switches[SWITCHES_COUNT] = {
  Bounce(12, DEBOUNCE_TIME),
  Bounce(13, DEBOUNCE_TIME),
  Bounce(14, DEBOUNCE_TIME)
};

const int MODE_MIDI_NOTES = 0;
const int MODE_MIDI_CCS = 1;
const int MODE_KEYBOARD = 2;

//Variable that stores the current MIDI mode of the device (what type of messages the push buttons send).
int operationMode = MODE_MIDI_NOTES;

// the MIDI channel number to send messages
const int MIDI_CHANNEL = 1;

//Arrays the store the exact note and CC messages each push button will send.
const int MIDI_NOTE_NUMS[MIDI_BUTTON_COUNT] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
const int MIDI_NOTE_VELS[MIDI_BUTTON_COUNT] = {127, 127, 127, 127, 127, 127, 127, 127, 127, 127};
const int MIDI_CC_NUMS[MIDI_BUTTON_COUNT] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
const int MIDI_CC_VALS[MIDI_BUTTON_COUNT] = {127, 127, 127, 127, 127, 127, 127, 127, 127, 127};

// TODO: Initialize array for keyboard values.

//The setup function. Called once when the Teensy is turned on or restarted
void setup() {

  // Configure the pins for input mode with pullup resistors.
  // The buttons/switch connect from each pin to ground.  When
  // the button is pressed/on, the pin reads LOW because the button
  // shorts it to ground.  When released/off, the pin reads HIGH
  // because the pullup resistor connects to +5 volts inside
  // the chip.  LOW for "on", and HIGH for "off" may seem
  // backwards, but using the on-chip pullup resistors is very
  // convenient.  The scheme is called "active low", and it's
  // very commonly used in electronics... so much that the chip
  // has built-in pullup resistors!

  for (int i = 0; i < ALL_BUTTONS_COUNT + SWITCHES_COUNT; i++) {
    pinMode (i, INPUT_PULLUP);
  }

}

//The loop function. Called over-and-over once the setup function has been called.
void loop() {

  // Update all the buttons/switch. There should not be any long
  // delays in loop(), so this runs repetitively at a rate
  // faster than the buttons could be pressed and released.
  for (int i = 0; i < ALL_BUTTONS_COUNT; i++) {
    buttons[i].update();
  }

  for (int i = 0; i < SWITCHES_COUNT; i++) {
    switches[i].update();
  }

  // Check the status of each push button
  for (int i = 0; i < MIDI_BUTTON_COUNT; i++) {

    // Check each button for "falling" edge.
    // Falling = high (not pressed - voltage from pullup resistor) to low (pressed - button connects pin to ground)
    if (buttons[i + 1].fallingEdge()) {

      //If in note mode send a MIDI note-on message.
      //Else send a CC message.
      if (operationMode == MODE_MIDI_NOTES) {
        usbMIDI.sendNoteOn(MIDI_NOTE_NUMS[i], MIDI_NOTE_VELS[i], MIDI_CHANNEL);
      } else {
        usbMIDI.sendControlChange(MIDI_CC_NUMS[i], MIDI_CC_VALS[i], MIDI_CHANNEL);
      }
    }

    // Check each button for "rising" edge
    // Rising = low (pressed - button connects pin to ground) to high (not pressed - voltage from pullup resistor)
    else if (buttons[i + 1].risingEdge()) {

      //If in note mode send a MIDI note-off message.
      //Else send a CC message with a value of 0.
      if (operationMode == MODE_MIDI_NOTES) {
        usbMIDI.sendNoteOff MIDI_NOTE_NUMS[i], 0, MIDI_CHANNEL);
      } else {
        usbMIDI.sendControlChange(MIDI_CC_NUMS[i], 0, MIDI_CHANNEL);
      }
    }
  }

  // Check the status of the toggle switch, and set the MIDI mode based on this.
  if (switches[0].fallingEdge()) {
    operationMode = MODE_MIDI_NOTES;
  } else if (buttons[1].fallingEdge()) {
    operationMode = MODE_MIDI_CCS;
  } else {
    operationMode = MODE_KEYBOARD;
  }

  // MIDI Controllers should discard incoming MIDI messages.
  // http://forum.pjrc.com/threads/24179-Teensy-3-Ableton-Analog-CC-causes-midi-crash
  while (usbMIDI.read()) {
    // ignoring incoming messages, so don't do anything here.
  }
  
}
