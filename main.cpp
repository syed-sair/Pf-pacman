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
	int defaultx = 0, defaulty = 0; // default x and y positions
	int px, py; // previous x and y positions
};
struct ghost {
    int x, y, direction,speed,defaultx, defaulty;
	bool powerdown = false; // active flag
	int px, py; // previous x and y positions
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
            else if ((key->code == sf::Keyboard::Key::Down || key->code == sf::Keyboard::Key::S) && maze[p.y+1][p.x] != '#')
                p.direction = 3;
            else if ((key->code == sf::Keyboard::Key::Left || key->code == sf::Keyboard::Key::A) && maze[p.y][p.x-1] != '#')
                p.direction = 2;
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
        p.px = p.x;
        p.py = p.y;
        if (p.direction) {
            if (p.direction == 1 && maze[p.y - 1][p.x] != '#') {
                p.y--;
            }
            else if (p.direction == 2 && maze[p.y][p.x-1] != '#') {
                p.x--;
            }
            else if (p.direction == 3 && maze[p.y+1][p.x] != '#') {
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
    //opposite used for avoid reversing
    int opposite=-1,current = g.direction;
    if (current == 1)opposite = 3;
    if (current == 2)opposite = 4;
    if (current == 3)opposite = 1;
    if (current == 4)opposite = 2;
    if (count == 10) {
            g.px = g.x;
            g.py = g.y;
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
        g.px = g.x;
        g.py = g.y;
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
void update(char maze[r][c],ghost g[], player& p, int& pellets, int& pspeed,int& powercount,int& reset,bool& win, bool& lose) {
    //playermove
    playermove(maze,p,pspeed);
    //ghostmove(chaser)
    for (int i = 0;i < 4;i++) {
        if (g[i].powerdown)
            ghostf(maze, g[i], g[i].speed);
        else {
            if(i==0)chaser(maze, p, g[0], g[0].speed);
            if (i == 2)chaser(maze, p, g[2], g[2].speed);
            if (i == 1)wanderers(maze, g[1], g[1].speed);
            if (i == 3)wanderers(maze, g[3], g[3].speed);
        }
    }
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
        powercount = 0;
		for (int i = 0;i < 4;i++) {
			g[i].powerdown = 1;
		}
    }
    //Lives
        for (int k = 0;k < 4;k++)
            if (((p.y == g[k].y && p.x == g[k].x)|| p.x == g[k].px && p.y == g[k].py && g[k].x == p.px && g[k].y == p.py)&&!g[k].powerdown)
            {
                p.lives--;
                p.x = p.defaultx;
                p.y = p.defaulty;
                p.px = p.defaultx;
                p.py = p.defaulty;
                p.direction = 0;
				powercount = 0;
                for (int k = 0;k < 4;k++)
                {
                    g[k].x = g[k].defaultx;
                    g[k].y = g[k].defaulty;
                    g[k].px = g[k].defaultx;
                    g[k].py = g[k].defaulty;
					g[k].powerdown = 0;
                    g[k].direction = 0;
                }
				break;//so multiple lives are not lost in one frame
            }
    //POWERUP
    if (p.powerUp)
    {
        if (powercount == 300)
        {
            p.powerUp = 0;
            for (int k = 0;k < 4;k++)
                {
                g[k].powerdown = false;
                }
        }
        powercount++;
        for (int k = 0;k < 4;k++)
            if (((p.y == g[k].y && p.x == g[k].x) || p.x == g[k].px && p.y == g[k].py && g[k].x == p.px && g[k].y == p.py)&&g[k].powerdown)
            {
                p.score += 200;
                g[k].x = g[k].defaultx;
                g[k].y = g[k].defaulty;
				g[k].px = g[k].defaultx;
                g[k].py = g[k].defaulty;
                g[k].powerdown = false;
                g[k].direction = 0;
            }
    }
    //counting speed
    pspeed++;
    for(int k=0;k<4;k++)
    g[k].speed++;
    //GAMEOVER
	if (pellets == 0)
	{
		win = true;
	}
    if (p.lives == 0)
    {
        lose = true;
    }
}
void draw(sf::RenderWindow& window, char maze[r][c], ghost g[], const player& p, const sf::Font& font, bool paused, int pellets,bool& win, bool& lose) {
    const float TILE = 30.f;
    const float TOP_OFFSET = 50.f;

    window.clear(sf::Color::Black);

    sf::Text scoreText(font, "SCORE: " + to_string(p.score), 20);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition({ 15.f, 12.f });
    window.draw(scoreText);

    sf::Text livesText(font, "LIVES: " + to_string(p.lives), 20);
    livesText.setFillColor(sf::Color::Yellow);
    livesText.setPosition({ 450.f, 12.f });
    window.draw(livesText);

    if (!(win || lose))
    {
        sf::Text pelletsText(font, "PELLETS: " + to_string(pellets), 15);
        pelletsText.setFillColor(sf::Color::Cyan);
        pelletsText.setPosition({ 240.f, 5.f });
        window.draw(pelletsText);
    }

    if (win)
    {
        sf::Text WIN(font, "YOU WIN!", 20);
        WIN.setFillColor(sf::Color::Green);
        WIN.setPosition({ 240.f, 12.f });
        window.draw(WIN);
    }

    if (lose)
    {
        sf::Text LOSE(font, "YOU LOSE!", 20);
        LOSE.setFillColor(sf::Color::Red);
        LOSE.setPosition({ 240.f, 12.f });
        window.draw(LOSE);
    }

    if (paused&&!(win||lose)) {
        sf::Text pausedText(font, "PAUSED", 15);
        pausedText.setFillColor(sf::Color::Red);
        pausedText.setPosition({ 240.f, 20.f });
        window.draw(pausedText);
    }

    sf::RectangleShape divider({ 570.f, 2.f });
    divider.setFillColor(sf::Color(33, 33, 222));
    divider.setPosition({ 0.f, TOP_OFFSET - 2.f });
    window.draw(divider);

    sf::RectangleShape wallBlock({ TILE, TILE });
    wallBlock.setFillColor(sf::Color(33, 33, 222));

    sf::CircleShape pellet(3.f);
    pellet.setFillColor(sf::Color::White);

    sf::CircleShape powerPelletShape(6.f);
    powerPelletShape.setFillColor(sf::Color::Red);

    sf::CircleShape ghost1Shape(12.f);
    ghost1Shape.setFillColor(sf::Color::Cyan);

    sf::CircleShape ghost2Shape(12.f);
    ghost2Shape.setFillColor(sf::Color::Red);

    sf::CircleShape ghost3Shape(12.f);
    ghost3Shape.setFillColor(sf::Color::Green);

    sf::CircleShape ghost4Shape(12.f);
    ghost4Shape.setFillColor(sf::Color::Magenta);

    sf::CircleShape ghostfShape(12.f);
    ghostfShape.setFillColor(sf::Color::Blue);

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
            for (int k = 0; k < 4; k++)
                if (i == g[k].y && j == g[k].x && g[k].powerdown)
                {
                    ghostfShape.setPosition({ xPos + 3.f, yPos + 3.f });
                    window.draw(ghostfShape);
                }
                else
                {
                    if (i == g[k].y && j == g[k].x) {
                        switch (k) {
                        case 0:
                            ghost1Shape.setPosition({ xPos + 3.f, yPos + 3.f });
                            window.draw(ghost1Shape);
                            break;
                        case 1:
                            ghost2Shape.setPosition({ xPos + 3.f, yPos + 3.f });
                            window.draw(ghost2Shape);
                            break;
                        case 2:
                            ghost3Shape.setPosition({ xPos + 3.f, yPos + 3.f });
                            window.draw(ghost3Shape);
                            break;
                        case 3:
                            ghost4Shape.setPosition({ xPos + 3.f, yPos + 3.f });
                            window.draw(ghost4Shape);
                            break;
                        }
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
    sf::Font font;
    if (!font.openFromFile("arial.ttf")) {
        cout << "Failed to load font\n";
    }
    srand(static_cast<unsigned int>(time(0)));
    bool win=0, lose=0;
    int pellets = 158;
    player p;
    int count = 0;
	ghost g[4]; // ghost positions in the grid
	int powerPellet[2]; // position of the power pellet
    char maze[r][c];
	bool paused = false; // pause flag
    int pspeed = 0;
    int powercount = 0;
	int resetcount = 0;

    intmap(maze);
    
	// Print the MAZE to the console and locations of player, ghosts, and power pellet
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
                count++;
            }
            if (maze[i][j] == 'o') {
                powerPellet[0] = j;
                powerPellet[1] = i;
            }
        }
        cout << "\n";
    }

    //Counting Pellets
    pellets = 0;
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++) {
            if (maze[i][j] == '.' || maze[i][j] == 'o') {
                pellets++;
            }
        }
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
                if(!(win||lose))
                    update(maze, g, p, pellets, pspeed, powercount, resetcount, win, lose);
            }
            // draw
            draw(window, maze, g, p, font, paused, pellets, win, lose);
    }




    return 0;
}