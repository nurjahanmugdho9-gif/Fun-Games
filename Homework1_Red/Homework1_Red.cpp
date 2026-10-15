#include <graphics.h>
#include <conio.h>
#include <windows.h>

int main()
{
    constexpr int windowWidth = 800;
    constexpr int windowHeight = 600;
    constexpr int centerX = windowWidth / 2;
    constexpr int centerY = windowHeight / 2;
    constexpr int largestRadius = 230;
    constexpr int ringCount = 20;

    initgraph(windowWidth, windowHeight);
    setbkcolor(WHITE);
    cleardevice();

    for (int index = 0; index < ringCount; ++index)
    {
        float progress = (float)index / (ringCount - 1);
        float radius = largestRadius * (1.0f - progress * 0.9f);
        int red = (int)(255.0f - progress * 120.0f);
        int green = (int)(progress * 115.0f);
        int blue = (int)(progress * 115.0f);
        setlinecolor(RGB(red, green, blue));
        circle(centerX, centerY, (int)radius);
    }

    _getch();
    closegraph();
    return 0;
}