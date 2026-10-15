#include <graphics.h>
#include <conio.h>
#include <windows.h>

int main()
{
    initgraph(800, 600);
    setbkcolor(WHITE);
    cleardevice();

    int radius = 30;
    float x = 400;
    float y = 100;
    float vy = 0;
    float gravity = 0.5;
    float decay = 0.9;   // bounce back to 90% of height

    BeginBatchDraw();

    while (1)
    {
        vy = vy + gravity;      // accelerate (free fall)
        y = y + vy;             // move ball

        if (y + radius >= 600)  // hit ground
        {
            y = 600 - radius;
            vy = -vy * decay;   // bounce back with 90% energy

            if (vy > -0.5 && vy < 0.5)
                vy = 0;         // stop when bounce is tiny
        }

        cleardevice();
        setfillcolor(RED);
        fillcircle((int)x, (int)y, radius);
        FlushBatchDraw();
        Sleep(10);
    }

    EndBatchDraw();
    _getch();
    closegraph();
    return 0;
}