#include "dvale065_tasks.h"

#include "dvale065_game.h"
#include "dvale065_helpers.h"
#include <avr/io.h>

#define JOYSTICK_LEFT_THRESHOLD 350
#define JOYSTICK_RIGHT_THRESHOLD 650

static bool resetWasPressed = false;

// lowkey i dont fully know how the adc stuff works
void ADC_init() {
  ADMUX = (1 << REFS0);
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

unsigned short ADC_read(unsigned char channel) {
  channel &= 0x07;
  ADMUX = (ADMUX & 0xF0) | channel;
  ADCSRA |= (1 << ADSC);
  while (ADCSRA & (1 << ADSC)) {}
  return ADC;
}

enum Input_States { INPUT_START, INPUT_READ };
int TickFct_Input(int state) {
  switch (state) {
    case INPUT_START:
      state = INPUT_READ;
      break;
    case INPUT_READ:
      state = INPUT_READ;
      break;
    default:
      state = INPUT_START;
      break;
  }

  switch (state) {
    case INPUT_READ: {
      unsigned short joystickX = ADC_read(JOYSTICK_X_CHANNEL);

      if (joystickX < JOYSTICK_LEFT_THRESHOLD) {
        setInputDirection(-1);
      }
      else if (joystickX > JOYSTICK_RIGHT_THRESHOLD) {
        setInputDirection(1);
      }
      else {
        setInputDirection(0);
      }
      break;
    }
  }

  return state;
}

enum Reset_States { RESET_START, RESET_WAIT };
int TickFct_Reset(int state) {
  switch (state) {
    case RESET_START:
      state = RESET_WAIT;
      break;
    case RESET_WAIT:
      state = RESET_WAIT;
      break;
    default:
      state = RESET_START;
      break;
  }

  switch (state) {
    case RESET_WAIT: {
      bool resetPressed = !GetBit(PIND, RESET_BUTTON_BIT);

      if (resetPressed && !resetWasPressed) {
        joystickPress();
      }

      resetWasPressed = resetPressed;
      break;
    }
  }

  return state;
}

enum Paddle_States { PADDLE_START, PADDLE_UPDATE };
int TickFct_Paddle(int state) {
  switch (state) {
    case PADDLE_START:
      state = PADDLE_UPDATE;
      break;
    case PADDLE_UPDATE:
      state = PADDLE_UPDATE;
      break;
    default:
      state = PADDLE_START;
      break;
  }

  switch (state) {
    case PADDLE_UPDATE:
      paddleTick();
      break;
  }

  return state;
}

enum Ball_States { BALL_START, BALL_UPDATE };
int TickFct_Ball(int state) {
  switch (state) {
    case BALL_START:
      state = BALL_UPDATE;
      break;
    case BALL_UPDATE:
      state = BALL_UPDATE;
      break;
    default:
      state = BALL_START;
      break;
  }

  switch (state) {
    case BALL_UPDATE:
      ballTick();
      break;
  }

  return state;
}

enum Display_States { DISPLAY_START, DISPLAY_UPDATE };
int TickFct_Display(int state) {
  switch (state) {
    case DISPLAY_START:
      state = DISPLAY_UPDATE;
      break;
    case DISPLAY_UPDATE:
      state = DISPLAY_UPDATE;
      break;
    default:
      state = DISPLAY_START;
      break;
  }

  switch (state) {
    case DISPLAY_UPDATE:
      tickDisplay();
      break;
  }

  return state;
}
