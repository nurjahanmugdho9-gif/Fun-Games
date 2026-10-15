#include <graphics.h>
#include <conio.h>
#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <tchar.h>
#include <stdio.h>

int main()
{
    // ===== Window =====
    constexpr int W = 800;
    constexpr int H = 600;

    initgraph(W, H);
    srand((unsigned)time(0));

    // ===== Player (ball) =====
    constexpr int playerRadius = 25;
    constexpr int groundY = H - 50;
    float playerX = 100;
    float playerY = groundY - playerRadius;
    float playerVY = 0;

    // ===== Jump / gravity =====
    constexpr float gravity = 0.8f;
    constexpr float jumpPower = 8.0f;
    constexpr float holdBoost = 0.8f;
    constexpr float maxJumpHeight = 250.0f;   // how high the ball can go (in pixels)

    // ===== Pillar =====
    float pillarX = W;
    int pillarWidth = 40;
    int pillarHeight = 60 + rand() % 100;
    float pillarSpeed = 5.0f;

    // ===== Score =====
    int score = 0;
    bool passedPillar = false;

    BeginBatchDraw();

    while (1)
    {
        // ===== Exit on ESC =====
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
            break;

        // ===== Variable-height jump (capped at max height) =====
        float highestAllowedY = (groundY - playerRadius) - maxJumpHeight;

        if ((GetAsyncKeyState(VK_SPACE) & 0x8000) && playerVY == 0)
        {
            playerVY = -jumpPower;
        }

        if ((GetAsyncKeyState(VK_SPACE) & 0x8000)
            && playerVY < 0
            && playerY > highestAllowedY)
        {
            playerVY = playerVY - holdBoost;
        }

        // ===== Gravity =====
        playerVY = playerVY + gravity;
        playerY = playerY + playerVY;

        // ===== Ground collision =====
        if (playerY + playerRadius >= groundY)
        {
            playerY = groundY - playerRadius;
            playerVY = 0;
        }

        // ===== Ceiling limit =====
        if (playerY - playerRadius < 0)
        {
            playerY = playerRadius;
            playerVY = 0;
        }

        // ===== Move pillar =====
        pillarX = pillarX - pillarSpeed;

        // ===== Reset pillar when off-screen =====
        if (pillarX + pillarWidth < 0)
        {
            pillarX = W;
            pillarHeight = 60 + rand() % 100;
            passedPillar = false;
        }

        // ===== Score + gradual speed-up =====
        if (!passedPillar && pillarX + pillarWidth < playerX - playerRadius)
        {
            score++;
            passedPillar = true;

            if (pillarSpeed < 15.0f)
                pillarSpeed += 0.2f;
        }

        // ===== Collision detection =====
        bool collision =
            (playerX + playerRadius > pillarX) &&
            (playerX - playerRadius < pillarX + pillarWidth) &&
            (playerY + playerRadius > groundY - pillarHeight);

        if (collision)
        {
            // Game over screen
            cleardevice();

            // Full-screen dark overlay
            setfillcolor(RGB(0, 0, 0));
            solidrectangle(0, 0, W, H);

            settextcolor(RGB(255, 80, 80));
            settextstyle(60, 0, _T("Arial"));
            outtextxy(180, 200, _T("GAME OVER"));

            settextcolor(WHITE);
            settextstyle(40, 0, _T("Arial"));
            TCHAR buf[64];
            _stprintf_s(buf, _T("Final Score: %d"), score);
            outtextxy(240, 300, buf);

            settextstyle(25, 0, _T("Arial"));
            outtextxy(260, 370, _T("Press ESC to exit"));

            FlushBatchDraw();
            Sleep(2500);

            // Reset game
            score = 0;
            passedPillar = false;
            pillarX = W;
            pillarHeight = 60 + rand() % 100;
            pillarSpeed = 5.0f;
            playerY = groundY - playerRadius;
            playerVY = 0;
        }

        // ===== Draw everything =====
        cleardevice();

        // Sky
        setfillcolor(RGB(180, 220, 255));
        solidrectangle(0, 0, W, groundY);

        // Ground
        setfillcolor(RGB(100, 180, 100));
        solidrectangle(0, groundY, W, H);

        // Pillar
        setfillcolor(RGB(120, 80, 40));
        setlinecolor(RGB(60, 40, 20));
        solidrectangle((int)pillarX, groundY - pillarHeight,
            (int)(pillarX + pillarWidth), groundY);

        // Player
        setfillcolor(RGB(220, 40, 40));
        setlinecolor(RGB(150, 0, 0));
        fillcircle((int)playerX, (int)playerY, playerRadius);

        // ==========================================
        // ===== SCORE PANEL (top-left) =============
        // ==========================================
        setfillcolor(WHITE);
        setlinecolor(RGB(0, 0, 0));
        setlinestyle(PS_SOLID, 3);
        solidrectangle(15, 15, 200, 70);

        settextcolor(RGB(0, 0, 0));
        settextstyle(32, 0, _T("Arial"));
        setbkmode(TRANSPARENT);

        TCHAR scoreBuf[64];
        _stprintf_s(scoreBuf, _T("Score: %d"), score);
        outtextxy(25, 28, scoreBuf);

        // ==========================================
        // ===== SPEED PANEL (top-right) ============
        // ==========================================
        setfillcolor(WHITE);
        setlinecolor(RGB(0, 0, 0));
        solidrectangle(W - 220, 15, W - 15, 70);

        settextcolor(RGB(0, 0, 0));
        settextstyle(28, 0, _T("Arial"));

        TCHAR speedBuf[64];
        _stprintf_s(speedBuf, _T("Speed: %.1f"), pillarSpeed);
        outtextxy(W - 210, 30, speedBuf);

        FlushBatchDraw();
        Sleep(15);
    }

    EndBatchDraw();
    _getch();
    closegraph();
    return 0;
}