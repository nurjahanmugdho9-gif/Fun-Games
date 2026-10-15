#include <graphics.h>
#include <conio.h>
#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <tchar.h>
#include <stdio.h>

int main()
{
    // ===== Screen =====
    constexpr int W = 600;
    constexpr int H = 600;
    initgraph(W, H);
    srand((unsigned)time(0));

    // ===== Game variables (reset each game) =====
    int r = 10;
    float x, y;
    float vx, vy;

    constexpr int rows = 5;
    constexpr int cols = 6;
    int w = 90, h = 40;
    int gap = 5;
    int bricks[cols][rows];

    constexpr int boardW = 100;
    constexpr int boardH = 15;
    constexpr int boardY = H - 30;
    float boardX;
    constexpr float boardSpeed = 8.0f;

    bool started = false;
    bool gameOver = false;
    bool win = false;
    int score = 0;
    constexpr int totalBricks = cols * rows;

    // ===== Ball speed constants (FASTER now) =====
    constexpr float ballSpeedX = 7.0f;    // was 5
    constexpr float ballSpeedY = -9.0f;   // was -7

    // ===== Reset function (used on start + restart) =====
    auto resetGame = [&]()
        {
            x = W / 2.0f;
            y = boardY - r - 2.0f;
            vx = ballSpeedX;
            vy = ballSpeedY;

            boardX = W / 2.0f - boardW / 2.0f;

            for (int i = 0; i < cols; ++i)
                for (int j = 0; j < rows; ++j)
                    bricks[i][j] = 1;

            started = false;
            gameOver = false;
            win = false;
            score = 0;
        };

    resetGame();   // initial setup

    BeginBatchDraw();

    while (1)
    {
        // ===== ESC to quit =====
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
            break;

        // ===== R to restart =====
        if ((GetAsyncKeyState('R') & 0x8000) && (gameOver || win))
        {
            resetGame();
            Sleep(150);   // debounce
        }

        // ===== SPACE to start =====
        if ((GetAsyncKeyState(VK_SPACE) & 0x8000) && !started && !gameOver && !win)
        {
            started = true;
        }

        // ===== LEFT / RIGHT paddle =====
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

        // ===== Before start: ball follows paddle =====
        if (!started && !gameOver && !win)
        {
            x = boardX + boardW / 2.0f;
            y = boardY - r - 2.0f;
        }

        // ===== Ball physics =====
        if (started && !gameOver && !win)
        {
            x += vx;
            y += vy;

            // Wall bounce
            if (x - r < 0) { x = r;         vx = -vx; }
            if (x + r > W) { x = W - r;     vx = -vx; }
            if (y - r < 0) { y = r;         vy = -vy; }

            // Paddle collision
            if (vy > 0 &&
                y + r >= boardY &&
                y + r <= boardY + boardH + 10 &&
                x >= boardX && x <= boardX + boardW)
            {
                vy = -vy;
                float hit = (x - (boardX + boardW / 2.0f)) / (boardW / 2.0f);
                vx += hit * 1.5f;
                if (vx > 8) vx = 8;
                if (vx < -8) vx = -8;
                y = boardY - r;
            }

            // Brick collision
            for (int i = 0; i < cols; ++i)
            {
                for (int j = 0; j < rows; ++j)
                {
                    if (!bricks[i][j]) continue;

                    int bx = gap * (i + 1) + i * w;
                    int by = gap * (j + 1) + j * h;
                    int bx2 = bx + w;
                    int by2 = by + h;

                    float cx = x < bx ? (float)bx : (x > bx2 ? (float)bx2 : x);
                    float cy = y < by ? (float)by : (y > by2 ? (float)by2 : y);
                    float dx = x - cx;
                    float dy = y - cy;

                    if (dx * dx + dy * dy <= (float)(r * r))
                    {
                        bricks[i][j] = 0;
                        score++;

                        if (y < by || y > by2) vy = -vy;
                        else                   vx = -vx;

                        goto brickDone;
                    }
                }
            }
        brickDone:

            if (score >= totalBricks) { win = true; gameOver = true; }
            if (y - r > H) { gameOver = true; }
        }

        // ===== Draw =====
        setbkcolor(RGB(200, 200, 200));
        cleardevice();

        // ---- Bricks ----
        setlinecolor(RGB(0, 128, 128));
        for (int i = 0; i < cols; ++i)
        {
            for (int j = 0; j < rows; ++j)
            {
                if (!bricks[i][j]) continue;

                int bx = gap * (i + 1) + i * w;
                int by = gap * (j + 1) + j * h;

                switch (j)
                {
                case 0: setfillcolor(RGB(200, 60, 60));  break;
                case 1: setfillcolor(RGB(200, 140, 60)); break;
                case 2: setfillcolor(RGB(200, 200, 60)); break;
                case 3: setfillcolor(RGB(60, 180, 90));  break;
                default: setfillcolor(RGB(0, 128, 128)); break;
                }
                fillrectangle(bx, by, bx + w, by + h);
            }
        }

        // ---- Paddle ----
        setfillcolor(RGB(0, 0, 128));
        setlinecolor(RGB(0, 0, 200));
        fillrectangle((int)boardX, boardY,
            (int)(boardX + boardW), boardY + boardH);

        // ---- Ball ----
        setfillcolor(RGB(128, 0, 0));
        setlinecolor(RGB(180, 0, 0));
        fillcircle((int)x, (int)y, r);

        // ---- HUD ----
        settextcolor(RGB(0, 0, 0));
        settextstyle(22, 0, _T("Arial"));
        setbkmode(TRANSPARENT);
        TCHAR buf[64];
        _stprintf_s(buf, _T("Score: %d / %d"), score, totalBricks);
        outtextxy(15, 10, buf);

        // ---- Messages ----
        if (!started && !gameOver && !win)
        {
            settextcolor(RGB(0, 0, 200));
            settextstyle(26, 0, _T("Arial"));
            outtextxy(120, 250, _T("Press SPACE to start"));
            settextstyle(16, 0, _T("Arial"));
            outtextxy(150, 290, _T("LEFT / RIGHT to move"));
        }

        if (win)
        {
            settextcolor(RGB(0, 150, 0));
            settextstyle(40, 0, _T("Arial"));
            outtextxy(180, 230, _T("YOU WIN!"));

            settextstyle(20, 0, _T("Arial"));
            outtextxy(200, 290, _T("Press R to restart"));
            outtextxy(200, 320, _T("Press ESC to exit"));
        }

        if (gameOver && !win)
        {
            settextcolor(RGB(200, 0, 0));
            settextstyle(40, 0, _T("Arial"));
            outtextxy(140, 220, _T("GAME OVER"));

            settextstyle(20, 0, _T("Arial"));
            TCHAR mbuf[64];
            _stprintf_s(mbuf, _T("Final Score: %d"), score);
            outtextxy(210, 280, mbuf);

            outtextxy(200, 320, _T("Press R to restart"));
            outtextxy(200, 350, _T("Press ESC to exit"));
        }

        FlushBatchDraw();
        Sleep(15);
    }

    EndBatchDraw();
    closegraph();
    return 0;
}