#include <SFML/Graphics.hpp>
#include <optional>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <cstdio>

using namespace std;

const int ROWS = 21;
const int COLS = 19;
const float TILE_SIZE = 30.f;
const float TOP_OFFSET = 50.f;
const int WINDOW_WIDTH = COLS * static_cast<int>(TILE_SIZE);
const int WINDOW_HEIGHT = ROWS * static_cast<int>(TILE_SIZE) + static_cast<int>(TOP_OFFSET);

const int PLAYER_SPEED = 8;
const int GHOST_SPEED = 10;
const int FRIGHTENED_GHOST_SPEED = 16;
const int POWERUP_DURATION = 300;
const int NUM_GHOSTS = 4;
const int START_LIVES = 3;

const int PELLET_SCORE = 10;
const int POWER_PELLET_SCORE = 50;
const int GHOST_SCORE = 200;
const int INF_DIST = 100000;

struct player {
    int lives = START_LIVES;
    int score = 0;
    int x = 0;
    int y = 0;
    int direction = 0;
    bool powerUp = false;
    int defaultx = 0, defaulty = 0;
    int px = 0, py = 0;
};

struct ghost {
    int x = 0, y = 0, direction = 0, speed = 0, defaultx = 0, defaulty = 0;
    bool powerdown = false;
    int px = 0, py = 0;
};

struct gameState {
    bool main = true;
    bool pause = false;
    bool win = false, lose = false, playing = false, savegame = false;
    int selected = -1, active = 0;
};

int countPellets(const char maze[ROWS][COLS]) {
    int total = 0;
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (maze[i][j] == '.' || maze[i][j] == 'o') {
                total++;
            }
        }
    }
    return total;
}

void centerText(sf::Text& text, float centerX, float centerY) {
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin({ bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f });
    text.setPosition({ centerX, centerY });
}

void intmap(char arr[ROWS][COLS]) {
    ifstream in("maze.txt");
    if (!in) {
        cerr << "Failed to open maze.txt for reading.\n";
        return;
    }
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (!(in >> arr[i][j])) {
                cout << "File ended early or had invalid data at row "
                    << i << ", col " << j << "\n";
                i = ROWS;
                break;
            }
        }
    }
}

void intlocations(char maze[ROWS][COLS], player& p, ghost g[], int powerPellet[]) {
    int count = 0;
    cout << "MAZE USED IN GAME\n";
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            cout << maze[i][j] << " ";
            if (maze[i][j] == 'P') {
                p.x = j;
                p.y = i;
                p.px = j;
                p.py = i;
                p.defaultx = j;
                p.defaulty = i;
            }
            if (maze[i][j] == 'G' && count < NUM_GHOSTS) {
                g[count].x = j;
                g[count].y = i;
                g[count].px = j;
                g[count].py = i;
                g[count].defaultx = j;
                g[count].defaulty = i;
                g[count].speed = 0;
                g[count].direction = 0;
                g[count].powerdown = false;
                count++;
            }
            if (maze[i][j] == 'o') {
                powerPellet[0] = j;
                powerPellet[1] = i;
            }
        }
        cout << "\n";
    }
}

void menu(sf::RenderWindow& window, gameState& game) {
    while (const optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            window.close();
        else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            if ((key->code == sf::Keyboard::Key::Up || key->code == sf::Keyboard::Key::W) && game.active > 0)
                game.active--;
            else if ((key->code == sf::Keyboard::Key::Down || key->code == sf::Keyboard::Key::S) && game.active < 2)
                game.active++;
            else if (key->code == sf::Keyboard::Key::Enter)
                game.selected = game.active;
        }
    }
}

void inValues(sf::RenderWindow& window, char maze[ROWS][COLS], player& p, ghost g[], gameState& game) {
    if (!game.main) {
        while (const optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
            else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if ((key->code == sf::Keyboard::Key::Up || key->code == sf::Keyboard::Key::W) && p.y > 0 && maze[p.y - 1][p.x] != '#')
                    p.direction = 1;
                else if ((key->code == sf::Keyboard::Key::Down || key->code == sf::Keyboard::Key::S) && p.y < ROWS - 1 && maze[p.y + 1][p.x] != '#')
                    p.direction = 3;
                else if ((key->code == sf::Keyboard::Key::Left || key->code == sf::Keyboard::Key::A) && p.x > 0 && maze[p.y][p.x - 1] != '#')
                    p.direction = 2;
                else if ((key->code == sf::Keyboard::Key::Right || key->code == sf::Keyboard::Key::D) && p.x < COLS - 1 && maze[p.y][p.x + 1] != '#')
                    p.direction = 4;
                else if (key->code == sf::Keyboard::Key::Enter && (game.win || game.lose)) {
                    game.main = true;
                    game.playing = false;
                    game.win = false;
                    game.lose = false;
                    game.pause = false;
                    game.active = 0;
                    game.selected = -1;
                }
                else if (key->code == sf::Keyboard::Key::Escape) {
                    game.main = true;
                    game.playing = false;
                    game.savegame = true;
                    ofstream file("savegame.txt");
                    file << p.score << " " << p.lives << endl << p.x << " " << p.y << endl;
                    for (int i = 0; i < NUM_GHOSTS; i++) {
                        file << g[i].x << " " << g[i].y << " ";
                    }
                    file << endl;
                    for (int i = 0; i < ROWS; i++) {
                        for (int j = 0; j < COLS; j++) {
                            file << maze[i][j];
                        }
                        file << endl;
                    }
                    file.close();
                }
                else if (key->code == sf::Keyboard::Key::P)
                    game.pause = !game.pause;
                else if (key->code == sf::Keyboard::Key::X)
                    window.close();
            }
        }
    }
    else {
        menu(window, game);
    }
}

void playermove(char maze[ROWS][COLS], player& p, int& count) {
    if (count == PLAYER_SPEED) {
        p.px = p.x;
        p.py = p.y;
        if (p.direction) {
            if (p.direction == 1 && p.y > 0 && maze[p.y - 1][p.x] != '#') {
                p.y--;
            }
            else if (p.direction == 2 && p.x > 0 && maze[p.y][p.x - 1] != '#') {
                p.x--;
            }
            else if (p.direction == 3 && p.y < ROWS - 1 && maze[p.y + 1][p.x] != '#') {
                p.y++;
            }
            else if (p.direction == 4 && p.x < COLS - 1 && maze[p.y][p.x + 1] != '#') {
                p.x++;
            }
            else
                p.direction = 0;
        }
        count = 1;
    }
}

void chaser(char maze[ROWS][COLS], player p, ghost& g, int& count) {
    int opposite = -1, current = g.direction;
    if (current == 1) opposite = 3;
    if (current == 2) opposite = 4;
    if (current == 3) opposite = 1;
    if (current == 4) opposite = 2;

    if (count == GHOST_SPEED) {
        g.px = g.x;
        g.py = g.y;
        int dists[4] = { INF_DIST, INF_DIST, INF_DIST, INF_DIST };
        int dirs[4] = { 1, 2, 3, 4 };

        if (g.y > 0 && maze[g.y - 1][g.x] != '#') {
            int dx = g.x - p.x;
            int dy = (g.y - 1) - p.y;
            if (dx < 0) dx = -dx;
            if (dy < 0) dy = -dy;
            dists[0] = dx + dy;
        }
        if (g.x > 0 && maze[g.y][g.x - 1] != '#') {
            int dx = (g.x - 1) - p.x;
            int dy = g.y - p.y;
            if (dx < 0) dx = -dx;
            if (dy < 0) dy = -dy;
            dists[1] = dx + dy;
        }
        if (g.y < ROWS - 1 && maze[g.y + 1][g.x] != '#') {
            int dx = g.x - p.x;
            int dy = (g.y + 1) - p.y;
            if (dx < 0) dx = -dx;
            if (dy < 0) dy = -dy;
            dists[2] = dx + dy;
        }
        if (g.x < COLS - 1 && maze[g.y][g.x + 1] != '#') {
            int dx = (g.x + 1) - p.x;
            int dy = g.y - p.y;
            if (dx < 0) dx = -dx;
            if (dy < 0) dy = -dy;
            dists[3] = dx + dy;
        }

        for (int k = 0; k < 4; k++) {
            if (opposite == dirs[k])
                dists[k] = INF_DIST;
        }
        int bestDist = dists[0];
        int bestDir = dirs[0];
        for (int i = 1; i < 4; i++) {
            if (dists[i] < bestDist) {
                bestDist = dists[i];
                bestDir = dirs[i];
            }
        }

        if (bestDist < INF_DIST) {
            g.direction = bestDir;
            if (g.direction == 1)      g.y--;
            else if (g.direction == 2) g.x--;
            else if (g.direction == 3) g.y++;
            else if (g.direction == 4) g.x++;
        }

        count = 1;
    }
}

void wanderers(char maze[ROWS][COLS], ghost& g, int& count) {
    int dirs[4] = { 1, 2, 3, 4 }, temp = 0, current = g.direction, opposite = -1;
    bool move[4] = { 0, 0, 0, 0 };
    if (current == 1) opposite = 3;
    if (current == 2) opposite = 4;
    if (current == 3) opposite = 1;
    if (current == 4) opposite = 2;

    if (count == GHOST_SPEED) {
        g.px = g.x;
        g.py = g.y;
        if (g.y > 0 && maze[g.y - 1][g.x] != '#') { temp++; move[0] = 1; }
        if (g.x > 0 && maze[g.y][g.x - 1] != '#') { temp++; move[1] = 1; }
        if (g.y < ROWS - 1 && maze[g.y + 1][g.x] != '#') { temp++; move[2] = 1; }
        if (g.x < COLS - 1 && maze[g.y][g.x + 1] != '#') { temp++; move[3] = 1; }

        if (opposite != -1 && temp > 1 && move[opposite - 1]) {
            move[opposite - 1] = 0;
            temp--;
        }

        g.direction = 0;
        if (temp > 0) {
            int pick = rand() % temp;
            int seen = 0;
            for (int j = 0; j < 4; j++) {
                if (move[j]) {
                    if (seen == pick) {
                        g.direction = dirs[j];
                        break;
                    }
                    seen++;
                }
            }
        }

        if (g.direction == 1)      g.y--;
        else if (g.direction == 2) g.x--;
        else if (g.direction == 3) g.y++;
        else if (g.direction == 4) g.x++;

        count = 1;
    }
}

void ghostf(char maze[ROWS][COLS], ghost& g, int& count) {
    int dirs[4] = { 1, 2, 3, 4 }, temp = 0, current = g.direction, opposite = -1;
    bool move[4] = { 0, 0, 0, 0 };
    if (current == 1) opposite = 3;
    if (current == 2) opposite = 4;
    if (current == 3) opposite = 1;
    if (current == 4) opposite = 2;

    if (count == FRIGHTENED_GHOST_SPEED) {
        g.px = g.x;
        g.py = g.y;
        if (g.y > 0 && maze[g.y - 1][g.x] != '#') { temp++; move[0] = 1; }
        if (g.x > 0 && maze[g.y][g.x - 1] != '#') { temp++; move[1] = 1; }
        if (g.y < ROWS - 1 && maze[g.y + 1][g.x] != '#') { temp++; move[2] = 1; }
        if (g.x < COLS - 1 && maze[g.y][g.x + 1] != '#') { temp++; move[3] = 1; }

        if (opposite != -1 && temp > 1 && move[opposite - 1]) {
            move[opposite - 1] = 0;
            temp--;
        }

        g.direction = 0;
        if (temp > 0) {
            int pick = rand() % temp;
            int seen = 0;
            for (int j = 0; j < 4; j++) {
                if (move[j]) {
                    if (seen == pick) {
                        g.direction = dirs[j];
                        break;
                    }
                    seen++;
                }
            }
        }

        if (g.direction == 1)      g.y--;
        else if (g.direction == 2) g.x--;
        else if (g.direction == 3) g.y++;
        else if (g.direction == 4) g.x++;

        count = 1;
    }
}

void update(char maze[ROWS][COLS], ghost g[], player& p, int& pellets, int& pspeed, int& powercount, gameState& game) {
    playermove(maze, p, pspeed);

    for (int i = 0; i < NUM_GHOSTS; i++) {
        if (g[i].powerdown)
            ghostf(maze, g[i], g[i].speed);
        else {
            if (i == 0 || i == 2) chaser(maze, p, g[i], g[i].speed);
            if (i == 1 || i == 3) wanderers(maze, g[i], g[i].speed);
        }
    }

    if (maze[p.y][p.x] == '.') {
        p.score += PELLET_SCORE;
        maze[p.y][p.x] = '-';
        pellets--;
    }
    if (maze[p.y][p.x] == 'o') {
        p.score += POWER_PELLET_SCORE;
        maze[p.y][p.x] = '-';
        pellets--;
        p.powerUp = true;
        powercount = 0;
        for (int i = 0; i < NUM_GHOSTS; i++) {
            g[i].powerdown = true;
        }
    }

    for (int k = 0; k < NUM_GHOSTS; k++) {
        if (((p.y == g[k].y && p.x == g[k].x) || (p.x == g[k].px && p.y == g[k].py && g[k].x == p.px && g[k].y == p.py)) && !g[k].powerdown) {
            p.lives--;
            p.x = p.defaultx;
            p.y = p.defaulty;
            p.px = p.defaultx;
            p.py = p.defaulty;
            p.direction = 0;
            p.powerUp = false;
            powercount = 0;
            for (int m = 0; m < NUM_GHOSTS; m++) {
                g[m].x = g[m].defaultx;
                g[m].y = g[m].defaulty;
                g[m].px = g[m].defaultx;
                g[m].py = g[m].defaulty;
                g[m].powerdown = false;
                g[m].direction = 0;
                g[m].speed = 0;
            }
            break;
        }
    }

    if (p.powerUp) {
        if (powercount == POWERUP_DURATION) {
            p.powerUp = false;
            for (int k = 0; k < NUM_GHOSTS; k++) {
                g[k].powerdown = false;
                g[k].speed = 0;
            }
        }
        powercount++;
        for (int k = 0; k < NUM_GHOSTS; k++) {
            if (((p.y == g[k].y && p.x == g[k].x) || (p.x == g[k].px && p.y == g[k].py && g[k].x == p.px && g[k].y == p.py)) && g[k].powerdown) {
                p.score += GHOST_SCORE;
                g[k].x = g[k].defaultx;
                g[k].y = g[k].defaulty;
                g[k].px = g[k].defaultx;
                g[k].py = g[k].defaulty;
                g[k].powerdown = false;
                g[k].direction = 0;
                g[k].speed = 0;
            }
        }
    }

    pspeed++;
    for (int k = 0; k < NUM_GHOSTS; k++)
        g[k].speed++;

    if (pellets == 0) {
        game.win = true;
    }
    if (p.lives <= 0) {
        game.lose = true;
    }
    if (game.win || game.lose) {
        game.savegame = false;
        remove("savegame.txt");
    }
}

void draw(sf::RenderWindow& window, char maze[ROWS][COLS], ghost g[], player& p, const sf::Font& font, int& pellets, int& pspeed, int& powercount, gameState& game, int powerPellet[]) {
    const float winW = static_cast<float>(WINDOW_WIDTH);
    const float winH = static_cast<float>(WINDOW_HEIGHT);
    const float centerX = winW / 2.f;

    const float pelletRadius = TILE_SIZE * 0.10f;
    const float powerRadius = TILE_SIZE * 0.20f;
    const float entityRadius = TILE_SIZE * 0.40f;
    const float entityInset = TILE_SIZE * 0.10f;
    const float pelletInset = TILE_SIZE * 0.40f;
    const float powerInset = TILE_SIZE * 0.30f;

    window.clear(sf::Color::Black);

    sf::RectangleShape wallBlock({ TILE_SIZE, TILE_SIZE });
    wallBlock.setFillColor(sf::Color(33, 33, 222));

    sf::CircleShape pellet(pelletRadius);
    pellet.setFillColor(sf::Color::White);

    sf::CircleShape powerPelletShape(powerRadius);
    powerPelletShape.setFillColor(sf::Color::Red);

    sf::CircleShape ghost1Shape(entityRadius); ghost1Shape.setFillColor(sf::Color::Cyan);
    sf::CircleShape ghost2Shape(entityRadius); ghost2Shape.setFillColor(sf::Color::Red);
    sf::CircleShape ghost3Shape(entityRadius); ghost3Shape.setFillColor(sf::Color::Green);
    sf::CircleShape ghost4Shape(entityRadius); ghost4Shape.setFillColor(sf::Color::Magenta);
    sf::CircleShape ghostfShape(entityRadius); ghostfShape.setFillColor(sf::Color::Blue);

    sf::CircleShape playerShape(entityRadius);
    playerShape.setFillColor(sf::Color::Yellow);

    if (!game.playing) {
        sf::RectangleShape menuBorder({ winW - 60.f, winH - 60.f });
        menuBorder.setFillColor(sf::Color::Transparent);
        menuBorder.setOutlineThickness(4.f);
        menuBorder.setOutlineColor(sf::Color(33, 33, 222));
        menuBorder.setPosition({ 30.f, 30.f });
        window.draw(menuBorder);

        sf::Text menuTitle(font, "PAC-MAN", 56);
        menuTitle.setFillColor(sf::Color::Yellow);
        centerText(menuTitle, centerX, 140.f);
        window.draw(menuTitle);

        sf::Color previewColors[NUM_GHOSTS] = { sf::Color::Cyan, sf::Color::Red, sf::Color::Green, sf::Color::Magenta };
        float previewStartX = centerX - 100.f;
        for (int i = 0; i < NUM_GHOSTS; i++) {
            sf::CircleShape previewGhost(14.f);
            previewGhost.setFillColor(previewColors[i]);
            previewGhost.setPosition({ previewStartX + i * 55.f, 210.f });
            window.draw(previewGhost);
        }

        sf::Text menuOption[3] = {
            sf::Text(font, game.savegame ? "CONTINUE" : "NO SAVED GAME", 26),
            sf::Text(font, "NEW GAME", 26),
            sf::Text(font, "EXIT", 26)
        };

        for (int k = 0; k < 3; k++) {
            menuOption[k].setFillColor(game.active == k ? sf::Color::Yellow : sf::Color::White);
            centerText(menuOption[k], centerX, 315.f + k * 55.f);
            window.draw(menuOption[k]);
        }

        sf::Text menuHint(font, "UP/DOWN TO SELECT   |   ENTER TO CONFIRM", 14);
        menuHint.setFillColor(sf::Color(180, 180, 180));
        centerText(menuHint, centerX, winH - 125.f);
        window.draw(menuHint);
    }

    switch (game.selected) {
    case 0:
        if (game.savegame) {
            ifstream file("savegame.txt");
            if (file.is_open() && (file >> p.score >> p.lives >> p.x >> p.y)) {
                p.px = p.x;
                p.py = p.y;
                p.direction = 0;
                p.powerUp = false;
                for (int i = 0; i < NUM_GHOSTS; i++) {
                    file >> g[i].x >> g[i].y;
                    g[i].px = g[i].x;
                    g[i].py = g[i].y;
                    g[i].speed = 0;
                    g[i].direction = 0;
                    g[i].powerdown = false;
                }
                for (int i = 0; i < ROWS; i++) {
                    for (int j = 0; j < COLS; j++) {
                        file >> maze[i][j];
                        if (maze[i][j] == 'P' || maze[i][j] == 'G') {
                            maze[i][j] = '-';
                        }
                    }
                }
                pspeed = 0;
                powercount = 0;
                pellets = countPellets(maze);
                game.main = false;
                game.playing = true;
                game.pause = false;
                game.win = false;
                game.lose = false;
            }
            else {
                cout << "WRONG FORMAT OF SAVEGAME FILE" << endl;
                window.close();
            }
        }
        game.selected = -1;
        game.active = 0;
        break;
    case 1:
        intmap(maze);
        intlocations(maze, p, g, powerPellet);
        pellets = countPellets(maze);
        p.lives = START_LIVES;
        p.score = 0;
        p.direction = 0;
        p.powerUp = false;
        pspeed = 0;
        powercount = 0;
        game.main = false;
        game.playing = true;
        game.pause = false;
        game.win = false;
        game.lose = false;
        game.selected = -1;
        game.active = 0;
        break;
    case 2:
        game.selected = -1;
        game.active = 0;
        window.close();
        break;
    }

    if (game.playing) {
        sf::RectangleShape divider({ winW, 2.f });
        divider.setFillColor(sf::Color(33, 33, 222));
        divider.setPosition({ 0.f, TOP_OFFSET - 2.f });
        window.draw(divider);

        sf::Text scoreText(font, (game.lose || game.win) ? "FINAL SCORE:" + to_string(p.score) : "SCORE: " + to_string(p.score), 20);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition({ 15.f, 12.f });
        window.draw(scoreText);

        sf::Text livesText(font, "LIVES: " + to_string(p.lives), 20);
        livesText.setFillColor(sf::Color::Yellow);
        livesText.setPosition({ winW - 120.f, 12.f });
        window.draw(livesText);

        if (!(game.win || game.lose)) {
            sf::Text pelletsText(font, "PELLETS: " + to_string(pellets), 15);
            pelletsText.setFillColor(sf::Color::Cyan);
            pelletsText.setPosition({ centerX - 45.f, 5.f });
            window.draw(pelletsText);
        }

        if (game.pause && !(game.win || game.lose)) {
            sf::Text pausedText(font, "PAUSED", 15);
            pausedText.setFillColor(sf::Color::Red);
            pausedText.setPosition({ centerX - 45.f, 20.f });
            window.draw(pausedText);
        }

        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                float xPos = j * TILE_SIZE;
                float yPos = i * TILE_SIZE + TOP_OFFSET;

                if (maze[i][j] == '#') {
                    wallBlock.setPosition({ xPos, yPos });
                    window.draw(wallBlock);
                }
                if (maze[i][j] == '.') {
                    pellet.setPosition({ xPos + pelletInset, yPos + pelletInset });
                    window.draw(pellet);
                }
                if (maze[i][j] == 'o') {
                    powerPelletShape.setPosition({ xPos + powerInset, yPos + powerInset });
                    window.draw(powerPelletShape);
                }
                for (int k = 0; k < NUM_GHOSTS; k++) {
                    if (i == g[k].y && j == g[k].x && g[k].powerdown) {
                        ghostfShape.setPosition({ xPos + entityInset, yPos + entityInset });
                        window.draw(ghostfShape);
                    }
                    else if (i == g[k].y && j == g[k].x) {
                        switch (k) {
                        case 0: ghost1Shape.setPosition({ xPos + entityInset, yPos + entityInset }); window.draw(ghost1Shape); break;
                        case 1: ghost2Shape.setPosition({ xPos + entityInset, yPos + entityInset }); window.draw(ghost2Shape); break;
                        case 2: ghost3Shape.setPosition({ xPos + entityInset, yPos + entityInset }); window.draw(ghost3Shape); break;
                        case 3: ghost4Shape.setPosition({ xPos + entityInset, yPos + entityInset }); window.draw(ghost4Shape); break;
                        }
                    }
                }

                if (i == p.y && j == p.x) {
                    playerShape.setPosition({ xPos + entityInset, yPos + entityInset });
                    window.draw(playerShape);
                }
            }
        }

        if (game.win || game.lose) {
            sf::RectangleShape backdrop({ winW, winH });
            backdrop.setFillColor(sf::Color(0, 0, 0, 165));
            window.draw(backdrop);

            const float popupW = 360.f;
            const float popupH = 165.f;
            const float popupX = centerX - popupW / 2.f;
            const float popupY = TOP_OFFSET + (ROWS * TILE_SIZE) / 2.f - popupH / 2.f;

            sf::RectangleShape popup({ popupW, popupH });
            popup.setFillColor(sf::Color(15, 15, 35));
            popup.setOutlineThickness(3.f);
            popup.setOutlineColor(game.win ? sf::Color::Green : sf::Color::Red);
            popup.setPosition({ popupX, popupY });
            window.draw(popup);

            sf::Text statusText(font, game.win ? "YOU WIN!" : "YOU LOSE!", 26);
            statusText.setFillColor(game.win ? sf::Color::Green : sf::Color::Red);
            centerText(statusText, centerX, popupY + 37.f);
            window.draw(statusText);

            sf::Text finalScoreText(font, "FINAL SCORE:" + to_string(p.score), 20);
            finalScoreText.setFillColor(sf::Color::White);
            centerText(finalScoreText, centerX, popupY + 80.f);
            window.draw(finalScoreText);

            sf::Text noteText(font, "Press Enter to Main Menu!", 16);
            noteText.setFillColor(sf::Color::Yellow);
            centerText(noteText, centerX, popupY + 125.f);
            window.draw(noteText);
        }
    }

    window.display();
}

int main() {
    sf::Font font;
    if (!font.openFromFile("arial.ttf")) {
        cout << "Failed to load font\n";
    }
    srand(static_cast<unsigned int>(time(0)));

    player p;
    ghost g[NUM_GHOSTS];
    int powerPellet[2] = { 0, 0 };
    char maze[ROWS][COLS];
    int pspeed = 0;
    int powercount = 0;

    gameState game;
    game.pause = false;
    game.main = true;
    game.lose = false;
    game.win = false;
    game.playing = false;
    game.selected = -1;
    game.active = 0;

    ifstream in("savegame.txt");
    game.savegame = static_cast<bool>(in);
    in.close();

    intmap(maze);
    intlocations(maze, p, g, powerPellet);
    int pellets = countPellets(maze);

    sf::RenderWindow window(sf::VideoMode({ WINDOW_WIDTH, WINDOW_HEIGHT }), "Pac-Man");
    window.setFramerateLimit(60);

    while (window.isOpen()) {
        inValues(window, maze, p, g, game);

        if (!(game.pause || !game.playing)) {
            if (!(game.win || game.lose))
                update(maze, g, p, pellets, pspeed, powercount, game);
        }

        draw(window, maze, g, p, font, pellets, pspeed, powercount, game, powerPellet);
    }

    return 0;
}