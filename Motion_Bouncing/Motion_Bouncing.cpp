#include <graphics.h>
#include <conio.h>
#include <windows.h>

int main()
{
    // ===== Window setup =====
    constexpr int W = 800;
    constexpr int H = 600;

    initgraph(W, H);
    setbkcolor(WHITE);
    cleardevice();

    // ===== Ball properties =====
    constexpr int radius = 30;
    float x = W / 2.0f;      // ball X position (center)
    float y = 100.0f;        // ball Y position (starts near top)
    float vy = 0.0f;          // vertical velocity

    // ===== Physics constants =====
    float gravity = 0.5f;     // how fast velocity increases each frame
    float decay = 0.75f;    // energy retained after each bounce (25% lost)

    BeginBatchDraw();          // Start double-buffering (prevents flicker)

    while (true)
    {
        // ---- 1. Apply gravity ----
        vy = vy + gravity;

        // ---- 2. Move the ball ----
        y = y + vy;

        // ---- 3. Bounce check ----
        if (y + radius >= H)      // Ball touches ground
        {
            y = H - radius;       // Snap ball to ground
            vy = -vy * decay;     // Reverse velocity + lose energy

            // Stop the ball if bounce is tiny
            if (vy > -0.5f && vy < 0.5f)
                vy = 0;
        }

        // ---- 4. Draw everything ----
        cleardevice();

        // Ground line
        setlinecolor(RGB(100, 100, 100));
        line(0, H - 1, W, H - 1);

        // The ball
        setfillcolor(RGB(220, 40, 40));
        setlinecolor(RGB(150, 0, 0));
        fillcircle((int)x, (int)y, radius);

        FlushBatchDraw();         // Push frame to screen
        Sleep(10);                // ~60 FPS
    }

    EndBatchDraw();
    _getch();
    closegraph();
    return 0;
}