#include <graphics.h>
#include <conio.h>
#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <tchar.h>
#include <stdio.h>

// =====================================================================
// Practice-1 : Bomberman (basic version)
// Controls:
//   ARROW KEYS -> move player
//   SPACE      -> place bomb
//   R          -> restart
//   ESC        -> quit
// =====================================================================

// ----- Grid constants -----
constexpr int TILE = 50;
constexpr int COLS = 9;
constexpr int ROWS = 9;
constexpr int W = COLS * TILE;
constexpr int H = ROWS * TILE + 60;    // +60 for HUD

// ----- Tile types -----
enum Tile { EMPTY = 0, CONCRETE = 1, BRICK = 2 };

// ----- Directions -----
constexpr int DX[4] = { 0, 0, -1, 1 };
constexpr int DY[4] = { -1, 1, 0, 0 };

// ----- Bomb -----
struct Bomb { bool active; int cx, cy; int timer; };

// ----- Enemy -----
struct Enemy { bool alive; int cx, cy; int dir; int moveTimer; };

// ----- Flame (explosion) -----
struct Flame { bool active; int cx, cy; int timer; };

// =====================================================================
// Draw a player (bomber)
// =====================================================================
void drawPlayer(int px, int py)
{
    int cx = px + TILE / 2;
    int cy = py + TILE / 2;

    setfillcolor(RGB(255, 255, 255));
    setlinecolor(RGB(0, 0, 0));
    setlinestyle(PS_SOLID, 2);

    // Head
    fillcircle(cx, cy - 8, 10);

    // Body
    setfillcolor(RGB(80, 120, 255));
    solidrectangle(cx - 10, cy + 2, cx + 10, cy + 18);

    // Eyes
    setfillcolor(RGB(0, 0, 0));
    solidcircle(cx - 3, cy - 10, 2);
    solidcircle(cx + 3, cy - 10, 2);
}

// =====================================================================
// Draw an enemy
// =====================================================================
void drawEnemy(int px, int py)
{
    int cx = px + TILE / 2;
    int cy = py + TILE / 2;

    setfillcolor(RGB(255, 80, 80));
    setlinecolor(RGB(120, 0, 0));
    setlinestyle(PS_SOLID, 2);
    fillcircle(cx, cy, 16);

    setfillcolor(RGB(255, 220, 0));
    solidcircle(cx - 5, cy - 3, 3);
    solidcircle(cx + 5, cy - 3, 3);

    setfillcolor(RGB(0, 0, 0));
    solidcircle(cx - 5, cy - 3, 1);
    solidcircle(cx + 5, cy - 3, 1);
}

// =====================================================================
// Draw a bomb
// =====================================================================
void drawBomb(int px, int py, int timer)
{
    int cx = px + TILE / 2;
    int cy = py + TILE / 2;

    // Pulsing size based on time remaining
    int pulse = (timer % 6 < 3) ? 0 : 2;

    setfillcolor(RGB(30, 30, 30));
    setlinecolor(RGB(0, 0, 0));
    setlinestyle(PS_SOLID, 2);
    fillcircle(cx, cy, 15 + pulse);

    // Fuse
    setlinecolor(RGB(200, 150, 60));
    line(cx + 8, cy - 14, cx + 14, cy - 22);
}

// =====================================================================
// Draw flames (explosion)
// =====================================================================
void drawFlame(int px, int py, int timer)
{
    int cx = px + TILE / 2;
    int cy = py + TILE / 2;
    int r = TILE / 2;

    int g = 100 + (timer * 10);
    if (g > 255) g = 255;

    setfillcolor(RGB(255, g, 0));
    setlinecolor(RGB(200, 80, 0));
    setlinestyle(PS_SOLID, 2);
    fillcircle(cx, cy, r);
}

// =====================================================================
// Main
// =====================================================================
int main()
{
    initgraph(W, H);
    srand((unsigned)time(0));

    // ---- Grid ----
    int grid[ROWS][COLS];

    auto resetGame = [&]()
        {
            // Fill with empty
            for (int r = 0; r < ROWS; ++r)
                for (int c = 0; c < COLS; ++c)
                    grid[r][c] = EMPTY;

            // Concrete on even rows/cols (checker pattern, but not on spawn areas)
            for (int r = 0; r < ROWS; ++r)
                for (int c = 0; c < COLS; ++c)
                    if ((r % 2 == 0) && (c % 2 == 0))
                        grid[r][c] = CONCRETE;

            // Bricks on remaining empty tiles (randomly), except spawn area
            for (int r = 0; r < ROWS; ++r)
            {
                for (int c = 0; c < COLS; ++c)
                {
                    if (grid[r][c] != EMPTY) continue;
                    if (r == 0 && c == 0) continue;            // player spawn
                    if (r == 0 && c == 1) continue;            // escape corridor
                    if (r == 1 && c == 0) continue;
                    if (rand() % 3 == 0) grid[r][c] = BRICK;
                }
            }
        };

    resetGame();

    // ---- Player ----
    int playerCX = 0, playerCY = 0;
    int playerMoveTimer = 0;
    constexpr int PLAYER_MOVE_DELAY = 6;

    // ---- Bombs / flames ----
    constexpr int maxBombs = 3;
    constexpr int maxFlames = 60;
    Bomb bombs[maxBombs];
    Flame flames[maxFlames];
    for (int i = 0; i < maxBombs; ++i) bombs[i].active = false;
    for (int i = 0; i < maxFlames; ++i) flames[i].active = false;
    int bombsPlaced = 0;

    // ---- Enemies ----
    constexpr int maxEnemies = 4;
    Enemy enemies[maxEnemies];
    for (int i = 0; i < maxEnemies; ++i) enemies[i].alive = false;

    auto resetEnemies = [&]()
        {
            for (int i = 0; i < maxEnemies; ++i)
            {
                int attempts = 50;
                while (attempts-- > 0)
                {
                    int c = rand() % COLS;
                    int r = ROWS - 1 - (rand() % 4);        // spawn in bottom half
                    if (grid[r][c] == EMPTY && !(r == playerCY && c == playerCX))
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

    // ---- Game state ----
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

            // ===== Bomb placement =====
            if ((GetAsyncKeyState('B') & 0x8000) && bombsPlaced < maxBombs)
            {
                for (int i = 0; i < maxBombs; ++i)
                {
                    if (!bombs[i].active)
                    {
                        bombs[i].active = true;
                        bombs[i].cx = playerCX;
                        bombs[i].cy = playerCY;
                        bombs[i].timer = 60;   // ~1 second
                        bombsPlaced++;
                        break;
                    }
                }
                Sleep(200);   // debounce
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

                    // ---- Spawn cross-shaped flame ----
                    int cx = bombs[i].cx;
                    int cy = bombs[i].cy;

                    auto spawnFlame = [&](int c, int r)
                        {
                            if (c < 0 || c >= COLS || r < 0 || r >= ROWS) return;
                            if (grid[r][c] == CONCRETE) return;

                            // Destroy brick
                            if (grid[r][c] == BRICK)
                            {
                                grid[r][c] = EMPTY;
                                score += 10;
                                return;   // flame does not pass through
                            }

                            // Add flame
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

                    // Center
                    spawnFlame(cx, cy);
                    // Cross
                    for (int d = 0; d < 4; ++d)
                    {
                        for (int dist = 1; dist <= 2; ++dist)
                        {
                            spawnFlame(cx + DX[d] * dist, cy + DY[d] * dist);
                        }
                    }
                }
            }

            // ===== Flame lifetime =====
            for (int i = 0; i < maxFlames; ++i)
            {
                if (!flames[i].active) continue;
                flames[i].timer--;
                if (flames[i].timer <= 0) flames[i].active = false;
            }
        }

        // ===== Enemies movement =====
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
                    // Try to move in current direction; if blocked, pick new direction
                    int nc = enemies[e].cx + DX[enemies[e].dir];
                    int nr = enemies[e].cy + DY[enemies[e].dir];

                    if (nc >= 0 && nc < COLS && nr >= 0 && nr < ROWS &&
                        grid[nr][nc] == EMPTY)
                    {
                        // Don't walk into other enemies
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
                        else
                        {
                            enemies[e].dir = rand() % 4;
                        }
                    }
                    else
                    {
                        enemies[e].dir = rand() % 4;
                    }

                    enemies[e].moveTimer = 25;   // ~0.4s between moves
                }

                // ===== Flame hits enemy =====
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

            // ===== Flame hits player =====
            for (int f = 0; f < maxFlames; ++f)
            {
                if (!flames[f].active) continue;
                if (flames[f].cx == playerCX && flames[f].cy == playerCY)
                {
                    // Damage once per flame (flame is consumed)
                    flames[f].active = false;
                    hp--;
                    if (hp <= 0) gameOver = true;
                    break;
                }
            }

            // ===== Enemy hits player =====
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

        // ===== Win condition: all enemies dead =====
        if (started && !gameOver && !win)
        {
            bool anyAlive = false;
            for (int e = 0; e < maxEnemies; ++e)
                if (enemies[e].alive) anyAlive = true;

            if (!anyAlive) win = true;
        }

        // ===== Draw =====
        cleardevice();

        // HUD bar at top
        setfillcolor(RGB(20, 20, 20));
        solidrectangle(0, 0, W, 60);
        settextcolor(WHITE);
        settextstyle(20, 0, _T("Arial"));
        setbkmode(TRANSPARENT);
        TCHAR buf[64];
        _stprintf_s(buf, _T("HP: %d    Score: %d    Bombs: %d/%d"),
            hp, score, bombsPlaced, maxBombs);
        outtextxy(20, 20, buf);

        // ===== Draw grid =====
        setlinestyle(PS_SOLID, 2);
        for (int r = 0; r < ROWS; ++r)
        {
            for (int c = 0; c < COLS; ++c)
            {
                int x = c * TILE;
                int y = r * TILE + 60;

                // Ground
                setfillcolor(RGB(60, 160, 60));
                setlinecolor(RGB(40, 120, 40));
                solidrectangle(x, y, x + TILE, y + TILE);

                // Tile
                if (grid[r][c] == CONCRETE)
                {
                    setfillcolor(RGB(160, 160, 160));
                    setlinecolor(RGB(90, 90, 90));
                    solidrectangle(x + 2, y + 2, x + TILE - 2, y + TILE - 2);
                }
                else if (grid[r][c] == BRICK)
                {
                    setfillcolor(RGB(180, 90, 40));
                    setlinecolor(RGB(100, 50, 20));
                    solidrectangle(x + 2, y + 2, x + TILE - 2, y + TILE - 2);
                    // Brick lines
                    setlinecolor(RGB(80, 40, 20));
                    line(x + 2, y + TILE / 2, x + TILE - 2, y + TILE / 2);
                }
            }
        }

        // ===== Draw bombs =====
        for (int i = 0; i < maxBombs; ++i)
        {
            if (!bombs[i].active) continue;
            drawBomb(bombs[i].cx * TILE, bombs[i].cy * TILE + 60, bombs[i].timer);
        }

        // ===== Draw enemies =====
        for (int i = 0; i < maxEnemies; ++i)
        {
            if (!enemies[i].alive) continue;
            drawEnemy(enemies[i].cx * TILE, enemies[i].cy * TILE + 60);
        }

        // ===== Draw player =====
        drawPlayer(playerCX * TILE, playerCY * TILE + 60);

        // ===== Draw flames on top =====
        for (int i = 0; i < maxFlames; ++i)
        {
            if (!flames[i].active) continue;
            drawFlame(flames[i].cx * TILE, flames[i].cy * TILE + 60, flames[i].timer);
        }

        // ===== Overlays =====
        if (!started)
        {
            setfillcolor(RGB(0, 0, 0));
            solidrectangle(100, H / 2 - 80, W - 100, H / 2 + 80);
            settextcolor(RGB(255, 255, 100));
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