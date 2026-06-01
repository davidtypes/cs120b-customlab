#ifndef CUSTOMPROJECT120B_TASKS_H
#define CUSTOMPROJECT120B_TASKS_H


typedef struct task {
    int state;
    unsigned long period;
    unsigned long elapsedTime;
    int (*TickFct)(int);
} task;

extern const unsigned long timerPeriod;
extern const unsigned char numTasks;
extern task tasks[];

void Game_Init();
int TickFct_Display(int state);
int TickFct_Paddle(int state);


#endif //CUSTOMPROJECT120B_TASKS_H