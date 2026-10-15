#include <graphics.h>
#include <conio.h>
#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <tchar.h>
#include <stdio.h>
#include <math.h>

// ---------- Helper: draw a proper arrow ----------
void drawArrow(float x, float y, float vx, float vy)
{
    float angle = atan2f(vy, vx);
    constexpr float length = 40.0f;

    float tipX = x + cosf(angle) * length;
    float tipY = y + sinf(angle) * length;

    setlinecolor(RGB(80, 50, 20));
    setlinestyle(PS_SOLID, 4);
    line((int)x, (int)y, (int)tipX, (int)tipY);

    constexpr float headLen = 20.0f;
    constexpr float headAngle = 0.45f;
    float lx = tipX - cosf(angle - headAngle) * headLen;
    float ly = tipY - sinf(angle - headAngle) * headLen;
    float rx = tipX - cosf(angle + headAngle) * headLen;
    float ry = tipY - sinf(angle + headAngle) * headLen;

    setfillcolor(RGB(120, 120, 120));
    setlinecolor(RGB(0, 0, 0));
    setlinestyle(PS_SOLID, 2);
    POINT head[3] = {
        { (int)tipX, (int)tipY },
        { (int)lx,   (int)ly   },
        { (int)rx,   (int)ry   }
    };
    fillpolygon(head, 3);
    polygon(head, 3);

    float backX = x, backY = y;
    constexpr float featherLen = 12.0f;
    float fx1 = backX - cosf(angle - 0.6f) * featherLen;
    float fy1 = backY - sinf(angle - 0.6f) * featherLen;
    float fx2 = backX - cosf(angle + 0.6f) * featherLen;
    float fy2 = backY - sinf(angle + 0.6f) * featherLen;

    setfillcolor(RGB(220, 60, 60));
    setlinecolor(RGB(150, 0, 0));
    POINT f1[3] = {
        { (int)backX, (int)backY },
        { (int)fx1,   (int)fy1   },
        { (int)(backX + cosf(angle) * 4), (int)(backY + sinf(angle) * 4) }
    };
    POINT f2[3] = {
        { (int)backX, (int)backY },
        { (int)fx2,   (int)fy2   },
        { (int)(backX + cosf(angle) * 4), (int)(backY + sinf(angle) * 4) }
    };
    fillpolygon(f1, 3);
    fillpolygon(f2, 3);
}

// ---------- Hit flash ----------
struct Flash { bool active; float x, y; int frames; };
constexpr int maxFlashes = 10;
Flash flashes[maxFlashes];

void spawnFlash(float x, float y)
{
    for (int i = 0; i < maxFlashes; ++i)
    {
        if (!flashes[i].active)
        {
            flashes[i].active = true;
            flashes[i].x = x;
            flashes[i].y = y;
            flashes[i].frames = 12;
            return;
        }
    }
}

int main()
{
    constexpr int W = 900;
    constexpr int H = 600;

    initgraph(W, H);
    srand((unsigned)time(0));
    for (int i = 0; i < maxFlashes; ++i) flashes[i].active = false;

    constexpr int launcherX = 80;
    constexpr int launcherY = H / 2;

    constexpr int barX = W - 120;
    constexpr int barY = 20;
    constexpr int barW = 100;
    constexpr int barH = 30;

    // ===== Balls =====
    constexpr int maxBalls = 10;
    struct Ball {
        bool active;
        float x, y;
        float vx, vy;
        float r;
    };
    Ball balls[maxBalls];
    for (int i = 0; i < maxBalls; ++i) balls[i].active = false;

    // Pre-spawn 3 balls at staggered heights (FIX 1)
    for (int i = 0; i < 3; ++i)
    {
        balls[i].active = true;
        balls[i].x = (float)(barX + barW / 2);
        balls[i].y = 60.0f + i * 90.0f;    // staggered
        balls[i].vx = 0.0f;
        balls[i].vy = 0.0f;
        balls[i].r = 16.0f;
    }

    int spawnTimer = 40;
    constexpr int spawnInterval = 45;
    constexpr float ballGravity = 0.25f;   // FIX 4: slower fall

    // ===== Arrows =====
    constexpr int maxArrows = 5;
    struct Arrow {
        bool active;
        float x, y;
        float vx, vy;
    };
    Arrow arrows[maxArrows];
    for (int i = 0; i < maxArrows; ++i) arrows[i].active = false;

    constexpr float arrowSpeed = 18.0f;   // slightly faster
    constexpr float arrowGravity = 0.04f;   // FIX 2: flatter
    int shootCooldown = 0;

    int score = 0;
    int best = 0;
    int misses = 0;

    BeginBatchDraw();

    while (1)
    {
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
            break;

        // ===== Shoot =====
        if (shootCooldown > 0) shootCooldown--;

        if ((GetAsyncKeyState(VK_SPACE) & 0x8000) && shootCooldown == 0)
        {
            for (int i = 0; i < maxArrows; ++i)
            {
                if (!arrows[i].active)
                {
                    arrows[i].active = true;
                    arrows[i].x = launcherX + 30.0f;
                    arrows[i].y = (float)launcherY;
                    arrows[i].vx = arrowSpeed;
                    arrows[i].vy = 0.0f;
                    shootCooldown = 8;
                    break;
                }
            }
        }

        // ===== Update arrows =====
        for (int i = 0; i < maxArrows; ++i)
        {
            if (!arrows[i].active) continue;

            arrows[i].vy += arrowGravity;
            arrows[i].x += arrows[i].vx;
            arrows[i].y += arrows[i].vy;

            if (arrows[i].x > W + 50 || arrows[i].y > H + 50)
            {
                arrows[i].active = false;
                misses++;
                continue;
            }

            for (int j = 0; j < maxBalls; ++j)
            {
                if (!balls[j].active) continue;

                float dx = arrows[i].x - balls[j].x;
                float dy = arrows[i].y - balls[j].y;
                float r = balls[j].r + 12;   // FIX 3: bigger hitbox
                if (dx * dx + dy * dy <= r * r)
                {
                    spawnFlash(balls[j].x, balls[j].y);
                    balls[j].active = false;
                    arrows[i].active = false;
                    score++;
                    if (score > best) best = score;
                    break;
                }
            }
        }

        // ===== Update flashes =====
        for (int i = 0; i < maxFlashes; ++i)
        {
            if (flashes[i].active)
            {
                flashes[i].frames--;
                if (flashes[i].frames <= 0) flashes[i].active = false;
            }
        }

        // ===== Spawn ball =====
        if (--spawnTimer <= 0)
        {
            for (int i = 0; i < maxBalls; ++i)
            {
                if (!balls[i].active)
                {
                    balls[i].active = true;
                    balls[i].x = (float)(barX + barW / 2);
                    balls[i].y = (float)(barY + barH);
                    balls[i].vx = 0.0f;
                    balls[i].vy = 0.0f;
                    balls[i].r = 16.0f;
                    break;
                }
            }
            spawnTimer = spawnInterval;
        }

        // ===== Update balls =====
        for (int i = 0; i < maxBalls; ++i)
        {
            if (!balls[i].active) continue;

            balls[i].vy += ballGravity;
            balls[i].x += balls[i].vx;
            balls[i].y += balls[i].vy;

            if (balls[i].y + balls[i].r >= H - 20)
            {
                balls[i].active = false;
                misses++;
            }
        }

        // ===== Draw =====
        cleardevice();

        setfillcolor(RGB(200, 230, 255));
        solidrectangle(0, 0, W, H);

        setfillcolor(RGB(110, 180, 100));
        solidrectangle(0, H - 20, W, H);

        setfillcolor(RGB(90, 90, 110));
        setlinecolor(RGB(50, 50, 70));
        solidrectangle(barX, barY, barX + barW, barY + barH);

        setfillcolor(RGB(30, 30, 40));
        solidrectangle(barX + 15, barY + barH - 5, barX + barW - 15, barY + barH + 5);

        // Balls
        for (int i = 0; i < maxBalls; ++i)
        {
            if (!balls[i].active) continue;

            setfillcolor(RGB(255, 90, 90));
            setlinecolor(RGB(160, 0, 0));
            fillcircle((int)balls[i].x, (int)balls[i].y, (int)balls[i].r);

            setfillcolor(RGB(255, 190, 190));
            setlinecolor(RGB(255, 190, 190));
            solidcircle((int)(balls[i].x - balls[i].r * 0.3f),
                (int)(balls[i].y - balls[i].r * 0.3f),
                (int)(balls[i].r * 0.3f));
        }

        // Hit flashes (yellow rings)
        for (int i = 0; i < maxFlashes; ++i)
        {
            if (!flashes[i].active) continue;
            int alpha = flashes[i].frames;
            setlinecolor(RGB(255, 255, 100));
            setlinestyle(PS_SOLID, 3);
            circle((int)flashes[i].x, (int)flashes[i].y, 25 + (12 - alpha));
            circle((int)flashes[i].x, (int)flashes[i].y, 15 + (12 - alpha));
        }

        // Launcher
        setfillcolor(RGB(70, 70, 90));
        setlinecolor(RGB(40, 40, 60));
        solidrectangle(launcherX - 25, launcherY - 30,
            launcherX + 25, launcherY + 30);

        setlinecolor(RGB(30, 30, 30));
        setlinestyle(PS_SOLID, 8);
        line(launcherX + 25, launcherY, launcherX + 55, launcherY);

        // Arrows
        for (int i = 0; i < maxArrows; ++i)
        {
            if (!arrows[i].active) continue;
            drawArrow(arrows[i].x, arrows[i].y, arrows[i].vx, arrows[i].vy);
        }

        // Score
        setfillcolor(WHITE);
        setlinecolor(RGB(0, 0, 0));
        setlinestyle(PS_SOLID, 3);
        solidrectangle(15, 15, 230, 70);

        settextcolor(RGB(0, 0, 0));
        settextstyle(30, 0, _T("Arial"));
        setbkmode(TRANSPARENT);
        TCHAR buf[64];
        _stprintf_s(buf, _T("Score: %d"), score);
        outtextxy(25, 28, buf);

        setfillcolor(WHITE);
        setlinecolor(RGB(0, 0, 0));
        solidrectangle(W / 2 - 140, 15, W / 2 + 140, 70);

        TCHAR buf2[64];
        _stprintf_s(buf2, _T("Best: %d   Miss: %d"), best, misses);
        outtextxy(W / 2 - 130, 28, buf2);

        settextcolor(RGB(60, 60, 60));
        settextstyle(16, 0, _T("Arial"));
        outtextxy(20, H - 22, _T("SPACE = shoot    ESC = exit"));

        FlushBatchDraw();
        Sleep(15);
    }

    EndBatchDraw();
    _getch();
    closegraph();
    return 0;
}