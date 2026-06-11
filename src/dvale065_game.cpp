#include "dvale065_game.h"

#include "dvale065_TFTLCD.h"

#define PADDLE_WIDTH 32
#define PADDLE_HEIGHT 5
#define PADDLE_START_X 48
#define PADDLE_Y 10

#define BALL_SIZE 5 // ball is drawn as a 5x5 square
#define BALL_START_X 61
#define BALL_START_Y 65
#define BALL_X_SPEED 2
#define BALL_Y_SPEED 2

#define BRICK_WIDTH 18
#define BRICK_HEIGHT 8
#define BRICK_X_SPACE 20
#define BRICK_Y_SPACE 20
#define BRICK_ROWS 3
#define BRICK_COLS 6 // do not change probably...
#define BRICK_TOTAL (BRICK_ROWS * BRICK_COLS)

#define PADDLE_SPEED 2
#define MAX_LIVES 3

#define HUD_Y 148
#define PLAYFIELD_TOP_Y HUD_Y
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

static bool scoreDirty;
static bool livesDirty;
static bool screenNeedsFullRedraw;
static Game_Status gameStatus;

static signed char inputDirection; // -1 = left, 0 = neutral, 1 = right

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
        default: return glyphBlank;
    }
}

static void drawGlyphUpsideDown(unsigned char x, unsigned char y, char character,
                                unsigned char scale, unsigned int color) {
    const unsigned char *glyph = getGlyph(character);

    for (unsigned char row = 0; row < GLYPH_ROWS; row++) {
        for (unsigned char col = 0; col < GLYPH_COLS; col++) {
            if (glyph[row] & (1 << (GLYPH_COLS - 1 - col))) {
                LCD_fillRect(x + (GLYPH_COLS - 1 - col) * scale,
                             y + (GLYPH_ROWS - 1 - row) * scale,
                             scale, scale, color);
            }
        }
    }
}

static unsigned char textLength(const char *text) {
    unsigned char length = 0;

    while (text[length] != '\0') {
        length++;
    }

    return length;
}

static void drawTextUpsideDown(unsigned char x, unsigned char y, const char *text,
                               unsigned char scale, unsigned int color) {
    unsigned char length = textLength(text);
    unsigned char cursorX = x;

    for (unsigned char i = 0; i < length; i++) {
        drawGlyphUpsideDown(cursorX, y, text[length - 1 - i], scale, color);
        cursorX += (GLYPH_COLS + 1) * scale;
    }
}

static void drawTwoDigitNumberUpsideDown(unsigned char x, unsigned char y, unsigned char value,
                                         unsigned char scale, unsigned int color) {
    drawGlyphUpsideDown(x, y, (char)('0' + (value % 10)), scale, color);
    drawGlyphUpsideDown(x + (GLYPH_COLS + 1) * scale, y,
                        (char)('0' + (value / 10)), scale, color);
}

static bool rectsOverlap(int x1, int y1, int width1, int height1,
                         int x2, int y2, int width2, int height2) {
    return (x1 < x2 + width2) &&
           (x1 + width1 > x2) &&
           (y1 < y2 + height2) &&
           (y1 + height1 > y2);
}

static signed char absSpeed(signed char value) {
    return (value < 0) ? -value : value;
}

static unsigned char brickX(unsigned char col) {
    return 105 - col * BRICK_X_SPACE;
}

static unsigned char brickY(unsigned char row) {
    return 132 - row * BRICK_Y_SPACE;
}

static unsigned int brickColor(unsigned char row) {
    if (row == 0) {
        return LCD_RED;
    }
    else if (row == 1) {
        return LCD_YELLOW;
    }
    else {
        return LCD_GREEN;
    }
}

static void drawBall(unsigned char x, unsigned char y, unsigned int color) {
    LCD_fillRect(x, y, BALL_SIZE, BALL_SIZE, color);
}

static void drawPaddle(unsigned char x, unsigned char y, unsigned int color) {
    LCD_fillRect(x, y, PADDLE_WIDTH, PADDLE_HEIGHT, color);
}

static void drawBricks() {
    for (unsigned char row = 0; row < BRICK_ROWS; row++) {
        for (unsigned char col = 0; col < BRICK_COLS; col++) {
            if (!bricks[row][col]) {
                continue;
            }

            LCD_fillRect(brickX(col), brickY(row), BRICK_WIDTH, BRICK_HEIGHT, brickColor(row));
        }
    }
}

static void redrawBricksInRect(unsigned char x, unsigned char y,
                               unsigned char width, unsigned char height) {
    for (unsigned char row = 0; row < BRICK_ROWS; row++) {
        for (unsigned char col = 0; col < BRICK_COLS; col++) {
            if (!bricks[row][col]) {
                continue;
            }

            unsigned char currentBrickX = brickX(col);
            unsigned char currentBrickY = brickY(row);

            if (rectsOverlap(x, y, width, height,
                             currentBrickX, currentBrickY, BRICK_WIDTH, BRICK_HEIGHT)) {
                LCD_fillRect(currentBrickX, currentBrickY,
                             BRICK_WIDTH, BRICK_HEIGHT, brickColor(row));
            }
        }
    }
}

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
    LCD_fillRect(0, HUD_Y, 42, 12, LCD_BLACK);
    drawGlyphUpsideDown(2, HUD_Y, 'S', 2, LCD_YELLOW);
    drawTwoDigitNumberUpsideDown(14, HUD_Y, score, 2, LCD_YELLOW);
}

static void drawLives() {
    LCD_fillRect(84, HUD_Y, 44, 12, LCD_BLACK);
    drawGlyphUpsideDown(86, HUD_Y, 'L', 2, LCD_CYAN);

    for (unsigned char i = 0; i < MAX_LIVES; i++) {
        unsigned int color = (i < lives) ? LCD_CYAN : LCD_BLUE;
        LCD_fillRect(100 + i * 9, HUD_Y + 3, 6, 6, color);
    }
}

static void drawHud() {
    drawScore();
    drawLives();
}

static void drawGameScreen() {
    LCD_fillScreen(LCD_BLACK);
    drawBricks();
    drawHud();
    drawBall(ballX, ballY, LCD_WHITE);
    drawPaddle(paddleX, paddleY, LCD_WHITE);

    lastBallX = ballX;
    lastBallY = ballY;
    lastPaddleX = paddleX;
    lastPaddleY = paddleY;
}

static void drawStartScreen() {
    LCD_fillScreen(LCD_BLACK);
    drawBricks();
    drawHud();
    drawBall(ballX, ballY, LCD_WHITE);
    drawPaddle(paddleX, paddleY, LCD_WHITE);
    drawTextUpsideDown(44, 50, "PRESS", 2, LCD_WHITE);
    drawTextUpsideDown(44, 72, "START", 2, LCD_MAGENTA);

    lastBallX = ballX;
    lastBallY = ballY;
    lastPaddleX = paddleX;
    lastPaddleY = paddleY;
}

static void drawEndScreen() {
    LCD_fillScreen(LCD_BLACK);

    if (gameStatus == GAME_WON) {
        drawTextUpsideDown(48, 72, "WIN", 3, LCD_GREEN);
    }
    else {
        drawTextUpsideDown(42, 72, "LOSE", 3, LCD_RED);
    }

    drawTextUpsideDown(36, 104, "SCORE", 2, LCD_WHITE);
    drawTwoDigitNumberUpsideDown(82, 104, score, 2, LCD_YELLOW);
}

static void resetBallAndPaddle() {
    paddleX = PADDLE_START_X;
    paddleY = PADDLE_Y;
    ballX = BALL_START_X;
    ballY = BALL_START_Y;
    ballDx = BALL_X_SPEED;
    ballDy = BALL_Y_SPEED;
}

static void resetGameData(Game_Status nextStatus) {
    resetBallAndPaddle();

    brickBroken = false;
    brokenBrickRow = 0;
    brokenBrickCol = 0;
    bricksRemaining = BRICK_TOTAL;
    score = 0;
    lives = MAX_LIVES;
    inputDirection = 0;
    gameStatus = nextStatus;

    for (unsigned char row = 0; row < BRICK_ROWS; row++) {
        for (unsigned char col = 0; col < BRICK_COLS; col++) {
            bricks[row][col] = true;
        }
    }

    scoreDirty = true;
    livesDirty = true;
    screenNeedsFullRedraw = true;
}

static void showStartScreen() {
    resetGameData(GAME_WAITING);
}

static void startGame() {
    resetGameData(GAME_PLAYING);
}

static void loseLife() {
    if (lives > 0) {
        lives--;
    }

    livesDirty = true;

    if (lives == 0) {
        gameStatus = GAME_LOST;
        screenNeedsFullRedraw = true;
    }
    else {
        resetBallAndPaddle();
    }
}

static void updateBallDirectionFromPaddle(int nextBallX) {
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

    ballDy = BALL_Y_SPEED;
}

static void breakBrick(unsigned char row, unsigned char col, int nextBallX, int nextBallY) {
    bricks[row][col] = false;
    brickBroken = true;
    brokenBrickRow = row;
    brokenBrickCol = col;
    score++;
    scoreDirty = true;

    if (bricksRemaining > 0) {
        bricksRemaining--;
    }

    unsigned char x = brickX(col);
    unsigned char y = brickY(row);

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
        gameStatus = GAME_WON;
        screenNeedsFullRedraw = true;
    }
}

void Breakout_Init() {
    showStartScreen();
    drawStartScreen();
    screenNeedsFullRedraw = false;
    scoreDirty = false;
    livesDirty = false;
}

void Breakout_SetInputDirection(signed char direction) {
    inputDirection = direction;
}

void Breakout_HandleSwitchPress() {
    if (gameStatus == GAME_WAITING || gameStatus == GAME_WON || gameStatus == GAME_LOST) {
        startGame();
    }
    else {
        showStartScreen();
    }
}

void Breakout_TickPaddle() {
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

void Breakout_TickBall() {
    if (gameStatus != GAME_PLAYING) {
        return;
    }

    int nextX = ballX + ballDx;
    int nextY = ballY + ballDy;

    if (nextX <= 0) {
        nextX = 0;
        ballDx = absSpeed(ballDx);
    }
    else if (nextX + BALL_SIZE >= LCD_WIDTH) {
        nextX = LCD_WIDTH - BALL_SIZE;
        ballDx = -absSpeed(ballDx);
    }

    if (nextY + BALL_SIZE >= PLAYFIELD_TOP_Y) {
        nextY = PLAYFIELD_TOP_Y - BALL_SIZE;
        ballDy = -absSpeed(ballDy);
    }

    if (ballDy < 0 &&
        rectsOverlap(nextX, nextY, BALL_SIZE, BALL_SIZE,
                     paddleX, paddleY, PADDLE_WIDTH, PADDLE_HEIGHT)) {
        nextY = paddleY + PADDLE_HEIGHT;
        updateBallDirectionFromPaddle(nextX);
    }
    else if (nextY <= 0) {
        loseLife();
        return;
    }

    for (unsigned char row = 0; row < BRICK_ROWS; row++) {
        for (unsigned char col = 0; col < BRICK_COLS; col++) {
            if (!bricks[row][col]) {
                continue;
            }

            unsigned char x = brickX(col);
            unsigned char y = brickY(row);

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

void Breakout_TickDisplay() {
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
        scoreDirty = false;
        livesDirty = false;
        return;
    }

    if (gameStatus != GAME_PLAYING) {
        return;
    }

    if (!brickBroken &&
        !scoreDirty &&
        !livesDirty &&
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
        drawPaddle(lastPaddleX, lastPaddleY, LCD_BLACK);
    }

    if (brickBroken) {
        LCD_fillRect(brickX(brokenBrickCol), brickY(brokenBrickRow),
                     BRICK_WIDTH, BRICK_HEIGHT, LCD_BLACK);
        brickBroken = false;
    }

    if (scoreDirty) {
        drawScore();
        scoreDirty = false;
    }

    if (livesDirty) {
        drawLives();
        livesDirty = false;
    }

    if (paddleMoved || oldBallTouchedPaddle) {
        drawPaddle(paddleX, paddleY, LCD_WHITE);
        lastPaddleX = paddleX;
        lastPaddleY = paddleY;
    }

    if (ballMoved) {
        drawBall(ballX, ballY, LCD_WHITE);
        lastBallX = ballX;
        lastBallY = ballY;
    }
}
