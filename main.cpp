#include <SFML/Graphics.hpp>
#include <optional>
#include<iostream>
using namespace std;

int main() {
    int r = 0, c = 0;
    sf::RenderWindow window(sf::VideoMode({ 570, 680 }), "Pac-Man");
    window.setFramerateLimit(60); // keeps the game speed the same on every laptop

    const float TILE = 30.f; // one cell is 30 x 30 pixels
    sf::RectangleShape wallBlock({ TILE, TILE });
    wallBlock.setFillColor(sf::Color(33, 33, 222));
    sf::CircleShape pellet(3.f); // radius 3 pixels
    pellet.setFillColor(sf::Color::White);
    // for the cell at row r and column c:
    wallBlock.setPosition({ c * TILE, r * TILE });
    window.draw(wallBlock);

    return 0;
}