#include <graphics.h>
#include <conio.h>
#include <windows.h>

int main()
{
    constexpr int W = 800;
    constexpr int H = 600;

    initgraph(W, H);
    setbkcolor(WHITE);
    cleardevice();

    // ===== Ball properties =====
    constexpr int radius = 30;
    float x = W / 2.0f;      // start center
    float y = H / 2.0f;      // start center
    float vx = 5.0f;          // horizontal velocity (moving right)
    float vy = 4.0f;          // vertical velocity (moving down)

    BeginBatchDraw();

    while (true)
    {
        // ---- Move the ball ----
        x = x + vx;
        y = y + vy;

        // ---- Rebound off LEFT wall ----
        if (x - radius <= 0)
        {
            x = radius;          // snap to wall
            vx = -vx;             // reverse horizontal direction
        }

        // ---- Rebound off RIGHT wall ----
        if (x + radius >= W)
        {
            x = W - radius;
            vx = -vx;
        }

        // ---- Rebound off TOP wall ----
        if (y - radius <= 0)
        {
            y = radius;
            vy = -vy;
        }

        // ---- Rebound off BOTTOM wall ----
        if (y + radius >= H)
        {
            y = H - radius;
            vy = -vy;
        }

        // ---- Draw ----
        cleardevice();

        // Walls (optional visual)
        setlinecolor(RGB(100, 100, 100));
        rectangle(0, 0, W - 1, H - 1);

        // Ball
        setfillcolor(RGB(220, 40, 40));
        setlinecolor(RGB(150, 0, 0));
        fillcircle((int)x, (int)y, radius);

        FlushBatchDraw();
        Sleep(10);
    }

    EndBatchDraw();
    _getch();
    closegraph();
    return 0;
}