#include <graphics.h>
#include <conio.h>
#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <tchar.h>
#include <stdio.h>

// =====================================================================
// Practice-1 : Bomberman  (fixed map + upgraded visuals)
// Controls:
//   ARROW KEYS -> move player
//   B          -> place bomb
//   SPACE      -> start the game
//   R          -> restart
//   ESC        -> quit
// =====================================================================

constexpr int TILE = 50;
constexpr int COLS = 9;
constexpr int ROWS = 9;
constexpr int HUD_H = 60;
constexpr int W = COLS * TILE;
constexpr int H = ROWS * TILE + HUD_H;

enum Tile { EMPTY = 0, CONCRETE = 1, BRICK = 2 };

constexpr int DX[4] = { 0, 0, -1, 1 };
constexpr int DY[4] = { -1, 1, 0, 0 };

struct Bomb { bool active; int cx, cy; int timer; };
struct Enemy { bool alive; int cx, cy; int dir; int moveTimer; };
struct Flame { bool active; int cx, cy; int timer; };

// =====================================================================
// Draw a person sprite (bomber) — better quality
// =====================================================================
void drawPlayer(int px, int py)
{
    int cx = px + TILE / 2;
    int cy = py + TILE / 2;

    // Shadow
    setfillcolor(RGB(0, 0, 0, 40));
    setlinecolor(RGB(0, 0, 0, 40));
    solidellipse(cx - 14, cy + 14, cx + 14, cy + 20);

    // Body (blue overalls)
    setfillcolor(RGB(60, 110, 220));
    setlinecolor(RGB(20, 40, 100));
    setlinestyle(PS_SOLID, 2);
    solidrectangle(cx - 10, cy + 2, cx + 10, cy + 16);

    // Legs
    setfillcolor(RGB(40, 40, 60));
    solidrectangle(cx - 8, cy + 16, cx - 3, cy + 20);
    solidrectangle(cx + 3, cy + 16, cx + 8, cy + 20);

    // Head
    setfillcolor(RGB(255, 220, 180));
    setlinecolor(RGB(120, 80, 40));
    fillcircle(cx, cy - 8, 10);

    // Helmet
    setfillcolor(RGB(240, 60, 60));
    setlinecolor(RGB(140, 20, 20));
    solidellipse(cx - 12, cy - 20, cx + 12, cy - 8);

    // Eyes
    setfillcolor(RGB(0, 0, 0));
    solidcircle(cx - 3, cy - 6, 2);
    solidcircle(cx + 3, cy - 6, 2);
}

// =====================================================================
// Draw an enemy sprite
// =====================================================================
void drawEnemy(int px, int py)
{
    int cx = px + TILE / 2;
    int cy = py + TILE / 2;

    // Shadow
    setfillcolor(RGB(0, 0, 0, 40));
    setlinecolor(RGB(0, 0, 0, 40));
    solidellipse(cx - 14, cy + 12, cx + 14, cy + 18);

    // Body (red ghost-like)
    setfillcolor(RGB(230, 70, 70));
    setlinecolor(RGB(120, 0, 0));
    setlinestyle(PS_SOLID, 2);
    fillcircle(cx, cy - 2, 16);

    // Bottom skirt (semi-circle flaps)
    setfillcolor(RGB(200, 50, 50));
    solidrectangle(cx - 16, cy - 2, cx + 16, cy + 8);

    // Eyes
    setfillcolor(RGB(255, 255, 255));
    setlinecolor(RGB(0, 0, 0));
    fillcircle(cx - 6, cy - 6, 5);
    fillcircle(cx + 6, cy - 6, 5);

    setfillcolor(RGB(0, 0, 0));
    setlinecolor(RGB(0, 0, 0));
    solidcircle(cx - 6, cy - 6, 2);
    solidcircle(cx + 6, cy - 6, 2);

    // Angry eyebrows
    setlinecolor(RGB(0, 0, 0));
    setlinestyle(PS_SOLID, 2);
    line(cx - 10, cy - 12, cx - 3, cy - 10);
    line(cx + 10, cy - 12, cx + 3, cy - 10);

    // Mouth
    setlinestyle(PS_SOLID, 2);
    arc(cx - 5, cy + 2, cx + 5, cy + 8, 0, 3.14f);
}

// =====================================================================
// Draw a bomb
// =====================================================================
void drawBomb(int px, int py, int timer)
{
    int cx = px + TILE / 2;
    int cy = py + TILE / 2;
    int pulse = (timer % 6 < 3) ? 0 : 2;

    // Shadow
    setfillcolor(RGB(0, 0, 0, 60));
    setlinecolor(RGB(0, 0, 0, 60));
    solidellipse(cx - 12, cy + 12, cx + 12, cy + 18);

    // Bomb body
    setfillcolor(RGB(30, 30, 30));
    setlinecolor(RGB(0, 0, 0));
    setlinestyle(PS_SOLID, 2);
    fillcircle(cx, cy, 15 + pulse);

    // Shine highlight
    setfillcolor(RGB(120, 120, 120));
    setlinecolor(RGB(120, 120, 120));
    solidcircle(cx - 5, cy - 6, 4);

    // Fuse
    setlinecolor(RGB(200, 150, 60));
    setlinestyle(PS_SOLID, 3);
    line(cx + 8, cy - 14, cx + 14, cy - 22);
    line(cx + 14, cy - 22, cx + 16, cy - 26);

    // Spark
    setfillcolor(RGB(255, 220, 60));
    setlinecolor(RGB(255, 120, 0));
    solidcircle(cx + 16, cy - 26, 3);
}

// =====================================================================
// Draw a flame
// =====================================================================
void drawFlame(int px, int py, int timer)
{
    int cx = px + TILE / 2;
    int cy = py + TILE / 2;
    int r = TILE / 2;

    // Outer glow
    setfillcolor(RGB(255, 100, 0));
    setlinecolor(RGB(255, 100, 0));
    fillcircle(cx, cy, r);

    // Middle
    setfillcolor(RGB(255, 180, 0));
    setlinecolor(RGB(255, 180, 0));
    fillcircle(cx, cy, r - 6);

    // Inner core
    setfillcolor(RGB(255, 240, 120));
    setlinecolor(RGB(255, 240, 120));
    fillcircle(cx, cy, r - 14);
}

// =====================================================================
// Draw a concrete block
// =====================================================================
void drawConcrete(int x, int y)
{
    // Base
    setfillcolor(RGB(180, 180, 190));
    setlinecolor(RGB(90, 90, 100));
    setlinestyle(PS_SOLID, 2);
    fillrectangle(x + 1, y + 1, x + TILE - 1, y + TILE - 1);

    // Top highlight
    setfillcolor(RGB(220, 220, 230));
    setlinecolor(RGB(220, 220, 230));
    solidrectangle(x + 3, y + 3, x + TILE - 3, y + 8);

    // Bottom shadow
    setfillcolor(RGB(120, 120, 130));
    solidrectangle(x + 3, y + TILE - 8, x + TILE - 3, y + TILE - 3);

    // Inner border
    setlinecolor(RGB(140, 140, 150));
    rectangle(x + 6, y + 6, x + TILE - 6, y + TILE - 6);
}

// =====================================================================
// Draw a breakable brick
// =====================================================================
void drawBrick(int x, int y)
{
    // Base
    setfillcolor(RGB(180, 90, 50));
    setlinecolor(RGB(100, 40, 20));
    setlinestyle(PS_SOLID, 2);
    fillrectangle(x + 1, y + 1, x + TILE - 1, y + TILE - 1);

    // Brick lines (2 rows)
    setlinecolor(RGB(80, 30, 10));
    setlinestyle(PS_SOLID, 2);

    // Horizontal split
    line(x + 1, y + TILE / 2, x + TILE - 1, y + TILE / 2);

    // Vertical splits (offset between rows)
    line(x + TILE / 3, y + 1, x + TILE / 3, y + TILE / 2);
    line(x + 2 * TILE / 3, y + 1, x + 2 * TILE / 3, y + TILE / 2);

    line(x + TILE / 2, y + TILE / 2, x + TILE / 2, y + TILE - 1);

    // Highlight
    setlinecolor(RGB(220, 140, 90));
    line(x + 3, y + 3, x + TILE - 3, y + 3);
}

// =====================================================================
// Draw the grass floor with a subtle pattern
// =====================================================================
void drawGround(int x, int y, int r, int c)
{
    // Two-tone grass for checker feel
    if ((r + c) % 2 == 0)
        setfillcolor(RGB(80, 180, 80));
    else
        setfillcolor(RGB(70, 165, 70));

    setlinecolor(RGB(60, 140, 60));
    setlinestyle(PS_SOLID, 1);
    fillrectangle(x, y, x + TILE, y + TILE);

    // Small grass blade highlights
    setlinecolor(RGB(110, 210, 110));
    line(x + 10, y + TILE - 12, x + 12, y + TILE - 6);
    line(x + 20, y + TILE - 15, x + 22, y + TILE - 8);
    line(x + 32, y + TILE - 10, x + 34, y + TILE - 5);
}

// =====================================================================
// Main
// =====================================================================
int main()
{
    initgraph(W, H);
    srand((unsigned)time(0));

    int grid[ROWS][COLS];

    auto resetGame = [&]()
        {
            // Fill with empty
            for (int r = 0; r < ROWS; ++r)
                for (int c = 0; c < COLS; ++c)
                    grid[r][c] = EMPTY;

            // Concrete blocks on odd rows/cols (checker pattern)
            for (int r = 0; r < ROWS; ++r)
                for (int c = 0; c < COLS; ++c)
                    if ((r % 2 == 1) && (c % 2 == 1))
                        grid[r][c] = CONCRETE;

            // Keep the player's spawn area + corridor clear
            grid[0][0] = EMPTY;
            grid[0][1] = EMPTY;
            grid[0][2] = EMPTY;   // horizontal escape
            grid[1][0] = EMPTY;
            grid[2][0] = EMPTY;   // vertical escape

            // Bricks on the remaining empty tiles (except spawn area)
            for (int r = 0; r < ROWS; ++r)
            {
                for (int c = 0; c < COLS; ++c)
                {
                    if (grid[r][c] != EMPTY) continue;
                    if (r == 0 && c == 0) continue;   // player spawn
                    if (r == 0 && c == 1) continue;
                    if (r == 1 && c == 0) continue;
                    if (rand() % 3 == 0) grid[r][c] = BRICK;
                }
            }
        };

    resetGame();

    int playerCX = 0, playerCY = 0;
    int playerMoveTimer = 0;
    constexpr int PLAYER_MOVE_DELAY = 6;

    constexpr int maxBombs = 3;
    constexpr int maxFlames = 60;
    Bomb bombs[maxBombs];
    Flame flames[maxFlames];
    for (int i = 0; i < maxBombs; ++i) bombs[i].active = false;
    for (int i = 0; i < maxFlames; ++i) flames[i].active = false;
    int bombsPlaced = 0;

    constexpr int maxEnemies = 4;
    Enemy enemies[maxEnemies];
    for (int i = 0; i < maxEnemies; ++i) enemies[i].alive = false;

    auto resetEnemies = [&]()
        {
            for (int i = 0; i < maxEnemies; ++i)
            {
                enemies[i].alive = false;
                int attempts = 100;
                while (attempts-- > 0)
                {
                    int c = rand() % COLS;
                    int r = ROWS - 1 - (rand() % 4);
                    if (grid[r][c] == EMPTY &&
                        !(r == playerCY && c == playerCX) &&
                        !(r == 0 && c == 0))
                    {
                        enemies[i].alive = true;
                        enemies[i].cx = c;
                        enemies[i].cy = r;
                        enemies[i].dir = rand() % 4;
                        enemies[i].moveTimer = 0;
                        break;
                    }
                }
            }
        };

    resetEnemies();

    int hp = 3;
    int score = 0;
    bool gameOver = false;
    bool win = false;
    bool started = false;

    auto fullReset = [&]()
        {
            resetGame();
            playerCX = 0;
            playerCY = 0;
            hp = 3;
            score = 0;
            gameOver = false;
            win = false;
            started = false;
            bombsPlaced = 0;
            for (int i = 0; i < maxBombs; ++i) bombs[i].active = false;
            for (int i = 0; i < maxFlames; ++i) flames[i].active = false;
            resetEnemies();
        };

    BeginBatchDraw();

    while (1)
    {
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) break;

        if ((GetAsyncKeyState('R') & 0x8000) && (gameOver || win))
        {
            fullReset();
            Sleep(200);
        }

        if ((GetAsyncKeyState(VK_SPACE) & 0x8000) && !started && !gameOver && !win)
            started = true;

        // ===== Player movement =====
        if (started && !gameOver && !win)
        {
            if (playerMoveTimer > 0) playerMoveTimer--;

            int moveDir = -1;
            if (GetAsyncKeyState(VK_LEFT) & 0x8000)  moveDir = 2;
            if (GetAsyncKeyState(VK_RIGHT) & 0x8000) moveDir = 3;
            if (GetAsyncKeyState(VK_UP) & 0x8000)    moveDir = 0;
            if (GetAsyncKeyState(VK_DOWN) & 0x8000)  moveDir = 1;

            if (moveDir >= 0 && playerMoveTimer == 0)
            {
                int nc = playerCX + DX[moveDir];
                int nr = playerCY + DY[moveDir];

                if (nc >= 0 && nc < COLS && nr >= 0 && nr < ROWS)
                {
                    if (grid[nr][nc] == EMPTY)
                    {
                        playerCX = nc;
                        playerCY = nr;
                        playerMoveTimer = PLAYER_MOVE_DELAY;
                    }
                }
            }

            if ((GetAsyncKeyState('B') & 0x8000) && bombsPlaced < maxBombs)
            {
                for (int i = 0; i < maxBombs; ++i)
                {
                    if (!bombs[i].active)
                    {
                        bombs[i].active = true;
                        bombs[i].cx = playerCX;
                        bombs[i].cy = playerCY;
                        bombs[i].timer = 60;
                        bombsPlaced++;
                        break;
                    }
                }
                Sleep(200);
            }
        }

        // ===== Bomb ticking =====
        if (started && !gameOver && !win)
        {
            for (int i = 0; i < maxBombs; ++i)
            {
                if (!bombs[i].active) continue;
                bombs[i].timer--;
                if (bombs[i].timer <= 0)
                {
                    bombs[i].active = false;
                    bombsPlaced--;

                    int cx = bombs[i].cx;
                    int cy = bombs[i].cy;

                    auto spawnFlame = [&](int c, int r)
                        {
                            if (c < 0 || c >= COLS || r < 0 || r >= ROWS) return;
                            if (grid[r][c] == CONCRETE) return;

                            if (grid[r][c] == BRICK)
                            {
                                grid[r][c] = EMPTY;
                                score += 10;
                                return;
                            }

                            for (int f = 0; f < maxFlames; ++f)
                            {
                                if (!flames[f].active)
                                {
                                    flames[f].active = true;
                                    flames[f].cx = c;
                                    flames[f].cy = r;
                                    flames[f].timer = 20;
                                    break;
                                }
                            }
                        };

                    spawnFlame(cx, cy);
                    for (int d = 0; d < 4; ++d)
                        for (int dist = 1; dist <= 2; ++dist)
                            spawnFlame(cx + DX[d] * dist, cy + DY[d] * dist);
                }
            }

            for (int i = 0; i < maxFlames; ++i)
            {
                if (!flames[i].active) continue;
                flames[i].timer--;
                if (flames[i].timer <= 0) flames[i].active = false;
            }
        }

        // ===== Enemy movement =====
        if (started && !gameOver && !win)
        {
            for (int e = 0; e < maxEnemies; ++e)
            {
                if (!enemies[e].alive) continue;

                if (enemies[e].moveTimer > 0)
                {
                    enemies[e].moveTimer--;
                }
                else
                {
                    int nc = enemies[e].cx + DX[enemies[e].dir];
                    int nr = enemies[e].cy + DY[enemies[e].dir];

                    if (nc >= 0 && nc < COLS && nr >= 0 && nr < ROWS &&
                        grid[nr][nc] == EMPTY)
                    {
                        bool blocked = false;
                        for (int k = 0; k < maxEnemies; ++k)
                        {
                            if (k == e || !enemies[k].alive) continue;
                            if (enemies[k].cx == nc && enemies[k].cy == nr) { blocked = true; break; }
                        }
                        if (!blocked)
                        {
                            enemies[e].cx = nc;
                            enemies[e].cy = nr;
                        }
                        else enemies[e].dir = rand() % 4;
                    }
                    else enemies[e].dir = rand() % 4;

                    enemies[e].moveTimer = 25;
                }

                for (int f = 0; f < maxFlames; ++f)
                {
                    if (!flames[f].active) continue;
                    if (flames[f].cx == enemies[e].cx &&
                        flames[f].cy == enemies[e].cy)
                    {
                        enemies[e].alive = false;
                        score += 50;
                        break;
                    }
                }
            }

            for (int f = 0; f < maxFlames; ++f)
            {
                if (!flames[f].active) continue;
                if (flames[f].cx == playerCX && flames[f].cy == playerCY)
                {
                    flames[f].active = false;
                    hp--;
                    if (hp <= 0) gameOver = true;
                    break;
                }
            }

            for (int e = 0; e < maxEnemies; ++e)
            {
                if (!enemies[e].alive) continue;
                if (enemies[e].cx == playerCX && enemies[e].cy == playerCY)
                {
                    hp = 0;
                    gameOver = true;
                    break;
                }
            }
        }

        if (started && !gameOver && !win)
        {
            bool anyAlive = false;
            for (int e = 0; e < maxEnemies; ++e)
                if (enemies[e].alive) anyAlive = true;
            if (!anyAlive) win = true;
        }

        // ===== Draw =====
        cleardevice();

        // HUD
        setfillcolor(RGB(30, 20, 40));
        solidrectangle(0, 0, W, HUD_H);
        setfillcolor(RGB(60, 40, 80));
        solidrectangle(0, HUD_H - 3, W, HUD_H);

        settextcolor(RGB(255, 220, 100));
        settextstyle(22, 0, _T("Arial"));
        setbkmode(TRANSPARENT);
        TCHAR buf[64];
        _stprintf_s(buf, _T("HP: %d    Score: %d    Bombs: %d/%d"),
            hp, score, bombsPlaced, maxBombs);
        outtextxy(20, 20, buf);

        // Grid
        for (int r = 0; r < ROWS; ++r)
        {
            for (int c = 0; c < COLS; ++c)
            {
                int x = c * TILE;
                int y = r * TILE + HUD_H;

                drawGround(x, y, r, c);

                if (grid[r][c] == CONCRETE)      drawConcrete(x, y);
                else if (grid[r][c] == BRICK)    drawBrick(x, y);
            }
        }

        for (int i = 0; i < maxBombs; ++i)
            if (bombs[i].active)
                drawBomb(bombs[i].cx * TILE, bombs[i].cy * TILE + HUD_H, bombs[i].timer);

        for (int i = 0; i < maxEnemies; ++i)
            if (enemies[i].alive)
                drawEnemy(enemies[i].cx * TILE, enemies[i].cy * TILE + HUD_H);

        drawPlayer(playerCX * TILE, playerCY * TILE + HUD_H);

        for (int i = 0; i < maxFlames; ++i)
            if (flames[i].active)
                drawFlame(flames[i].cx * TILE, flames[i].cy * TILE + HUD_H, flames[i].timer);

        // Overlays
        if (!started)
        {
            setfillcolor(RGB(0, 0, 0));
            solidrectangle(100, H / 2 - 80, W - 100, H / 2 + 80);
            settextcolor(RGB(255, 220, 100));
            settextstyle(36, 0, _T("Arial"));
            outtextxy(160, H / 2 - 50, _T("BOMBERMAN"));
            settextstyle(22, 0, _T("Arial"));
            outtextxy(140, H / 2 + 5, _T("SPACE to start"));
            settextstyle(16, 0, _T("Arial"));
            outtextxy(100, H / 2 + 40, _T("Arrows: move    B: bomb"));
        }

        if (gameOver)
        {
            setfillcolor(RGB(0, 0, 0));
            solidrectangle(100, H / 2 - 80, W - 100, H / 2 + 80);
            settextcolor(RGB(255, 60, 60));
            settextstyle(40, 0, _T("Arial"));
            outtextxy(140, H / 2 - 50, _T("GAME OVER"));
            settextcolor(WHITE);
            settextstyle(22, 0, _T("Arial"));
            TCHAR mb[64];
            _stprintf_s(mb, _T("Score: %d"), score);
            outtextxy(200, H / 2 + 10, mb);
            settextstyle(18, 0, _T("Arial"));
            outtextxy(180, H / 2 + 50, _T("Press R to restart"));
        }

        if (win)
        {
            setfillcolor(RGB(0, 0, 0));
            solidrectangle(100, H / 2 - 80, W - 100, H / 2 + 80);
            settextcolor(RGB(80, 255, 80));
            settextstyle(40, 0, _T("Arial"));
            outtextxy(180, H / 2 - 50, _T("YOU WIN!"));
            settextcolor(WHITE);
            settextstyle(22, 0, _T("Arial"));
            TCHAR mb[64];
            _stprintf_s(mb, _T("Score: %d"), score);
            outtextxy(200, H / 2 + 10, mb);
            settextstyle(18, 0, _T("Arial"));
            outtextxy(180, H / 2 + 50, _T("Press R to restart"));
        }

        FlushBatchDraw();
        Sleep(16);
    }

    EndBatchDraw();
    closegraph();
    return 0;
}