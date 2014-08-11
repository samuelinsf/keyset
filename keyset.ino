/* 
A USB adapter for the Doug Englebart 5 key keyset
http://www.dougengelbart.org/firsts/keyset.html

Runs on the Arduino Micro
http://arduino.cc/en/Main/arduinoBoardMicro

by github.com/samuelinsf August 2014
This code is released under the terms of the GNU Public License version 3
*/

// https://github.com/thomasfredericks/Bounce-Arduino-Wiring
#include <Bounce2.h>

#define DEBOUNCE_INTERVAL 5
#define CHORD_SETTLE_MSEC 75

#define FIRST_PIN 2

#define META_NONE    0
#define META_CAPITAL 1
#define META_NUMBER  2
#define META_EXTRA   3
byte do_meta = META_NONE;

Bounce keyset_buttons[5];
byte last_state = 0;
byte chord_processed = 1;

unsigned long last_change;

byte show_help = 0;
void help() {
  Keyboard.println("# NLS Keyset to USB Adapter");
  Keyboard.println("# by Samuel Stoller for Ted Nelson");
  Keyboard.println("# August 2014, San Francisco");
  Keyboard.println("# Chord    Types    Shift    Numeric    Extra");
  Keyboard.println("# ============================================");  
  Keyboard.println("# 00001    a        A        !           [");
  Keyboard.println("# 00010    b        B        @           ]");
  Keyboard.println("# 00011    c        C        #           _");
  Keyboard.println("# 00100    d        D        $           {");
  Keyboard.println("# 00101    e        E        %           }");
  Keyboard.println("# 00110    f        F        ^           |");
  Keyboard.println("# 00111    g        G        &           `");
  Keyboard.println("# 01000    h        H        *           ~");
  Keyboard.println("# 01001    i        I        (           \\");
  Keyboard.println("# 01010    j        J        )");
  Keyboard.println("# 01011    k        K        -");
  Keyboard.println("# 01100    l        L        +");
  Keyboard.println("# 01101    m        M        =");
  Keyboard.println("# 01110    n        N        ?");
  Keyboard.println("# 01111    o        O        \"");
  Keyboard.println("# 10000    p        P        0");
  Keyboard.println("# 10001    q        Q        1");
  Keyboard.println("# 10010    r        R        2");
  Keyboard.println("# 10011    s        S        3");
  Keyboard.println("# 10100    t        T        4");
  Keyboard.println("# 10101    u        U        5");
  Keyboard.println("# 10110    v        V        6");
  Keyboard.println("# 10111    w        W        7");
  Keyboard.println("# 11000    x        X        8");
  Keyboard.println("# 11001    y        Y        9");
  Keyboard.println("# 11010    z        Z        /");
  Keyboard.println("# 11011    Shift    ESCAPE   :");
  Keyboard.println("# 11100    Numeric  '        .");
  Keyboard.println("# 11101    Extra    |        ,");
  Keyboard.println("# 11110    DELETE   <        TAB");
  Keyboard.println("# 11111    SPACE    >        RETURN");
  Keyboard.println("# Pressing 11101 three times in a row types this help message.");
}

void setup() {
  // Setup input pins, and debounce
  int i;
  for (i=0; i<5; i++) {
    int pin;
    pin = FIRST_PIN + i;
    pinMode(pin, INPUT);
    digitalWrite(pin, HIGH);
    keyset_buttons[i] = Bounce();
    keyset_buttons[i].attach(pin);
    keyset_buttons[i].interval(DEBOUNCE_INTERVAL);
  }
  
  Keyboard.begin();  
  last_change = millis();
}

void loop() {
  int i;
  int saw_keydown;
  int saw_keychange;
  int pin_value;
  int keyset_value;
  
  saw_keydown = 0;  
  saw_keychange = 0;
  keyset_value = 0;
  
  for (i=0; i<5; i++) {
    int button_value;
    
    if (keyset_buttons[i].update()) {
      saw_keychange = 1;
      pin_value = keyset_buttons[i].read();
      if (pin_value == LOW) {
        last_change = millis();
      }
    }
    pin_value = keyset_buttons[i].read();

    if (pin_value == LOW) {
      button_value = 1;
    }
    else {
      button_value = 0;
    }
    keyset_value = keyset_value + (button_value << i);
  } /* for */
  
  if ((keyset_value == 0) && (chord_processed == 1)) {
    chord_processed = 0;
    Keyboard.releaseAll();
   }
 
 if (millis() - last_change < 0) {
   // counter wrap!
   last_change = millis();
 }
  
  if (((millis() - last_change) > CHORD_SETTLE_MSEC) && (! chord_processed == 1) && (keyset_value != 0)) {
    // Chord has settled, interpret it
    chord_processed = 1;
    if (do_meta == META_NONE) {
      if (keyset_value <= 26) {
        Keyboard.press(('a' - 1) + keyset_value);
        show_help = 0;
      }
      else if (keyset_value == 27) {
        // 11011
        do_meta = META_CAPITAL;
      }
      else if (keyset_value == 28) {
        // 11100
        do_meta = META_NUMBER;
      }
      else if (keyset_value == 29) {
        // 11101
        do_meta = META_EXTRA;
        show_help++;
      }
      else if (keyset_value == 30) {
        // 11110
        Keyboard.press(KEY_BACKSPACE);
      }
      else if (keyset_value == 31) {
        // 11111
        Keyboard.press(' ');
      }
    }
    else if (do_meta == META_CAPITAL) {
      // First press 11011 then capitals or these extra keys are active
      do_meta = META_NONE;
      show_help = 0;
      if (keyset_value <= 26) {
        Keyboard.press(('A' - 1) + keyset_value);
      }
      else {
        char keytable[] = {KEY_ESC, 39 /* ' */, '|', '<', '>'};
        Keyboard.press(keytable[keyset_value - 27]);
      }
    }
    else if (do_meta == META_NUMBER) {
      // First press 11100, then these keys are active
      do_meta = META_NONE;
      show_help = 0;
      char keytable[] = {'X', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '-', '+', '=', '?', '"',
                         '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '/', ':', '.', ',', KEY_TAB, KEY_RETURN };
      Keyboard.press(keytable[keyset_value]);
    }
    else if (do_meta == META_EXTRA) {
      // press 11101
      do_meta = META_NONE;
      char keytable[] = {'X', '[', ']', '_', '{', '}', '|', '`', '~', '\\', ';'};
      if (keyset_value < (int) sizeof(keytable)) {
        show_help = 0;
        Keyboard.press(keytable[keyset_value]);
      }
      else if (keyset_value == 29) {
        do_meta = META_EXTRA;
        show_help++;
        if (show_help >= 3) {
          show_help = 0;
          help();
          do_meta = META_NONE;
        }
      }
    }
  }
}
