#include "dvale065_tasks.h"

#include "dvale065_TFTLCD.h"
#include <avr/io.h>

#define PADDLE_WIDTH 32
#define PADDLE_HEIGHT 5

#define BALL_SIZE 5 // ball is drawn as a 5x5 square

#define BRICK_WIDTH 18
#define BRICK_HEIGHT 8
#define BRICK_X_SPACE 20
#define BRICK_Y_SPACE 20
#define BRICK_ROWS 3
#define BRICK_COLS 6 // do not change probably...

#define PADDLE_SPEED 2

#define JOYSTICK_X_CHANNEL 0 // A0 / PC0
#define JOYSTICK_LEFT_THRESHOLD 350
#define JOYSTICK_RIGHT_THRESHOLD 650

unsigned char paddleX, paddleY;
unsigned char lastPaddleX, lastPaddleY;
unsigned char ballX, ballY;
unsigned char lastBallX, lastBallY;

bool bricks[BRICK_ROWS][BRICK_COLS];
bool brickBroken;
unsigned char brokenBrickRow, brokenBrickCol;
unsigned char score;

signed char inputDirection; // -1 = left, 0 = neutral, 1 = right
void ADC_init() {
    ADMUX = (1 << REFS0); // AVcc reference, start on ADC0
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // enable ADC, prescaler 128
}

unsigned short ADC_read(unsigned char channel) {
    channel &= 0x07;
    ADMUX = (ADMUX & 0xF0) | channel;
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC)) {}
    return ADC;
}



void drawBall(unsigned char x, unsigned char y, unsigned int color = LCD_WHITE) {
    LCD_fillRect(x, y, BALL_SIZE, BALL_SIZE, color);
}

void drawPaddle(unsigned char x, unsigned char y, unsigned int color = LCD_WHITE) {
    LCD_fillRect(x, y, PADDLE_WIDTH, PADDLE_HEIGHT, color);
}

void drawBricks() {
    for (unsigned char row = 0; row < BRICK_ROWS; row++) {
        for (unsigned char col = 0; col < BRICK_COLS; col++) {
            if (!bricks[row][col]) {
                continue;
            }

            unsigned char x = 105 - col * BRICK_X_SPACE;
            unsigned char y = 132 - row * BRICK_Y_SPACE;

            unsigned int color;

            if (row == 0) {
                color = LCD_RED;
            }
            else if (row == 1) {
                color = LCD_YELLOW;
            }
            else {
                color = LCD_GREEN;
            }

            LCD_fillRect(x, y, BRICK_WIDTH, BRICK_HEIGHT, color);
        }
    }
}

void gameInit() {
    paddleX = 48;
    paddleY = 10;
    lastPaddleX = paddleX;
    lastPaddleY = paddleY;

    ballX = 61;
    ballY = 65;
    lastBallX = ballX;
    lastBallY = ballY;

    brickBroken = false;
    brokenBrickRow = 0;
    brokenBrickCol = 0;
    score = 0;
    inputDirection = 0;

    for (unsigned char row = 0; row < BRICK_ROWS; row++) {
        for (unsigned char col = 0; col < BRICK_COLS; col++) {
            bricks[row][col] = true;
        }
    }

    drawBall(61, 65);
    drawPaddle(48, 10);
    drawBricks();
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
            if (!brickBroken &&
                (ballX == lastBallX && ballY == lastBallY) &&
                (paddleX == lastPaddleX && paddleY == lastPaddleY)) {
                break;
            }

            if (brickBroken) {
                unsigned char x = 105 - brokenBrickCol * BRICK_X_SPACE;
                unsigned char y = 132 - brokenBrickRow * BRICK_Y_SPACE;
                LCD_fillRect(x, y, BRICK_WIDTH, BRICK_HEIGHT, LCD_BLACK);
                brickBroken = false;
            }

            if (ballX != lastBallX || ballY != lastBallY) {
                drawBall(lastBallX, lastBallY, LCD_BLACK);
                drawBall(ballX, ballY);
                lastBallX = ballX;
                lastBallY = ballY;
            }

            if (paddleX != lastPaddleX || paddleY != lastPaddleY) {
                drawPaddle(lastPaddleX, lastPaddleY, LCD_BLACK);
                drawPaddle(paddleX, paddleY);
                lastPaddleX = paddleX;
                lastPaddleY = paddleY;
            }

            break;
    }

    return state;
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
                inputDirection = -1;
            }
            else if (joystickX > JOYSTICK_RIGHT_THRESHOLD) {
                inputDirection = 1;
            }
            else {
                inputDirection = 0;
            }
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
            if (inputDirection < 0) {
                if (paddleX >= PADDLE_SPEED) {
                    paddleX -= PADDLE_SPEED;
                }
                else {
                    paddleX = 0;
                }
            }
            else if (inputDirection > 0) {
                if (paddleX + PADDLE_WIDTH + PADDLE_SPEED <= LCD_WIDTH) {
                    paddleX += PADDLE_SPEED;
                }
                else {
                    paddleX = LCD_WIDTH - PADDLE_WIDTH;
                }
            }
            break;
    }

    return state;
}