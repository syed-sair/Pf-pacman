#include <SFML/Graphics.hpp>
#include <optional>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>

const int r = 21; // number of rows
const int c = 19; // number of columns
using namespace std;

struct player {
    int lives = 3;
    int score = 0;
    int x = 0; // starting x position
    int y = 0; // starting y position
    int direction = 0;
    bool powerUp = false; // power-up flag
    int defaultx = 0, defaulty = 0; // default x and y positions
    int px = 0, py = 0; // previous x and y positions
};

struct ghost {
    int x = 0, y = 0, direction = 0, speed = 0, defaultx = 0, defaulty = 0;
    bool powerdown = false; // active flag
    int px = 0, py = 0; // previous x and y positions
};

struct gameState {
    bool main = true;
    bool pause = false;
    bool win = false, lose = false, playing = false;
    int selected = -1, active = 0;
};

int countPellets(const char maze[r][c]) {
    int total = 0;
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++) {
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

void intmap(char arr[r][c]) {
    ifstream in("maze.txt");
    if (!in) {
        std::cerr << "Failed to open maze.txt for reading.\n";
        return;
    }
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++) {
            if (!(in >> arr[i][j])) {
                cout << "File ended early or had invalid data at row "
                    << i << ", col " << j << "\n";
                i = r; // stop outer loop
                break;    // stop inner loop
            }
        }
    }
}

void intlocations(char maze[r][c], player& p, ghost g[], int powerPellet[]) {
    int count = 0;
    cout << "MAZE USED IN GAME\n";
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++) {
            cout << maze[i][j] << " ";
            if (maze[i][j] == 'P') {
                p.x = j;
                p.y = i;
                p.px = j;
                p.py = i;
                p.defaultx = j;
                p.defaulty = i;
            }
            if (maze[i][j] == 'G') {
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
    while (const std::optional event = window.pollEvent()) {
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

void inValues(sf::RenderWindow& window, char maze[r][c], player& p, gameState& game) {
    if (!game.main) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
            else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if ((key->code == sf::Keyboard::Key::Up || key->code == sf::Keyboard::Key::W) && maze[p.y - 1][p.x] != '#')
                    p.direction = 1;
                else if ((key->code == sf::Keyboard::Key::Down || key->code == sf::Keyboard::Key::S) && maze[p.y + 1][p.x] != '#')
                    p.direction = 3;
                else if ((key->code == sf::Keyboard::Key::Left || key->code == sf::Keyboard::Key::A) && maze[p.y][p.x - 1] != '#')
                    p.direction = 2;
                else if ((key->code == sf::Keyboard::Key::Right || key->code == sf::Keyboard::Key::D) && maze[p.y][p.x + 1] != '#')
                    p.direction = 4;
                else if ((key->code == sf::Keyboard::Key::Enter) && (game.win || game.lose)) {
                    game.playing = false;
                    game.main = true;
                    game.win = false;
                    game.lose = false;
                    game.pause = false;
                    game.active = 0;
                    game.selected = -1;
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

void playermove(char maze[r][c], player& p, int& count) {
    if (count == 8) {
        p.px = p.x;
        p.py = p.y;
        if (p.direction) {
            if (p.direction == 1 && maze[p.y - 1][p.x] != '#') {
                p.y--;
            }
            else if (p.direction == 2 && maze[p.y][p.x - 1] != '#') {
                p.x--;
            }
            else if (p.direction == 3 && maze[p.y + 1][p.x] != '#') {
                p.y++;
            }
            else if (p.direction == 4 && maze[p.y][p.x + 1] != '#') {
                p.x++;
            }
            else
                p.direction = 0;
        }
        count = 1;
    }
}

void chaser(char maze[r][c], player p, ghost& g, int& count) {
    int opposite = -1, current = g.direction;
    if (current == 1) opposite = 3;
    if (current == 2) opposite = 4;
    if (current == 3) opposite = 1;
    if (current == 4) opposite = 2;
    if (count == 10) {
        g.px = g.x;
        g.py = g.y;
        int dists[4] = { 100000, 100000, 100000, 100000 };
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
        if (g.y < r - 1 && maze[g.y + 1][g.x] != '#') {
            int dx = g.x - p.x;
            int dy = (g.y + 1) - p.y;
            if (dx < 0) dx = -dx;
            if (dy < 0) dy = -dy;
            dists[2] = dx + dy;
        }
        if (g.x < c - 1 && maze[g.y][g.x + 1] != '#') {
            int dx = (g.x + 1) - p.x;
            int dy = g.y - p.y;
            if (dx < 0) dx = -dx;
            if (dy < 0) dy = -dy;
            dists[3] = dx + dy;
        }

        for (int k = 0; k < 4; k++) {
            if (opposite == dirs[k])
                dists[k] = 100000;
        }
        int bestDist = dists[0];
        int bestDir = dirs[0];
        for (int i = 1; i < 4; i++) {
            if (dists[i] < bestDist) {
                bestDist = dists[i];
                bestDir = dirs[i];
            }
        }

        if (bestDist < 100000) {
            g.direction = bestDir;
            if (g.direction == 1)      g.y--;
            else if (g.direction == 2) g.x--;
            else if (g.direction == 3) g.y++;
            else if (g.direction == 4) g.x++;
        }

        count = 1;
    }
}

void wanderers(char maze[r][c], ghost& g, int& count) {
    int dirs[4] = { 1, 2, 3, 4 }, temp = 0, current = g.direction, opposite = -1;
    bool move[4] = { 0, 0, 0, 0 };
    if (current == 1) opposite = 3;
    if (current == 2) opposite = 4;
    if (current == 3) opposite = 1;
    if (current == 4) opposite = 2;

    if (count == 10) {
        g.px = g.x;
        g.py = g.y;
        if (g.y > 0 && maze[g.y - 1][g.x] != '#') { temp++; move[0] = 1; }
        if (g.x > 0 && maze[g.y][g.x - 1] != '#') { temp++; move[1] = 1; }
        if (g.y < r - 1 && maze[g.y + 1][g.x] != '#') { temp++; move[2] = 1; }
        if (g.x < c - 1 && maze[g.y][g.x + 1] != '#') { temp++; move[3] = 1; }

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

void ghostf(char maze[r][c], ghost& g, int& count) {
    int dirs[4] = { 1, 2, 3, 4 }, temp = 0, current = g.direction, opposite = -1;
    bool move[4] = { 0, 0, 0, 0 };
    if (current == 1) opposite = 3;
    if (current == 2) opposite = 4;
    if (current == 3) opposite = 1;
    if (current == 4) opposite = 2;

    if (count == 16) {
        g.px = g.x;
        g.py = g.y;
        if (g.y > 0 && maze[g.y - 1][g.x] != '#') { temp++; move[0] = 1; }
        if (g.x > 0 && maze[g.y][g.x - 1] != '#') { temp++; move[1] = 1; }
        if (g.y < r - 1 && maze[g.y + 1][g.x] != '#') { temp++; move[2] = 1; }
        if (g.x < c - 1 && maze[g.y][g.x + 1] != '#') { temp++; move[3] = 1; }

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

void update(char maze[r][c], ghost g[], player& p, int& pellets, int& pspeed, int& powercount, gameState& game) {
    playermove(maze, p, pspeed);

    for (int i = 0; i < 4; i++) {
        if (g[i].powerdown)
            ghostf(maze, g[i], g[i].speed);
        else {
            if (i == 0) chaser(maze, p, g[0], g[0].speed);
            if (i == 2) chaser(maze, p, g[2], g[2].speed);
            if (i == 1) wanderers(maze, g[1], g[1].speed);
            if (i == 3) wanderers(maze, g[3], g[3].speed);
        }
    }

    if (maze[p.y][p.x] == '.') {
        p.score += 10;
        maze[p.y][p.x] = '-';
        pellets--;
    }
    if (maze[p.y][p.x] == 'o') {
        p.score += 50;
        maze[p.y][p.x] = '-';
        pellets--;
        p.powerUp = true;
        powercount = 0;
        for (int i = 0; i < 4; i++) {
            g[i].powerdown = true;
        }
    }

    for (int k = 0; k < 4; k++) {
        if (((p.y == g[k].y && p.x == g[k].x) || (p.x == g[k].px && p.y == g[k].py && g[k].x == p.px && g[k].y == p.py)) && !g[k].powerdown) {
            p.lives--;
            p.x = p.defaultx;
            p.y = p.defaulty;
            p.px = p.defaultx;
            p.py = p.defaulty;
            p.direction = 0;
            p.powerUp = false;
            powercount = 0;
            for (int m = 0; m < 4; m++) {
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
        if (powercount == 300) {
            p.powerUp = false;
            for (int k = 0; k < 4; k++) {
                g[k].powerdown = false;
                g[k].speed = 0;
            }
        }
        powercount++;
        for (int k = 0; k < 4; k++) {
            if (((p.y == g[k].y && p.x == g[k].x) || (p.x == g[k].px && p.y == g[k].py && g[k].x == p.px && g[k].y == p.py)) && g[k].powerdown) {
                p.score += 200;
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
    for (int k = 0; k < 4; k++)
        g[k].speed++;

    if (pellets == 0) {
        game.win = true;
    }
    if (p.lives == 0) {
        game.lose = true;
    }
}

void draw(sf::RenderWindow& window, char maze[r][c], ghost g[], player& p, const sf::Font& font, int& pellets, int& pspeed, int& powercount, gameState& game, int powerPellet[]) {
    const float TILE = 30.f;
    const float TOP_OFFSET = 50.f;

    window.clear(sf::Color::Black);

    sf::RectangleShape wallBlock({ TILE, TILE });
    wallBlock.setFillColor(sf::Color(33, 33, 222));

    sf::CircleShape pellet(3.f);
    pellet.setFillColor(sf::Color::White);

    sf::CircleShape powerPelletShape(6.f);
    powerPelletShape.setFillColor(sf::Color::Red);

    sf::CircleShape ghost1Shape(12.f); ghost1Shape.setFillColor(sf::Color::Cyan);
    sf::CircleShape ghost2Shape(12.f); ghost2Shape.setFillColor(sf::Color::Red);
    sf::CircleShape ghost3Shape(12.f); ghost3Shape.setFillColor(sf::Color::Green);
    sf::CircleShape ghost4Shape(12.f); ghost4Shape.setFillColor(sf::Color::Magenta);
    sf::CircleShape ghostfShape(12.f); ghostfShape.setFillColor(sf::Color::Blue);

    sf::CircleShape playerShape(12.f);
    playerShape.setFillColor(sf::Color::Yellow);

    if (!game.playing) {
        sf::RectangleShape menuBorder({ 510.f, 620.f });
        menuBorder.setFillColor(sf::Color::Transparent);
        menuBorder.setOutlineThickness(4.f);
        menuBorder.setOutlineColor(sf::Color(33, 33, 222));
        menuBorder.setPosition({ 30.f, 30.f });
        window.draw(menuBorder);

        sf::Text menuTitle(font, "PAC-MAN", 56);
        menuTitle.setFillColor(sf::Color::Yellow);
        centerText(menuTitle, 285.f, 140.f);
        window.draw(menuTitle);

        sf::Color previewColors[4] = { sf::Color::Cyan, sf::Color::Red, sf::Color::Green, sf::Color::Magenta };
        for (int i = 0; i < 4; i++) {
            sf::CircleShape previewGhost(14.f);
            previewGhost.setFillColor(previewColors[i]);
            previewGhost.setPosition({ 185.f + i * 55.f, 210.f });
            window.draw(previewGhost);
        }

        sf::Text menuOption[3] = {
            sf::Text(font, "CONTINUE", 26),
            sf::Text(font, "NEW GAME", 26),
            sf::Text(font, "EXIT", 26)
        };

        for (int k = 0; k < 3; k++) {
            menuOption[k].setFillColor(game.active == k ? sf::Color::Yellow : sf::Color::White);
            centerText(menuOption[k], 285.f, 315.f + k * 55.f);
            window.draw(menuOption[k]);
        }

        sf::Text menuHint(font, "UP/DOWN TO SELECT   |   ENTER TO CONFIRM", 14);
        menuHint.setFillColor(sf::Color(180, 180, 180));
        centerText(menuHint, 285.f, 555.f);
        window.draw(menuHint);
    }

    switch (game.selected) {
    case 0:
        game.main = false;
        game.playing = true;
        game.selected = -1;
        break;
    case 1:
        intmap(maze);
        intlocations(maze, p, g, powerPellet);
        pellets = countPellets(maze);
        p.lives = 3;
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
        window.close();
        break;
    }

    if (game.playing) {
        sf::RectangleShape divider({ 570.f, 2.f });
        divider.setFillColor(sf::Color(33, 33, 222));
        divider.setPosition({ 0.f, TOP_OFFSET - 2.f });
        window.draw(divider);

        sf::Text scoreText(font, (game.lose || game.win) ? "FINAL SCORE:" + to_string(p.score) : "SCORE: " + to_string(p.score), 20);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition({ 15.f, 12.f });
        window.draw(scoreText);

        sf::Text livesText(font, "LIVES: " + to_string(p.lives), 20);
        livesText.setFillColor(sf::Color::Yellow);
        livesText.setPosition({ 450.f, 12.f });
        window.draw(livesText);

        if (!(game.win || game.lose)) {
            sf::Text pelletsText(font, "PELLETS: " + to_string(pellets), 15);
            pelletsText.setFillColor(sf::Color::Cyan);
            pelletsText.setPosition({ 240.f, 5.f });
            window.draw(pelletsText);
        }

        if (game.pause && !(game.win || game.lose)) {
            sf::Text pausedText(font, "PAUSED", 15);
            pausedText.setFillColor(sf::Color::Red);
            pausedText.setPosition({ 240.f, 20.f });
            window.draw(pausedText);
        }

        for (int i = 0; i < r; i++) {
            for (int j = 0; j < c; j++) {
                float xPos = j * TILE;
                float yPos = i * TILE + TOP_OFFSET;

                if (maze[i][j] == '#') {
                    wallBlock.setPosition({ xPos, yPos });
                    window.draw(wallBlock);
                }
                if (maze[i][j] == '.') {
                    pellet.setPosition({ xPos + 12.f, yPos + 12.f });
                    window.draw(pellet);
                }
                if (maze[i][j] == 'o') {
                    powerPelletShape.setPosition({ xPos + 9.f, yPos + 9.f });
                    window.draw(powerPelletShape);
                }
                for (int k = 0; k < 4; k++) {
                    if (i == g[k].y && j == g[k].x && g[k].powerdown) {
                        ghostfShape.setPosition({ xPos + 3.f, yPos + 3.f });
                        window.draw(ghostfShape);
                    }
                    else if (i == g[k].y && j == g[k].x) {
                        switch (k) {
                        case 0: ghost1Shape.setPosition({ xPos + 3.f, yPos + 3.f }); window.draw(ghost1Shape); break;
                        case 1: ghost2Shape.setPosition({ xPos + 3.f, yPos + 3.f }); window.draw(ghost2Shape); break;
                        case 2: ghost3Shape.setPosition({ xPos + 3.f, yPos + 3.f }); window.draw(ghost3Shape); break;
                        case 3: ghost4Shape.setPosition({ xPos + 3.f, yPos + 3.f }); window.draw(ghost4Shape); break;
                        }
                    }
                }

                if (i == p.y && j == p.x) {
                    playerShape.setPosition({ xPos + 3.f, yPos + 3.f });
                    window.draw(playerShape);
                }
            }
        }

        // Small pop-up rectangle for WIN / LOSE
        if (game.win || game.lose) {
            sf::RectangleShape backdrop({ 570.f, 680.f });
            backdrop.setFillColor(sf::Color(0, 0, 0, 165));
            window.draw(backdrop);

            sf::RectangleShape popup({ 360.f, 165.f });
            popup.setFillColor(sf::Color(15, 15, 35));
            popup.setOutlineThickness(3.f);
            popup.setOutlineColor(game.win ? sf::Color::Green : sf::Color::Red);
            popup.setPosition({ 105.f, 255.f });
            window.draw(popup);

            sf::Text statusText(font, game.win ? "YOU WIN!" : "YOU LOSE!", 26);
            statusText.setFillColor(game.win ? sf::Color::Green : sf::Color::Red);
            centerText(statusText, 285.f, 292.f);
            window.draw(statusText);

            sf::Text finalScoreText(font, "FINAL SCORE:" + to_string(p.score), 20);
            finalScoreText.setFillColor(sf::Color::White);
            centerText(finalScoreText, 285.f, 335.f);
            window.draw(finalScoreText);

            sf::Text noteText(font, "Press Enter to Main Menu!", 16);
            noteText.setFillColor(sf::Color::Yellow);
            centerText(noteText, 285.f, 380.f);
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
    ghost g[4];
    int powerPellet[2] = { 0, 0 };
    char maze[r][c];
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

    intmap(maze);
    intlocations(maze, p, g, powerPellet);
    int pellets = countPellets(maze);

    sf::RenderWindow window(sf::VideoMode({ 570, 680 }), "Pac-Man");
    window.setFramerateLimit(60);

    while (window.isOpen()) {
        inValues(window, maze, p, game);

        if (!(game.pause || !game.playing)) {
            if (!(game.win || game.lose))
                update(maze, g, p, pellets, pspeed, powercount, game);
        }

        draw(window, maze, g, p, font, pellets, pspeed, powercount, game, powerPellet);
    }

    return 0;
}