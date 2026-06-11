#include "dvale065_game.h"

#include "dvale065_TFTLCD.h"
#include <stdlib.h>

#define PADDLE_WIDTH 32
#define PADDLE_HEIGHT 5
#define PADDLE_SPEED 2

#define BALL_SIZE 5 // ball is not round :(
#define BALL_X_SPEED 3
#define BALL_Y_SPEED 3

#define BRICK_WIDTH 18
#define BRICK_HEIGHT 8
#define BRICK_ROWS 3
#define BRICK_COLS 6 // do not change probably...

#define MAX_LIVES 3

#define GLYPH_COLS 3
#define GLYPH_ROWS 5

enum Game_Status { GAME_WAITING, GAME_PLAYING, GAME_WON, GAME_LOST };

static unsigned char paddleX, paddleY;
static unsigned char lastPaddleX, lastPaddleY;
static unsigned char ballX, ballY;
static unsigned char lastBallX, lastBallY;
static signed char ballDx, ballDy;

static bool bricks[BRICK_ROWS][BRICK_COLS];
static bool brickBroken;
static unsigned char brokenBrickRow, brokenBrickCol;
static unsigned char bricksRemaining;
static unsigned char score;
static unsigned char lives;
static unsigned char winMultiplier;

static bool scoreNeedsRedraw;
static bool livesNeedsRedraw;
static bool screenNeedsFullRedraw;
static Game_Status gameStatus;

static signed char inputDirection; // -1 = left, 0 = neutral, 1 = right

// hard coded glyphs for numbers and some letters, makes drawing easier !
static const unsigned char digitGlyphs[10][GLYPH_ROWS] = {
  {0b111, 0b101, 0b101, 0b101, 0b111}, // 0
  {0b010, 0b110, 0b010, 0b010, 0b111}, // 1
  {0b111, 0b001, 0b111, 0b100, 0b111}, // 2
  {0b111, 0b001, 0b111, 0b001, 0b111}, // 3
  {0b101, 0b101, 0b111, 0b001, 0b001}, // 4
  {0b111, 0b100, 0b111, 0b001, 0b111}, // 5
  {0b111, 0b100, 0b111, 0b101, 0b111}, // 6
  {0b111, 0b001, 0b001, 0b001, 0b001}, // 7
  {0b111, 0b101, 0b111, 0b101, 0b111}, // 8
  {0b111, 0b101, 0b111, 0b001, 0b111}, // 9
};

static const unsigned char glyphBlank[GLYPH_ROWS] = {0b000, 0b000, 0b000, 0b000, 0b000};
static const unsigned char glyphA[GLYPH_ROWS] = {0b111, 0b101, 0b111, 0b101, 0b101};
static const unsigned char glyphC[GLYPH_ROWS] = {0b111, 0b100, 0b100, 0b100, 0b111};
static const unsigned char glyphE[GLYPH_ROWS] = {0b111, 0b100, 0b111, 0b100, 0b111};
static const unsigned char glyphI[GLYPH_ROWS] = {0b111, 0b010, 0b010, 0b010, 0b111};
static const unsigned char glyphL[GLYPH_ROWS] = {0b100, 0b100, 0b100, 0b100, 0b111};
static const unsigned char glyphN[GLYPH_ROWS] = {0b101, 0b111, 0b111, 0b111, 0b101};
static const unsigned char glyphO[GLYPH_ROWS] = {0b111, 0b101, 0b101, 0b101, 0b111};
static const unsigned char glyphP[GLYPH_ROWS] = {0b111, 0b101, 0b111, 0b100, 0b100};
static const unsigned char glyphR[GLYPH_ROWS] = {0b110, 0b101, 0b110, 0b101, 0b101};
static const unsigned char glyphS[GLYPH_ROWS] = {0b111, 0b100, 0b111, 0b001, 0b111};
static const unsigned char glyphT[GLYPH_ROWS] = {0b111, 0b010, 0b010, 0b010, 0b010};
static const unsigned char glyphW[GLYPH_ROWS] = {0b101, 0b101, 0b101, 0b111, 0b101};
static const unsigned char glyphX[GLYPH_ROWS] = {0b101, 0b101, 0b010, 0b101, 0b101};

static const unsigned char *getGlyph(char character) {
  if (character >= '0' && character <= '9') {
    return digitGlyphs[character - '0'];
  }

  switch (character) {
    case 'A': return glyphA;
    case 'C': return glyphC;
    case 'E': return glyphE;
    case 'I': return glyphI;
    case 'L': return glyphL;
    case 'N': return glyphN;
    case 'O': return glyphO;
    case 'P': return glyphP;
    case 'R': return glyphR;
    case 'S': return glyphS;
    case 'T': return glyphT;
    case 'W': return glyphW;
    case 'X': return glyphX;
    default: return glyphBlank;
  }
}

static void drawGlyph(unsigned char x, unsigned char y, char character,
                      unsigned char scale, unsigned int color) {
  const unsigned char *glyph = getGlyph(character);

  for (unsigned char row = 0; row < GLYPH_ROWS; row++) {
    for (unsigned char col = 0; col < GLYPH_COLS; col++) {
      if (glyph[row] & (1 << (GLYPH_COLS - 1 - col))) {
        LCD_fillRect(x + col * scale, y + row * scale, scale, scale, color);
      }
    }
  }
}

static void drawText(unsigned char x, unsigned char y, const char *text,
                     unsigned char scale, unsigned int color) {
  unsigned char cursorX = x;

  for (unsigned char i = 0; text[i] != '\0'; i++) {
    drawGlyph(cursorX, y, text[i], scale, color);
    cursorX += (GLYPH_COLS + 1) * scale;
  }
}

static void drawTwoDigits(unsigned char x, unsigned char y, unsigned char value,
                          unsigned char scale, unsigned int color) {
  drawGlyph(x, y, (char)('0' + (value / 10)), scale, color);
  drawGlyph(x + (GLYPH_COLS + 1) * scale, y,
            (char)('0' + (value % 10)), scale, color);
}

static bool rectsOverlap(int x1, int y1, int width1, int height1,
                         int x2, int y2, int width2, int height2) {
  return (x1 < x2 + width2) &&
         (x1 + width1 > x2) &&
         (y1 < y2 + height2) &&
         (y1 + height1 > y2);
}

// redraw only some bricks !!!
static void redrawBricksInRect(unsigned char x, unsigned char y,
                               unsigned char width, unsigned char height) {
  for (unsigned char row = 0; row < BRICK_ROWS; row++) {
    for (unsigned char col = 0; col < BRICK_COLS; col++) {
      if (!bricks[row][col]) {
        continue;
      }

      unsigned char currentBrickX = 5 + col * 20;
      unsigned char currentBrickY = 26 + row * 12;
      unsigned int color = LCD_GREEN;

      if (row == 0) {
        color = LCD_RED;
      }
      else if (row == 1) {
        color = LCD_YELLOW;
      }

      if (rectsOverlap(x, y, width, height,
                       currentBrickX, currentBrickY, BRICK_WIDTH, BRICK_HEIGHT)) {
        LCD_fillRect(currentBrickX, currentBrickY,
                     BRICK_WIDTH, BRICK_HEIGHT, color);
      }
    }
  }
}

// can be buggy if not called in the right location
static void clearBallTrail(unsigned char oldX, unsigned char oldY,
                           unsigned char newX, unsigned char newY) {
  unsigned char left = (oldX < newX) ? oldX : newX;
  unsigned char top = (oldY < newY) ? oldY : newY;
  unsigned char right = ((oldX + BALL_SIZE) > (newX + BALL_SIZE)) ?
                        (oldX + BALL_SIZE) : (newX + BALL_SIZE);
  unsigned char bottom = ((oldY + BALL_SIZE) > (newY + BALL_SIZE)) ?
                         (oldY + BALL_SIZE) : (newY + BALL_SIZE);

  LCD_fillRect(left, top, right - left, bottom - top, LCD_BLACK);
  redrawBricksInRect(left, top, right - left, bottom - top);
}

static void drawScore() {
  LCD_fillRect(0, 0, 48, 14, LCD_BLACK);
  drawTwoDigits(14, 2, score, 2, LCD_YELLOW);
}

static void drawLives() {
  LCD_fillRect(80, 0, 48, 14, LCD_BLACK);

  for (unsigned char i = 0; i < MAX_LIVES; i++) {
    unsigned int color = (i < lives) ? LCD_CYAN : LCD_BLUE;
    LCD_fillRect(98 + i * 9, 4, 6, 6, color);
  }
}

static void drawGameScreen() {
  LCD_fillScreen(LCD_BLACK);
  drawScore();
  drawLives();
  LCD_fillRect(0, 14, LCD_WIDTH, 1, LCD_BLUE);

  for (unsigned char row = 0; row < BRICK_ROWS; row++) {
    for (unsigned char col = 0; col < BRICK_COLS; col++) {
      if (!bricks[row][col]) {
        continue;
      }

      unsigned char x = 5 + col * 20;
      unsigned char y = 26 + row * 12;
      unsigned int color = LCD_GREEN;

      if (row == 0) {
        color = LCD_RED;
      }
      else if (row == 1) {
        color = LCD_YELLOW;
      }

      LCD_fillRect(x, y, BRICK_WIDTH, BRICK_HEIGHT, color);
    }
  }

  LCD_fillRect(ballX, ballY, BALL_SIZE, BALL_SIZE, LCD_WHITE);
  LCD_fillRect(paddleX, paddleY, PADDLE_WIDTH, PADDLE_HEIGHT, LCD_WHITE);

  lastBallX = ballX;
  lastBallY = ballY;
  lastPaddleX = paddleX;
  lastPaddleY = paddleY;
}

static void drawStartScreen() {
  drawGameScreen();
  drawText(45, 76, "PRESS", 2, LCD_WHITE);
  drawText(45, 94, "START", 2, LCD_GREEN);
}

static void drawEndScreen() {
  LCD_fillScreen(LCD_BLACK);

  if (gameStatus == GAME_WON) {
    drawText(47, 48, "WIN", 3, LCD_GREEN);
    drawText(45, 108, "SCORE", 2, LCD_WHITE);
    drawTwoDigits(58, 126, score, 2, LCD_YELLOW);
  }
  else {
    drawText(41, 62, "LOSE", 3, LCD_RED);
    drawText(45, 98, "SCORE", 2, LCD_WHITE);
    drawTwoDigits(58, 116, score, 2, LCD_YELLOW);
  }
}

static void resetBallAndPaddle() {
  paddleX = 48;
  paddleY = 148;
  ballX = 61;
  ballY = 139;
  ballDx = BALL_X_SPEED;
  ballDy = -BALL_Y_SPEED;
}

static void resetGameData(Game_Status nextStatus) {
  resetBallAndPaddle();

  brickBroken = false;
  brokenBrickRow = 0;
  brokenBrickCol = 0;
  bricksRemaining = BRICK_ROWS * BRICK_COLS;
  score = 0;
  lives = MAX_LIVES;
  winMultiplier = 1;
  inputDirection = 0;
  gameStatus = nextStatus;

  for (unsigned char row = 0; row < BRICK_ROWS; row++) {
    for (unsigned char col = 0; col < BRICK_COLS; col++) {
      bricks[row][col] = true;
    }
  }

  scoreNeedsRedraw = true;
  livesNeedsRedraw = true;
  screenNeedsFullRedraw = true;
}

static void loseLife() {
  if (lives > 0) {
    lives--;
  }

  livesNeedsRedraw = true;

  if (lives == 0) {
    gameStatus = GAME_LOST;
    screenNeedsFullRedraw = true;
  }
  else {
    resetBallAndPaddle();
  }
}

static void updateBallDirection(int nextBallX) {
  unsigned char ballCenterX = (unsigned char)(nextBallX + (BALL_SIZE / 2));
  unsigned char leftThird = paddleX + (PADDLE_WIDTH / 3);
  unsigned char rightThird = paddleX + ((PADDLE_WIDTH * 2) / 3);

  if (ballCenterX < leftThird) {
    ballDx = -BALL_X_SPEED;
  }
  else if (ballCenterX > rightThird) {
    ballDx = BALL_X_SPEED;
  }
  else if (inputDirection < 0) {
    ballDx = -1;
  }
  else if (inputDirection > 0) {
    ballDx = 1;
  }

  ballDy = -BALL_Y_SPEED;
}

static void breakBrick(unsigned char row, unsigned char col, int nextBallX, int nextBallY) {
  bricks[row][col] = false;
  brickBroken = true;
  brokenBrickRow = row;
  brokenBrickCol = col;
  score++;
  scoreNeedsRedraw = true;

  if (bricksRemaining > 0) {
    bricksRemaining--;
  }

  unsigned char x = 5 + col * 20;
  unsigned char y = 26 + row * 12;

  if ((lastBallY + BALL_SIZE <= y) || (lastBallY >= y + BRICK_HEIGHT)) {
    ballDy = -ballDy;
  }
  else if ((lastBallX + BALL_SIZE <= x) || (lastBallX >= x + BRICK_WIDTH)) {
    ballDx = -ballDx;
  }
  else if (nextBallY < y || nextBallY > y + BRICK_HEIGHT) {
    ballDy = -ballDy;
  }
  else {
    ballDx = -ballDx;
  }

  if (bricksRemaining == 0) {
    winMultiplier = (lives > 0) ? lives : 1;
    score *= winMultiplier;
    scoreNeedsRedraw = true;
    gameStatus = GAME_WON;
    screenNeedsFullRedraw = true;
  }
}

void gameInit() {
  resetGameData(GAME_WAITING);
  drawStartScreen();
  screenNeedsFullRedraw = false;
  scoreNeedsRedraw = false;
  livesNeedsRedraw = false;
}

void setInputDirection(signed char direction) {
  inputDirection = direction;
}

void joystickPress() {
  if (gameStatus == GAME_WAITING || gameStatus == GAME_WON || gameStatus == GAME_LOST) {
    resetGameData(GAME_PLAYING);
  }
  else {
    resetGameData(GAME_WAITING);
  }
}

void paddleTick() {
  if (gameStatus != GAME_PLAYING) {
    return;
  }

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
}

void ballTick() {
  if (gameStatus != GAME_PLAYING) {
    return;
  }

  int nextX = ballX + ballDx;
  int nextY = ballY + ballDy;

  if (nextX <= 0) {
    nextX = 0;
    ballDx = abs(ballDx);
  }
  else if (nextX + BALL_SIZE >= LCD_WIDTH) {
    nextX = LCD_WIDTH - BALL_SIZE;
    ballDx = -abs(ballDx);
  }

  if (nextY <= 16) {
    nextY = 16;
    ballDy = abs(ballDy);
  }

  if (ballDy > 0 &&
      rectsOverlap(nextX, nextY, BALL_SIZE, BALL_SIZE,
                   paddleX, paddleY, PADDLE_WIDTH, PADDLE_HEIGHT)) {
    nextY = paddleY - BALL_SIZE;
    updateBallDirection(nextX);
  }
  else if (nextY + BALL_SIZE >= LCD_HEIGHT) {
    loseLife();
    return;
  }

  for (unsigned char row = 0; row < BRICK_ROWS; row++) {
    for (unsigned char col = 0; col < BRICK_COLS; col++) {
      if (!bricks[row][col]) {
        continue;
      }

      unsigned char x = 5 + col * 20;
      unsigned char y = 26 + row * 12;

      if (rectsOverlap(nextX, nextY, BALL_SIZE, BALL_SIZE,
                       x, y, BRICK_WIDTH, BRICK_HEIGHT)) {
        breakBrick(row, col, nextX, nextY);
        row = BRICK_ROWS;
        break;
      }
    }
  }

  if (gameStatus == GAME_PLAYING) {
    ballX = (unsigned char)nextX;
    ballY = (unsigned char)nextY;
  }
}

void tickDisplay() {
  if (screenNeedsFullRedraw) {
    if (gameStatus == GAME_WAITING) {
      drawStartScreen();
    }
    else if (gameStatus == GAME_PLAYING) {
      drawGameScreen();
    }
    else {
      drawEndScreen();
    }

    screenNeedsFullRedraw = false;
    brickBroken = false;
    scoreNeedsRedraw = false;
    livesNeedsRedraw = false;
    return;
  }

  if (gameStatus != GAME_PLAYING) {
    return;
  }

  if (!brickBroken &&
      !scoreNeedsRedraw &&
      !livesNeedsRedraw &&
      (ballX == lastBallX && ballY == lastBallY) &&
      (paddleX == lastPaddleX && paddleY == lastPaddleY)) {
    return;
  }

  bool ballMoved = (ballX != lastBallX || ballY != lastBallY);
  bool paddleMoved = (paddleX != lastPaddleX || paddleY != lastPaddleY);
  bool oldBallTouchedPaddle = rectsOverlap(lastBallX, lastBallY, BALL_SIZE, BALL_SIZE,
                                           paddleX, paddleY, PADDLE_WIDTH, PADDLE_HEIGHT);

  if (ballMoved) {
    clearBallTrail(lastBallX, lastBallY, ballX, ballY);
  }

  if (paddleMoved) {
    LCD_fillRect(lastPaddleX, lastPaddleY, PADDLE_WIDTH, PADDLE_HEIGHT, LCD_BLACK);
  }

  if (brickBroken) {
    LCD_fillRect(5 + brokenBrickCol * 20, 26 + brokenBrickRow * 12,
                 BRICK_WIDTH, BRICK_HEIGHT, LCD_BLACK);
    brickBroken = false;
  }

  if (scoreNeedsRedraw) {
    drawScore();
    scoreNeedsRedraw = false;
  }

  if (livesNeedsRedraw) {
    drawLives();
    livesNeedsRedraw = false;
  }

  if (paddleMoved || oldBallTouchedPaddle) {
    LCD_fillRect(paddleX, paddleY, PADDLE_WIDTH, PADDLE_HEIGHT, LCD_WHITE);
    lastPaddleX = paddleX;
    lastPaddleY = paddleY;
  }

  if (ballMoved) {
    LCD_fillRect(ballX, ballY, BALL_SIZE, BALL_SIZE, LCD_WHITE);
    lastBallX = ballX;
    lastBallY = ballY;
  }
}
