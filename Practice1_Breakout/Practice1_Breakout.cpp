#include <graphics.h>
#include <conio.h>
#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <tchar.h>
#include <stdio.h>

// =====================================================================
// Practice-1 : Breakout
// Controls:
//   LEFT / RIGHT  -> move the board
//   SPACE         -> start the game
//   ESC           -> quit
// =====================================================================

int main()
{
    // ===== Window =====
    constexpr int W = 600;
    constexpr int H = 800;

    initgraph(W, H);
    srand((unsigned)time(0));

    // ===== Bricks =====
    constexpr int rows = 5;
    constexpr int cols = 6;
    constexpr int brickW = 80;
    constexpr int brickH = 30;
    constexpr int gap = 8;
    constexpr int brickOffsetX = (W - (cols * brickW + (cols - 1) * gap)) / 2;
    constexpr int brickOffsetY = 60;

    int bricks[rows][cols];
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            bricks[i][j] = 1;       // 1 = alive, 0 = destroyed

    // ===== Board (paddle) =====
    constexpr int boardW = 100;
    constexpr int boardH = 15;
    constexpr int boardY = H - 40;
    float boardX = W / 2.0f - boardW / 2.0f;
    constexpr float boardSpeed = 8.0f;

    // ===== Ball =====
    float ballX = W / 2.0f;
    float ballY = boardY - 15.0f;
    constexpr float ballR = 10.0f;
    float ballVX = 0.0f;
    float ballVY = 0.0f;
    constexpr float ballSpeed = 5.0f;

    // ===== Game state =====
    bool gameStarted = false;
    bool gameOver = false;
    bool win = false;
    int score = 0;

    BeginBatchDraw();

    while (1)
    {
        // ===== ESC to quit =====
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
            break;

        // ===== Keyboard: start game with SPACE =====
        if ((GetAsyncKeyState(VK_SPACE) & 0x8000) && !gameStarted && !gameOver)
        {
            gameStarted = true;

            // Launch the ball upward with a slight angle
            ballVX = ballSpeed * 0.6f;
            ballVY = -ballSpeed;
        }

        // ===== Keyboard: move board =====
        if (GetAsyncKeyState(VK_LEFT) & 0x8000)
        {
            boardX -= boardSpeed;
            if (boardX < 0) boardX = 0;
        }
        if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
        {
            boardX += boardSpeed;
            if (boardX + boardW > W) boardX = W - boardW;
        }

        // ===== Ball follows board before game starts =====
        if (!gameStarted)
        {
            ballX = boardX + boardW / 2.0f;
            ballY = boardY - ballR - 2.0f;
        }

        // ===== Update ball physics =====
        if (gameStarted && !gameOver)
        {
            ballX += ballVX;
            ballY += ballVY;

            // Wall collision (left / right)
            if (ballX - ballR < 0)
            {
                ballX = ballR;
                ballVX = -ballVX;
            }
            if (ballX + ballR > W)
            {
                ballX = W - ballR;
                ballVX = -ballVX;
            }

            // Ceiling collision
            if (ballY - ballR < 0)
            {
                ballY = ballR;
                ballVY = -ballVY;
            }

            // ===== Board collision =====
            if (ballY + ballR >= boardY &&
                ballY + ballR <= boardY + boardH + 8 &&
                ballX >= boardX &&
                ballX <= boardX + boardW &&
                ballVY > 0)
            {
                // Reflect upward + nudge horizontally based on hit position
                ballVY = -ballVY;
                float hit = (ballX - (boardX + boardW / 2.0f)) / (boardW / 2.0f);
                ballVX += hit * 2.0f;

                // Cap horizontal speed
                if (ballVX > 6.0f) ballVX = 6.0f;
                if (ballVX < -6.0f) ballVX = -6.0f;

                ballY = boardY - ballR;
            }

            // ===== Brick collision =====
            for (int i = 0; i < rows; ++i)
            {
                for (int j = 0; j < cols; ++j)
                {
                    if (!bricks[i][j]) continue;

                    int bx = brickOffsetX + j * (brickW + gap);
                    int by = brickOffsetY + i * (brickH + gap);

                    // Circle-vs-rectangle (clamp ball position to brick)
                    float closestX = ballX < bx ? (float)bx
                        : (ballX > bx + brickW ? (float)(bx + brickW) : ballX);
                    float closestY = ballY < by ? (float)by
                        : (ballY > by + brickH ? (float)(by + brickH) : ballY);

                    float dx = ballX - closestX;
                    float dy = ballY - closestY;

                    if (dx * dx + dy * dy <= ballR * ballR)
                    {
                        bricks[i][j] = 0;
                        score++;

                        // Decide bounce direction: reverse vy if hit top/bottom, else vx
                        if (ballY < by || ballY > by + brickH)
                            ballVY = -ballVY;
                        else
                            ballVX = -ballVX;

                        break;   // only one brick per frame
                    }
                }
            }

            // ===== Win condition =====
            if (score >= rows * cols)
            {
                win = true;
                gameOver = true;
            }

            // ===== Ball fell off the bottom =====
            if (ballY - ballR > H)
            {
                gameOver = true;
            }
        }

        // ===== Draw =====
        cleardevice();

        // Background
        setfillcolor(RGB(20, 20, 40));
        solidrectangle(0, 0, W, H);

        // ---- Draw bricks ----
        for (int i = 0; i < rows; ++i)
        {
            for (int j = 0; j < cols; ++j)
            {
                if (!bricks[i][j]) continue;

                int bx = brickOffsetX + j * (brickW + gap);
                int by = brickOffsetY + i * (brickH + gap);

                // Row-based colors for a nicer look
                switch (i)
                {
                case 0: setfillcolor(RGB(230, 80, 80));  break;   // red
                case 1: setfillcolor(RGB(230, 160, 60)); break;   // orange
                case 2: setfillcolor(RGB(230, 220, 60)); break;   // yellow
                case 3: setfillcolor(RGB(80, 200, 100)); break;   // green
                default: setfillcolor(RGB(80, 160, 220)); break;  // blue
                }
                setlinecolor(RGB(255, 255, 255));
                setlinestyle(PS_SOLID, 2);
                solidrectangle(bx, by, bx + brickW, by + brickH);
            }
        }

        // ---- Draw board ----
        setfillcolor(RGB(80, 80, 200));
        setlinecolor(RGB(200, 200, 255));
        setlinestyle(PS_SOLID, 2);
        solidrectangle((int)boardX, boardY,
            (int)(boardX + boardW), boardY + boardH);

        // ---- Draw ball ----
        setfillcolor(RGB(255, 220, 0));
        setlinecolor(RGB(255, 255, 255));
        fillcircle((int)ballX, (int)ballY, (int)ballR);

        // ---- HUD ----
        settextcolor(WHITE);
        settextstyle(24, 0, _T("Arial"));
        setbkmode(TRANSPARENT);
        TCHAR buf[64];
        _stprintf_s(buf, _T("Score: %d / %d"), score, rows * cols);
        outtextxy(15, 15, buf);

        // ---- Messages ----
        if (!gameStarted && !gameOver)
        {
            settextcolor(RGB(255, 255, 100));
            settextstyle(28, 0, _T("Arial"));
            outtextxy(150, H / 2, _T("Press SPACE to start"));
            settextstyle(18, 0, _T("Arial"));
            outtextxy(140, H / 2 + 40, _T("Move with LEFT / RIGHT arrows"));
        }

        if (win)
        {
            setfillcolor(RGB(0, 0, 0));
            solidrectangle(100, H / 2 - 60, W - 100, H / 2 + 80);
            settextcolor(RGB(80, 255, 80));
            settextstyle(50, 0, _T("Arial"));
            outtextxy(140, H / 2 - 40, _T("YOU WIN!"));
            settextstyle(22, 0, _T("Arial"));
            outtextxy(180, H / 2 + 30, _T("Press ESC to exit"));
        }

        if (gameOver && !win)
        {
            setfillcolor(RGB(0, 0, 0));
            solidrectangle(100, H / 2 - 60, W - 100, H / 2 + 80);
            settextcolor(RGB(255, 80, 80));
            settextstyle(50, 0, _T("Arial"));
            outtextxy(120, H / 2 - 40, _T("GAME OVER"));
            settextstyle(22, 0, _T("Arial"));
            settextcolor(WHITE);
            TCHAR mbuf[64];
            _stprintf_s(mbuf, _T("Final Score: %d"), score);
            outtextxy(200, H / 2 + 20, mbuf);
            settextstyle(18, 0, _T("Arial"));
            outtextxy(180, H / 2 + 60, _T("Press ESC to exit"));
        }

        FlushBatchDraw();
        Sleep(10);
    }

    EndBatchDraw();
    _getch();
    closegraph();
    return 0;
}#include <graphics.h>
#include <conio.h>
#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <tchar.h>
#include <stdio.h>

// =====================================================================
// Practice-1 : Breakout
// Controls:
//   LEFT / RIGHT  -> move the board
//   SPACE         -> start the game
//   ESC           -> quit
// =====================================================================

int main()
{
    // ===== Window =====
    constexpr int W = 600;
    constexpr int H = 800;

    initgraph(W, H);
    srand((unsigned)time(0));

    // ===== Bricks =====
    constexpr int rows = 5;
    constexpr int cols = 6;
    constexpr int brickW = 80;
    constexpr int brickH = 30;
    constexpr int gap = 8;
    constexpr int brickOffsetX = (W - (cols * brickW + (cols - 1) * gap)) / 2;
    constexpr int brickOffsetY = 60;

    int bricks[rows][cols];
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            bricks[i][j] = 1;       // 1 = alive, 0 = destroyed

    // ===== Board (paddle) =====
    constexpr int boardW = 100;
    constexpr int boardH = 15;
    constexpr int boardY = H - 40;
    float boardX = W / 2.0f - boardW / 2.0f;
    constexpr float boardSpeed = 8.0f;

    // ===== Ball =====
    float ballX = W / 2.0f;
    float ballY = boardY - 15.0f;
    constexpr float ballR = 10.0f;
    float ballVX = 0.0f;
    float ballVY = 0.0f;
    constexpr float ballSpeed = 5.0f;

    // ===== Game state =====
    bool gameStarted = false;
    bool gameOver = false;
    bool win = false;
    int score = 0;

    BeginBatchDraw();

    while (1)
    {
        // ===== ESC to quit =====
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
            break;

        // ===== Keyboard: start game with SPACE =====
        if ((GetAsyncKeyState(VK_SPACE) & 0x8000) && !gameStarted && !gameOver)
        {
            gameStarted = true;

            // Launch the ball upward with a slight angle
            ballVX = ballSpeed * 0.6f;
            ballVY = -ballSpeed;
        }

        // ===== Keyboard: move board =====
        if (GetAsyncKeyState(VK_LEFT) & 0x8000)
        {
            boardX -= boardSpeed;
            if (boardX < 0) boardX = 0;
        }
        if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
        {
            boardX += boardSpeed;
            if (boardX + boardW > W) boardX = W - boardW;
        }

        // ===== Ball follows board before game starts =====
        if (!gameStarted)
        {
            ballX = boardX + boardW / 2.0f;
            ballY = boardY - ballR - 2.0f;
        }

        // ===== Update ball physics =====
        if (gameStarted && !gameOver)
        {
            ballX += ballVX;
            ballY += ballVY;

            // Wall collision (left / right)
            if (ballX - ballR < 0)
            {
                ballX = ballR;
                ballVX = -ballVX;
            }
            if (ballX + ballR > W)
            {
                ballX = W - ballR;
                ballVX = -ballVX;
            }

            // Ceiling collision
            if (ballY - ballR < 0)
            {
                ballY = ballR;
                ballVY = -ballVY;
            }

            // ===== Board collision =====
            if (ballY + ballR >= boardY &&
                ballY + ballR <= boardY + boardH + 8 &&
                ballX >= boardX &&
                ballX <= boardX + boardW &&
                ballVY > 0)
            {
                // Reflect upward + nudge horizontally based on hit position
                ballVY = -ballVY;
                float hit = (ballX - (boardX + boardW / 2.0f)) / (boardW / 2.0f);
                ballVX += hit * 2.0f;

                // Cap horizontal speed
                if (ballVX > 6.0f) ballVX = 6.0f;
                if (ballVX < -6.0f) ballVX = -6.0f;

                ballY = boardY - ballR;
            }

            // ===== Brick collision =====
            for (int i = 0; i < rows; ++i)
            {
                for (int j = 0; j < cols; ++j)
                {
                    if (!bricks[i][j]) continue;

                    int bx = brickOffsetX + j * (brickW + gap);
                    int by = brickOffsetY + i * (brickH + gap);

                    // Circle-vs-rectangle (clamp ball position to brick)
                    float closestX = ballX < bx ? (float)bx
                        : (ballX > bx + brickW ? (float)(bx + brickW) : ballX);
                    float closestY = ballY < by ? (float)by
                        : (ballY > by + brickH ? (float)(by + brickH) : ballY);

                    float dx = ballX - closestX;
                    float dy = ballY - closestY;

                    if (dx * dx + dy * dy <= ballR * ballR)
                    {
                        bricks[i][j] = 0;
                        score++;

                        // Decide bounce direction: reverse vy if hit top/bottom, else vx
                        if (ballY < by || ballY > by + brickH)
                            ballVY = -ballVY;
                        else
                            ballVX = -ballVX;

                        break;   // only one brick per frame
                    }
                }
            }

            // ===== Win condition =====
            if (score >= rows * cols)
            {
                win = true;
                gameOver = true;
            }

            // ===== Ball fell off the bottom =====
            if (ballY - ballR > H)
            {
                gameOver = true;
            }
        }

        // ===== Draw =====
        cleardevice();

        // Background
        setfillcolor(RGB(20, 20, 40));
        solidrectangle(0, 0, W, H);

        // ---- Draw bricks ----
        for (int i = 0; i < rows; ++i)
        {
            for (int j = 0; j < cols; ++j)
            {
                if (!bricks[i][j]) continue;

                int bx = brickOffsetX + j * (brickW + gap);
                int by = brickOffsetY + i * (brickH + gap);

                // Row-based colors for a nicer look
                switch (i)
                {
                case 0: setfillcolor(RGB(230, 80, 80));  break;   // red
                case 1: setfillcolor(RGB(230, 160, 60)); break;   // orange
                case 2: setfillcolor(RGB(230, 220, 60)); break;   // yellow
                case 3: setfillcolor(RGB(80, 200, 100)); break;   // green
                default: setfillcolor(RGB(80, 160, 220)); break;  // blue
                }
                setlinecolor(RGB(255, 255, 255));
                setlinestyle(PS_SOLID, 2);
                solidrectangle(bx, by, bx + brickW, by + brickH);
            }
        }

        // ---- Draw board ----
        setfillcolor(RGB(80, 80, 200));
        setlinecolor(RGB(200, 200, 255));
        setlinestyle(PS_SOLID, 2);
        solidrectangle((int)boardX, boardY,
            (int)(boardX + boardW), boardY + boardH);

        // ---- Draw ball ----
        setfillcolor(RGB(255, 220, 0));
        setlinecolor(RGB(255, 255, 255));
        fillcircle((int)ballX, (int)ballY, (int)ballR);

        // ---- HUD ----
        settextcolor(WHITE);
        settextstyle(24, 0, _T("Arial"));
        setbkmode(TRANSPARENT);
        TCHAR buf[64];
        _stprintf_s(buf, _T("Score: %d / %d"), score, rows * cols);
        outtextxy(15, 15, buf);

        // ---- Messages ----
        if (!gameStarted && !gameOver)
        {
            settextcolor(RGB(255, 255, 100));
            settextstyle(28, 0, _T("Arial"));
            outtextxy(150, H / 2, _T("Press SPACE to start"));
            settextstyle(18, 0, _T("Arial"));
            outtextxy(140, H / 2 + 40, _T("Move with LEFT / RIGHT arrows"));
        }

        if (win)
        {
            setfillcolor(RGB(0, 0, 0));
            solidrectangle(100, H / 2 - 60, W - 100, H / 2 + 80);
            settextcolor(RGB(80, 255, 80));
            settextstyle(50, 0, _T("Arial"));
            outtextxy(140, H / 2 - 40, _T("YOU WIN!"));
            settextstyle(22, 0, _T("Arial"));
            outtextxy(180, H / 2 + 30, _T("Press ESC to exit"));
        }

        if (gameOver && !win)
        {
            setfillcolor(RGB(0, 0, 0));
            solidrectangle(100, H / 2 - 60, W - 100, H / 2 + 80);
            settextcolor(RGB(255, 80, 80));
            settextstyle(50, 0, _T("Arial"));
            outtextxy(120, H / 2 - 40, _T("GAME OVER"));
            settextstyle(22, 0, _T("Arial"));
            settextcolor(WHITE);
            TCHAR mbuf[64];
            _stprintf_s(mbuf, _T("Final Score: %d"), score);
            outtextxy(200, H / 2 + 20, mbuf);
            settextstyle(18, 0, _T("Arial"));
            outtextxy(180, H / 2 + 60, _T("Press ESC to exit"));
        }

        FlushBatchDraw();
        Sleep(10);
    }

    EndBatchDraw();
    _getch();
    closegraph();
    return 0;
}