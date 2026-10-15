#include <graphics.h>
#include <conio.h>
#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <tchar.h>
#include <stdio.h>

// =====================================================================
// Practice-2 : Going Down 100 Levels  (safe-board rhythm + gift bags)
// =====================================================================

struct Board
{
    float x, y;
    float w;
    bool  thorny;
    bool  hasGift;
};

void drawPerson(float x, float y)
{
    setfillcolor(RGB(255, 220, 180));
    setlinecolor(RGB(80, 50, 30));
    setlinestyle(PS_SOLID, 2);
    fillcircle((int)x, (int)(y - 40), 10);

    setfillcolor(RGB(50, 100, 200));
    setlinecolor(RGB(20, 40, 100));
    solidrectangle((int)(x - 8), (int)(y - 30), (int)(x + 8), (int)(y - 10));

    setlinecolor(RGB(20, 40, 100));
    setlinestyle(PS_SOLID, 4);
    line((int)(x - 8), (int)(y - 25), (int)(x - 18), (int)(y - 15));
    line((int)(x + 8), (int)(y - 25), (int)(x + 18), (int)(y - 15));

    line((int)(x - 5), (int)(y - 10), (int)(x - 8), (int)(y));
    line((int)(x + 5), (int)(y - 10), (int)(x + 8), (int)(y));
}

void drawThornRow(int x1, int x2, int y, int toothW, int toothH)
{
    setlinecolor(RGB(230, 40, 40));
    setlinestyle(PS_SOLID, 2);
    for (int i = x1; i + toothW <= x2; i += toothW)
    {
        line(i, y, i + toothW / 2, y + toothH);
        line(i + toothW / 2, y + toothH, i + toothW, y);
    }
}

int main()
{
    constexpr int W = 600;
    constexpr int H = 800;
    initgraph(W, H);
    srand((unsigned)time(0));

    float playerX = W / 2.0f;
    float playerY = 0.0f;
    float playerVY = 0.0f;
    constexpr float gravity = 0.6f;
    constexpr float moveSpeed = 6.0f;
    constexpr float playerW = 20.0f;
    constexpr float playerH = 45.0f;

    constexpr int numBoards = 6;
    Board boards[numBoards];

    constexpr float BOARD_SPEED = 1.5f;
    constexpr float MAX_H_GAP = 200.0f;

    float lastBoardCenterX = W / 2.0f;

    // ===== Safe-board rhythm tracking =====
    int safeBoardsSinceThorny = 0;
    constexpr int MIN_SAFE_BEFORE_THORNY = 3;
    constexpr int MAX_SAFE_BEFORE_THORNY = 4;

    auto spawnBoard = [&](int i, float startY, bool forceNormal = false, bool useLastX = true)
        {
            float w = 100.0f + rand() % 40;

            float baseX = useLastX ? lastBoardCenterX : (W / 2.0f);
            float offset = (float)(rand() % (int)(2 * MAX_H_GAP)) - MAX_H_GAP;

            float centerX = baseX + offset;
            if (centerX < w / 2 + 10)     centerX = w / 2 + 10;
            if (centerX > W - w / 2 - 10) centerX = W - w / 2 - 10;

            lastBoardCenterX = centerX;

            boards[i].x = centerX - w / 2;
            boards[i].y = startY;
            boards[i].w = w;

            // ---- Decide if this board should be thorny ----
            if (forceNormal)
            {
                boards[i].thorny = false;
                safeBoardsSinceThorny = 0;
            }
            else
            {
                int targetSafe = MIN_SAFE_BEFORE_THORNY +
                    (rand() % (MAX_SAFE_BEFORE_THORNY - MIN_SAFE_BEFORE_THORNY + 1));

                if (safeBoardsSinceThorny >= targetSafe)
                {
                    boards[i].thorny = true;
                    safeBoardsSinceThorny = 0;
                }
                else
                {
                    boards[i].thorny = false;
                    safeBoardsSinceThorny++;
                }
            }

            // ---- Decide if this board has a gift ----
            // Only safe boards can have a gift, and only ~1 in 5
            if (!boards[i].thorny && !forceNormal && (rand() % 5 == 0))
                boards[i].hasGift = true;
            else
                boards[i].hasGift = false;
        };

    constexpr int thornTopY = 30;
    constexpr int thornH = 25;
    constexpr int toothW = 30;

    int hp = 100;
    bool gameOver = false;
    bool started = false;
    int score = 0;
    bool onThornyBoard = false;

    auto resetGame = [&]()
        {
            hp = 100;
            score = 0;
            gameOver = false;
            started = false;
            onThornyBoard = false;
            safeBoardsSinceThorny = 0;
            lastBoardCenterX = W / 2.0f;

            for (int i = 0; i < numBoards; ++i)
            {
                float y = (float)(H - 150 - i * 110);
                spawnBoard(i, y, (i == 0));
            }

            playerX = boards[0].x + boards[0].w / 2.0f;
            playerY = boards[0].y;
            playerVY = 0.0f;
        };

    resetGame();

    BeginBatchDraw();

    while (1)
    {
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
            break;

        if ((GetAsyncKeyState(VK_SPACE) & 0x8000) && !started && !gameOver)
        {
            started = true;
        }

        if ((GetAsyncKeyState('R') & 0x8000) && gameOver)
        {
            resetGame();
            Sleep(200);
        }

        if (started && !gameOver)
        {
            if (GetAsyncKeyState(VK_LEFT) & 0x8000)
            {
                playerX -= moveSpeed;
                if (playerX - playerW / 2 < 0) playerX = playerW / 2;
            }
            if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
            {
                playerX += moveSpeed;
                if (playerX + playerW / 2 > W) playerX = W - playerW / 2;
            }
        }

        if (!started && !gameOver)
        {
            playerX = boards[0].x + boards[0].w / 2.0f;
            playerY = boards[0].y;
            playerVY = 0.0f;
        }

        if (started && !gameOver)
        {
            float oldPlayerY = playerY;
            bool onBoard = false;
            int onBoardIndex = -1;

            for (int i = 0; i < numBoards; ++i)
            {
                boards[i].y -= BOARD_SPEED;

                if (boards[i].y + 12 < 0)
                {
                    spawnBoard(i, (float)H + 20);
                    score++;
                }
            }

            playerVY += gravity;
            playerY += playerVY;

            for (int i = 0; i < numBoards; ++i)
            {
                float px1 = playerX - playerW / 2;
                float px2 = playerX + playerW / 2;
                float bx1 = boards[i].x;
                float bx2 = boards[i].x + boards[i].w;
                float boardTop = boards[i].y;
                float boardBottom = boards[i].y + 12;

                bool overlapX = (px2 > bx1) && (px1 < bx2);

                bool feetInsideBoard = (playerY >= boardTop) &&
                    (playerY <= boardBottom);

                bool crossedFromAbove = (oldPlayerY <= boardTop) &&
                    (playerY >= boardTop);

                bool wasStanding = (oldPlayerY >= boardTop - 5) &&
                    (oldPlayerY <= boardBottom + 5) &&
                    (playerY >= boardTop - 5);

                if (overlapX && playerVY >= 0 &&
                    (feetInsideBoard || crossedFromAbove || wasStanding))
                {
                    playerY = boardTop;
                    playerVY = 0;
                    onBoard = true;
                    onBoardIndex = i;

                    if (boards[i].thorny && !onThornyBoard)
                    {
                        hp -= 50;
                        onThornyBoard = true;
                        if (hp <= 0) gameOver = true;
                    }

                    // ---- Pick up gift if this board has one ----
                    if (boards[i].hasGift)
                    {
                        hp += 25;
                        if (hp > 100) hp = 100;
                        boards[i].hasGift = false;
                    }

                    break;
                }
            }

            if (onBoard && onBoardIndex >= 0)
            {
                playerY = boards[onBoardIndex].y;
            }

            if (!onBoard || onBoardIndex < 0 || !boards[onBoardIndex].thorny)
                onThornyBoard = false;

            if (playerY - playerH <= thornTopY + thornH)
            {
                hp = 0;
                gameOver = true;
            }

            if (playerY > H + 50)
            {
                hp = 0;
                gameOver = true;
            }
        }

        cleardevice();

        setfillcolor(RGB(30, 30, 50));
        solidrectangle(0, 0, W, H);

        for (int i = 0; i < numBoards; ++i)
        {
            if (boards[i].thorny)
                setfillcolor(RGB(120, 30, 30));
            else
                setfillcolor(RGB(90, 90, 120));
            setlinecolor(RGB(200, 200, 200));
            setlinestyle(PS_SOLID, 2);
            solidrectangle((int)boards[i].x, (int)boards[i].y,
                (int)(boards[i].x + boards[i].w),
                (int)(boards[i].y + 12));

            if (boards[i].thorny)
                drawThornRow((int)boards[i].x,
                    (int)(boards[i].x + boards[i].w),
                    (int)boards[i].y - 10, 15, 10);

            // ---- Draw gift (health pack) on safe boards that have one ----
            if (boards[i].hasGift)
            {
                int gx = (int)(boards[i].x + boards[i].w / 2);
                int gy = (int)boards[i].y - 15;

                setfillcolor(RGB(220, 40, 40));
                setlinecolor(RGB(120, 0, 0));
                setlinestyle(PS_SOLID, 2);
                solidrectangle(gx - 9, gy - 3, gx + 9, gy + 3);
                solidrectangle(gx - 3, gy - 9, gx + 3, gy + 9);
            }
        }

        drawThornRow(0, W, thornTopY, toothW, thornH);
        drawPerson(playerX, playerY);

        setfillcolor(RGB(50, 50, 50));
        setlinecolor(RGB(255, 255, 255));
        setlinestyle(PS_SOLID, 2);
        solidrectangle(15, 15, 215, 40);

        setfillcolor(hp > 50 ? RGB(80, 220, 80) :
            hp > 0 ? RGB(220, 180, 60) :
            RGB(220, 40, 40));
        solidrectangle(17, 17, 17 + (int)(hp * 1.96f), 38);

        settextcolor(WHITE);
        settextstyle(16, 0, _T("Arial"));
        setbkmode(TRANSPARENT);
        TCHAR hpBuf[32];
        _stprintf_s(hpBuf, _T("HP: %d%%"), hp);
        outtextxy(25, 19, hpBuf);

        TCHAR scBuf[32];
        _stprintf_s(scBuf, _T("Score: %d"), score);
        outtextxy(W - 130, 20, scBuf);

        if (!started && !gameOver)
        {
            setfillcolor(RGB(0, 0, 0));
            solidrectangle(100, H / 2 - 60, W - 100, H / 2 + 60);

            settextcolor(RGB(255, 255, 100));
            settextstyle(30, 0, _T("Arial"));
            outtextxy(140, H / 2 - 30, _T("Press SPACE"));
            settextstyle(22, 0, _T("Arial"));
            outtextxy(180, H / 2 + 10, _T("to start"));
        }

        if (gameOver)
        {
            setfillcolor(RGB(0, 0, 0));
            solidrectangle(100, H / 2 - 80, W - 100, H / 2 + 80);

            settextcolor(RGB(255, 60, 60));
            settextstyle(40, 0, _T("Arial"));
            outtextxy(140, H / 2 - 60, _T("GAME OVER"));

            settextcolor(WHITE);
            settextstyle(22, 0, _T("Arial"));
            TCHAR mbuf[64];
            _stprintf_s(mbuf, _T("Score: %d"), score);
            outtextxy(230, H / 2, mbuf);

            settextstyle(18, 0, _T("Arial"));
            outtextxy(180, H / 2 + 40, _T("Press R to restart"));
            outtextxy(180, H / 2 + 65, _T("Press ESC to exit"));
        }
        else if (started)
        {
            settextcolor(RGB(180, 180, 180));
            settextstyle(14, 0, _T("Arial"));
            outtextxy(15, H - 25, _T("LEFT / RIGHT to move   ESC to quit"));
        }

        FlushBatchDraw();
        Sleep(15);
    }

    EndBatchDraw();
    closegraph();
    return 0;
}