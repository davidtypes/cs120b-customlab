#ifndef CUSTOMPROJECT120B_TASKS_H
#define CUSTOMPROJECT120B_TASKS_H

#define JOYSTICK_X_CHANNEL 0
#define JOYSTICK_X_BIT 0
#define RESET_BUTTON_BIT 2

typedef struct task {
  int state;
  unsigned long period;
  unsigned long elapsedTime;
  int (*TickFct)(int);
} task;

extern const unsigned long INPUT_PERIOD;
extern const unsigned long RESET_PERIOD;
extern const unsigned long PADDLE_PERIOD;
extern const unsigned long BALL_PERIOD;
extern const unsigned long DISPLAY_PERIOD;
extern const unsigned long GCD_PERIOD;

extern const unsigned char numTasks;
extern task tasks[];

void ADC_init();
unsigned short ADC_read(unsigned char channel);

int TickFct_Input(int state);
int TickFct_Reset(int state);
int TickFct_Paddle(int state);
int TickFct_Ball(int state);
int TickFct_Display(int state);

#endif // CUSTOMPROJECT120B_TASKS_H
