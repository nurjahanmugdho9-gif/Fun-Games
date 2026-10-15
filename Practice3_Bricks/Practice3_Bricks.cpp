// =====================================================================
// Practice-3 : Shooting the moving target ("the bricks")
// Project name to create in Visual Studio: Practice3_Bricks
// =====================================================================
// Controls:
//   SPACE -> fire the ball at whatever angle the tower currently has
//   ESC   -> quit
//
// This builds directly on the skeleton given on the slides:
//   - variables x,y,vx,vy,g,r are the ball's projectile-motion state
//     (same free-fall idea as "Motion: Free fall + bouncing").
//   - the moving red rectangle (p, vp) is the target, bouncing between
//     the top and bottom of the window.
//   - the rotating-tower code (const angle/rotateSpeed logic) is the
//     "advanced" feature: the barrel sweeps back and forth, and the
//     ball is fired along whatever direction it is pointing at the
//     moment you press space.
//
// New pieces the slide asked you to add on top of the skeleton:
//   1. Keyboard-controlled shooting   -> handled below (VK_SPACE)
//   2. Score calculation              -> `score` / `shots`
//   3. Collision detection            -> circle-vs-rectangle test
//   4. Random length and speed        -> target height & speed re-rolled
//      every time it's hit
// =====================================================================

#include <graphics.h>
#include <conio.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <tchar.h>

#define WIDTH  600
#define HEIGHT 800
#define PI 3.1415926f

int main()
{
    srand((unsigned)time(NULL));

    initgraph(WIDTH, HEIGHT);
    BeginBatchDraw();

    // ---------------- Tower (rotating cannon) ----------------
    float towerX = 30, towerY = HEIGHT / 2.0f;
    float towerLen = 40;
    float angle = 0;                 // current aim angle, in radians
    float rotateSpeed = PI / 90;     // ~2 degrees per frame
    float maxAngle = PI / 3;         // sweeps between 0 and 60 degrees

    // ---------------- Ball (projectile motion, same as the skeleton) ----------------
    float x = towerX, y = towerY;
    float vx = 0, vy = 0;
    float g = 1.0f;           // gravity, added to vy every frame
    int   r = 12;
    bool  f = false;          // f = "is the ball currently flying" (from the skeleton)
    float shotSpeed = 18;

    // ---------------- Moving target ("the brick") ----------------
    int targetX1 = WIDTH - 30, targetX2 = WIDTH - 10;   // fixed x-band, right edge
    int p = 60 + rand() % 60;                          // target height (random length)
    int targetY = rand() % (HEIGHT - p);
    int vp = 3 + rand() % 5;                             // target speed (random)

    int score = 0;
    int shots = 0;

    while (true)
    {
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) break;

        // ---- Sweep the tower angle automatically (advanced feature) ----
        angle += rotateSpeed;
        if (angle > maxAngle || angle < 0)
            rotateSpeed = -rotateSpeed;

        // ---- Fire on space, only if a ball isn't already in the air ----
        if ((GetAsyncKeyState(VK_SPACE) & 0x8000) && !f)
        {
            f = true;
            shots++;
            x = towerX;
            y = towerY;
            vx = shotSpeed * cos(angle);
            vy = -shotSpeed * sin(angle);   // screen y grows downward, so "up" is negative
        }

        // ---- Move the target, bounce off the top/bottom ----
        targetY += vp;
        if (targetY <= 0 || targetY + p >= HEIGHT)
            vp = -vp;

        // ---- Move the ball under gravity, exactly like the skeleton ----
        if (f)
        {
            vy = vy + g;
            x = x + vx;
            y = y + vy;

            // ---- Collision detection: circle (x,y,r) vs target rectangle ----
            float closestX = x < targetX1 ? (float)targetX1 : (x > targetX2 ? (float)targetX2 : x);
            float closestY = y < targetY ? (float)targetY : (y > targetY + p ? (float)(targetY + p) : y);
            float dx = x - closestX;
            float dy = y - closestY;

            if (dx * dx + dy * dy <= r * r)
            {
                score++;
                f = false;

                // re-roll the target's length and speed, as the slide asks
                p = 60 + rand() % 60;
                targetY = rand() % (HEIGHT - p);
                vp = (rand() % 2 == 0 ? 1 : -1) * (3 + rand() % 5);
            }
            // ball left the play field without hitting anything -> reset
            else if (x > WIDTH || x < 0 || y > HEIGHT)
            {
                f = false;
            }
        }

        // ---------------- Draw ----------------
        setbkcolor(RGB(30, 30, 30));
        cleardevice();

        // tower: base + barrel pointing at the current angle
        setfillcolor(RGB(100, 100, 255));
        fillcircle((int)towerX, (int)towerY, 15);
        float endX = towerX + towerLen * cos(angle);
        float endY = towerY - towerLen * sin(angle);
        setlinecolor(WHITE);
        setlinestyle(PS_SOLID, 4);
        line((int)towerX, (int)towerY, (int)endX, (int)endY);
        setlinestyle(PS_SOLID, 1);

        // target
        setfillcolor(RGB(230, 60, 60));
        solidrectangle(targetX1, targetY, targetX2, targetY + p);

        // ball
        if (f)
        {
            setfillcolor(RGB(255, 220, 0));
            fillcircle((int)x, (int)y, r);
        }

        // HUD
        settextcolor(WHITE);
        settextstyle(22, 0, _T("Consolas"));
        TCHAR buf[64];
        _stprintf_s(buf, _T("Score: %d   Shots: %d"), score, shots);
        outtextxy(15, 15, buf);

        settextstyle(14, 0, _T("Consolas"));
        outtextxy(15, HEIGHT - 25, _T("SPACE: fire   ESC: quit"));

        FlushBatchDraw();
        Sleep(15);
    }

    EndBatchDraw();
    closegraph();
    return 0;
}