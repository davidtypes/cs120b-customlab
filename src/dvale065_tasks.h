#ifndef CUSTOMPROJECT120B_TASKS_H
#define CUSTOMPROJECT120B_TASKS_H

typedef struct task {
  int state;
  unsigned long period;
  unsigned long elapsedTime;
  int (*TickFct)(int);
} task;

extern const unsigned long INPUT_PERIOD;
extern const unsigned long PADDLE_PERIOD;
extern const unsigned long DISPLAY_PERIOD;
extern const unsigned long GCD_PERIOD;

extern const unsigned char numTasks;
extern task tasks[];

void ADC_init();
unsigned short ADC_read(unsigned char channel);

void gameInit();

int TickFct_Input(int state);
int TickFct_Paddle(int state);
int TickFct_Display(int state);

#endif // CUSTOMPROJECT120B_TASKS_H