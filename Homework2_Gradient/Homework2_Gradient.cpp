#include <graphics.h>
#include <conio.h>
#include <windows.h>

int main()
{
    constexpr int windowWidth = 800;
    constexpr int windowHeight = 600;
    constexpr int circleRadius = 110;

    initgraph(windowWidth, windowHeight);
    cleardevice();

    // Green gradient background
    for (int row = 0; row < windowHeight; ++row)
    {
        float progress = (float)row / (windowHeight - 1);
        int green = (int)(55.0f + progress * 200.0f);
        setlinecolor(RGB(0, green, 0));
        line(0, row, windowWidth, row);
    }

    // Red-yellow gradient circle
    constexpr int layers = 100;
    for (int layer = layers; layer >= 1; --layer)
    {
        float progress = (float)(layers - layer) / (layers - 1);
        int green = (int)(progress * 145.0f);
        int blue = (int)(progress * 8.0f);
        int radius = circleRadius * layer / layers;
        setfillcolor(RGB(255, green, blue));
        setlinecolor(RGB(255, green, blue));
        fillcircle(windowWidth / 2, windowHeight / 2, radius);
    }

    _getch();
    closegraph();
    return 0;
}