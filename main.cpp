#include <SFML/Graphics.hpp>
#include <optional>
#include<iostream>
#include<fstream>
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
};
struct ghost {
    int x, y, direction=0,speed=0;
};
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
void inValues(sf::RenderWindow& window, char maze[r][c], player& p, bool& paused) {
    while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            window.close();
        else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            if ((key->code == sf::Keyboard::Key::Up || key->code == sf::Keyboard::Key::W) && maze[p.y - 1][p.x] != '#')
                p.direction = 1;
            else if ((key->code == sf::Keyboard::Key::Down || key->code == sf::Keyboard::Key::S) && maze[p.y + 1][p.x] != '#')
                p.direction = 2;
            else if ((key->code == sf::Keyboard::Key::Left || key->code == sf::Keyboard::Key::A) && maze[p.y][p.x - 1] != '#')
                p.direction = 3;
            else if ((key->code == sf::Keyboard::Key::Right || key->code == sf::Keyboard::Key::D) && maze[p.y][p.x + 1] != '#')
                p.direction = 4;
            else if (key->code == sf::Keyboard::Key::P)
                paused = !paused;
            else if (key->code == sf::Keyboard::Key::X)
                window.close();
        }
    }
}
void playermove(char maze[r][c],player& p,int& count) {
    if (count == 8)
    {
        if (p.direction) {
            if (p.direction == 1 && maze[p.y - 1][p.x] != '#') {
                p.y--;
            }
            else if (p.direction == 2 && maze[p.y + 1][p.x] != '#') {
                p.y++;
            }
            else if (p.direction == 3 && maze[p.y][p.x - 1] != '#') {
                p.x--;
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
    //opposite used for avoid reversing
    int opposite=-1,current = g.direction;
    if (current == 1)opposite = 3;
    if (current == 2)opposite = 4;
    if (current == 3)opposite = 1;
    if (current == 4)opposite = 2;
    if (count == 10) {
        // Order: 0 = Up, 1 = Left, 2 = Down, 3 = Right
        int dists[4] = { 100000, 100000, 100000, 100000 };
        int dirs[4] = { 1,      2,      3,      4 };

        // 1. Up (dir 1)
        if (g.y > 0 && maze[g.y - 1][g.x] != '#') {
            int dx = g.x - p.x;
            int dy = (g.y - 1) - p.y;
            if (dx < 0) dx = -dx;
            if (dy < 0) dy = -dy;
            dists[0] = dx + dy;
        }
        // 2. Left (dir 2)
        if (g.x > 0 && maze[g.y][g.x - 1] != '#') {
            int dx = (g.x - 1) - p.x;
            int dy = g.y - p.y;
            if (dx < 0) dx = -dx;
            if (dy < 0) dy = -dy;
            dists[1] = dx + dy;
        }
        // 3. Down (dir 3)
        if (g.y < r - 1 && maze[g.y + 1][g.x] != '#') {
            int dx = g.x - p.x;
            int dy = (g.y + 1) - p.y;
            if (dx < 0) dx = -dx;
            if (dy < 0) dy = -dy;
            dists[2] = dx + dy;
        }
        // 4. Right (dir 4)
        if (g.x < c - 1 && maze[g.y][g.x + 1] != '#') {
            int dx = (g.x + 1) - p.x;
            int dy = g.y - p.y;
            if (dx < 0) dx = -dx;
            if (dy < 0) dy = -dy;
            dists[3] = dx + dy;
        }

        // Strict '<' in Up -> Left -> Down -> Right order preserves priority on ties
        for (int k = 0;k < 4;k++)
        {
            if (opposite == dirs[k])
                dists[k]=100000;
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
            if (g.direction == 1)      g.y--; // Up
            else if (g.direction == 2) g.x--; // Left
            else if (g.direction == 3) g.y++; // Down
            else if (g.direction == 4) g.x++; // Right
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
        // 1. Up (dir 1)
        if (g.y > 0 && maze[g.y - 1][g.x] != '#') {
            temp++;
            move[0] = 1;
        }
        // 2. Left (dir 2)
        if (g.x > 0 && maze[g.y][g.x - 1] != '#') {
            temp++;
            move[1] = 1;
        }
        // 3. Down (dir 3)
        if (g.y < r - 1 && maze[g.y + 1][g.x] != '#') {
            temp++;
            move[2] = 1;
        }
        // 4. Right (dir 4)
        if (g.x < c - 1 && maze[g.y][g.x + 1] != '#') {
            temp++;
            move[3] = 1;
        }

        // Only block reversing if there is at least one other open direction
        if (opposite != -1 && temp > 1 && move[opposite - 1]) {
            move[opposite - 1] = 0;
            temp--;
        }

        g.direction = 0;
        if (temp > 0) {
            int pick = rand() % temp; // 0-based index among valid moves
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

        if (g.direction == 1)      g.y--; // Up
        else if (g.direction == 2) g.x--; // Left
        else if (g.direction == 3) g.y++; // Down
        else if (g.direction == 4) g.x++; // Right

        count = 1;
    }
}
void update(char maze[r][c],ghost g[], player& p, int& pellets, int& pspeed,int& powercount) {
    //playermove
    playermove(maze,p,pspeed);
    //ghostmove(chaser)
    chaser(maze,p,g[0], g[0].speed);
    chaser(maze,p,g[2], g[2].speed);
    wanderers(maze, g[1], g[1].speed);
    wanderers(maze, g[3], g[3].speed);
    //score
    if (maze[p.y][p.x] == '.') {
        p.score += 10;
        maze[p.y][p.x] = '-';
        pellets--;
    }
    if (maze[p.y][p.x] == 'o') {
        p.score += 50;
        maze[p.y][p.x] = '-';
        pellets--;
        p.powerUp = 1;
        powercount == 300;
    }
    //Lives
    for (int k = 0;k < 4;k++)
        if (maze[p.y][p.x] == maze[g[k].y][g[k].x])
        {
            p.lives--;
        }
    //POWERUP
    if (p.powerUp) {
        if (powercount == 300)
        {
            powercount = 0;
        }
    }
    //couting speed
    pspeed++;
    for(int k=0;k<4;k++)
    g[k].speed++;
    powercount++;
}
void draw(sf::RenderWindow& window, char maze[r][c],ghost g[], const player& p) {
    const float TILE = 30.f;
    const float TOP_OFFSET = 50.f;

    window.clear(sf::Color::Black);

    sf::RectangleShape wallBlock({ TILE, TILE });
    wallBlock.setFillColor(sf::Color(33, 33, 222));

    sf::CircleShape pellet(3.f);
    pellet.setFillColor(sf::Color::White);

    sf::CircleShape powerPelletShape(6.f);
    powerPelletShape.setFillColor(sf::Color::Red);

    sf::CircleShape ghostShape(12.f);
    ghostShape.setFillColor(sf::Color::Cyan);

    sf::CircleShape playerShape(12.f);
    playerShape.setFillColor(sf::Color::Yellow);

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
                if (i == g[k].y && j == g[k].x) {
                    ghostShape.setPosition({ xPos + 3.f, yPos + 3.f });
                    window.draw(ghostShape);
                }
            }

            if (i == p.y && j == p.x) {
                playerShape.setPosition({ xPos + 3.f, yPos + 3.f });
                window.draw(playerShape);
            }
        }
    }

    window.display();
}
int main() {
    srand(static_cast<unsigned int>(time(0)));
    int pellets = 158;
    player p;
    int count = 0;
	ghost g[4]; // ghost positions in the grid
	int powerPellet[2]; // position of the power pellet
    char maze[r][c];
	bool gameOver = false; // game over flag
	bool win = false; // win flag
	bool paused = false; // pause flag
    bool change=false;
    int pspeed = 0;
    int powercount = 0;

    intmap(maze);
    
	// 4. Print the MAZE to the console and locations of player, ghosts, and power pellet
    cout << "MAZE USED IN GAME\n";
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++) {
            cout << maze[i][j] << " ";
            if (maze[i][j] == 'P') {
                p.x = j;
                p.y = i;
            }
            if (maze[i][j] == 'G') {
                g[count].x = j;
                g[count].y = i;
                g[count].speed = 0;
                g[count].direction = 0;
                count++;
            }
            if (maze[i][j] == 'o') {
                powerPellet[0] = j;
                powerPellet[1] = i;
            }
        }
        cout << "\n";
    }

    sf::RenderWindow window(sf::VideoMode({ 570, 680 }), "Pac-Man");
    window.setFramerateLimit(60); // keeps the game speed the same on every laptop

    //MAIN PROGRAM
    count = 0;
    while (window.isOpen()) {
        // input
        inValues(window, maze, p, paused);
        // update
        if (!paused) {
            update(maze, g,p, pellets, pspeed,powercount);
        }
        // draw
        draw(window, maze,g, p);
    }




    return 0;
}