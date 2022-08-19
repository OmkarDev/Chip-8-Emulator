#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Static.h"
#include <iostream>
#include "Chip8.h"

using namespace sf;

void init(RenderWindow &window) {
	window.create(VideoMode(WIN_WIDTH, WIN_HEIGHT), "Chip8 Emulator",
			Style::Close);
	auto desktop = VideoMode::getDesktopMode();
	window.setPosition(
			Vector2i((desktop.width - window.getSize().x) / 2,
					(desktop.height - window.getSize().y) / 2));
}


RenderWindow window;

int main() {
	init(window);
	window.setFramerateLimit(60);

	ChipInit();
//	loadRom("roms/Pong.ch8");

	loadRom("roms/TICTAC.ch8");

	while (window.isOpen()) {
		Event event;
		while (window.pollEvent(event)) {
			if (event.type == Event::Closed) {
				window.close();
			}
			if (event.type == sf::Event::KeyReleased) {
				key = 200;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::X)) {
				key = 0x0;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) {
				key = 0x1;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) {
				key = 0x2;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) {
				key = 0x3;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
				key = 0x4;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
				key = 0x5;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
				key = 0x6;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
				key = 0x7;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
				key = 0x8;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
				key = 0x9;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) {
				key = 0xA;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::C)) {
				key = 0xB;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num4)) {
				key = 0xC;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
				key = 0xD;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::F)) {
				key = 0xE;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::V)) {
				key = 0xF;
			}
		}
		emulateCycle();
		if (drawFlag) {
			drawGraphics(window);
			window.display();
		}
	}

	return 0;
}
