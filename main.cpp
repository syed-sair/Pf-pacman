#include <SFML/Graphics.hpp>
#include <optional>
#include<iostream>
#include<fstream>
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
        void inValues(sf::RenderWindow & window, char maze[r][c], player & p, bool& paused) {
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
		void update(char maze[r][c], player& p, int& pellets,int& count) {
            if (count == 7)
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
                count = 0;
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
			}
            count++;
		}
        void draw(sf::RenderWindow& window, char maze[r][c], const player& p) {
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
                    else if (maze[i][j] == '.') {
                        pellet.setPosition({ xPos + 12.f, yPos + 12.f });
                        window.draw(pellet);
                    }
                    else if (maze[i][j] == 'o') {
                        powerPelletShape.setPosition({ xPos + 9.f, yPos + 9.f });
                        window.draw(powerPelletShape);
                    }
                    else if (maze[i][j] == 'G') {
                        ghostShape.setPosition({ xPos + 3.f, yPos + 3.f });
                        window.draw(ghostShape);
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
    int pellets = 158;
    player p;
    int count = 0;
	int ghost[4][2]; // ghost positions in the grid
	int powerPellet[2]; // position of the power pellet
    char maze[r][c];
	bool gameOver = false; // game over flag
	bool win = false; // win flag
	bool paused = false; // pause flag
    bool change=false;

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
                ghost[count][0] = j;
                ghost[count][1] = i;
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
            update(maze, p, pellets, count);
        }
        // draw
        draw(window, maze, p);
    }




    return 0;
}