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

void inValues() {

}
void update() {

}
void draw() {

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
    ifstream in("maze.txt");
    if (!in) {
        std::cerr << "Failed to open maze.txt for reading.\n";
        return 1;
    }
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++) {
            if (!(in >> maze[i][j])) {
                cout << "File ended early or had invalid data at row "
                    << i << ", col " << j << "\n";
                i = r; // stop outer loop
                break;    // stop inner loop
            }
        }
    }

	// 4. Print the MAZE to the console
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

    const float TILE = 30.f; // one cell is 30 x 30 pixels
    //RECTANGLE
    sf::RectangleShape wallBlock({ TILE, TILE });
    wallBlock.setFillColor(sf::Color(33, 33, 222));
	//CIRCLE
    sf::CircleShape pellet(3.f); // radius 3 pixels
    pellet.setFillColor(sf::Color::White);
	//POWER PELLET
	sf::CircleShape powerPelletShape(6.f); // radius 6 pixels
	powerPelletShape.setFillColor(sf::Color::Red);
	//PLAYER
	sf::CircleShape playerShape(12.f); // radius 12 pixels
	playerShape.setFillColor(sf::Color::Yellow);
	//GHOST
	sf::CircleShape ghostShape(12.f); // radius 12 pixels
	ghostShape.setFillColor(sf::Color::Cyan);


    //MAIN PROGRAM
    count = 0;
    while (window.isOpen())
    {
        //INPUT
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
            else if (const auto* key = event->getIf<sf::Event::KeyPressed>())
            {
                if ((key->code == sf::Keyboard::Key::Up || key->code == sf::Keyboard::Key::W) && maze[p.y - 1][p.x] != '#')
                    p.direction = 1; // the direction the player asked for
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

        //update
        if (count == 7)
        {
            if (p.direction) {
                if (p.direction == 1 && maze[p.y - 1][p.x] != '#') {
                    p.y--;
                }
                else if (p.direction == 2 && maze[p.y + 1][p.x] != '#' ) {
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
            p.score+=10;
            maze[p.y][p.x] = '-';
            pellets--;
        }
        if (maze[p.y][p.x] == 'o') {
            p.score += 50;
            maze[p.y][p.x] = '-';
            pellets--;
        }
        count++;
        //draw
        window.clear(sf::Color::Black);
        for (int i = 0; i < r; i++) {
            for (int j = 0; j < c; j++) {
				if (maze[i][j] == '#') {
					wallBlock.setPosition({ j * TILE, i * TILE + 50.f});
					window.draw(wallBlock);
				}
				if (maze[i][j] == '.') {
                    pellet.setPosition({ j * TILE + 12.f, i * TILE + 12.f + 50.f });
					window.draw(pellet);
				}
				if (maze[i][j] == 'o') {
					powerPelletShape.setPosition({ j * TILE + 9.f, i * TILE + 9.f + 50.f });
					window.draw(powerPelletShape);
				}
				if (maze[i][j] == 'G') {
					ghostShape.setPosition({ j * TILE + 3.f, i * TILE + 3.f + 50.f });
					window.draw(ghostShape);
				}
                if(i==p.y && j==p.x) {
                    playerShape.setPosition({ j * TILE + 3.f, i * TILE + 3.f + 50.f });
                    window.draw(playerShape);
                }
            }
        }
        window.display();
    }




    return 0;
}