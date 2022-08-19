#ifndef CHIP8_H_
#define CHIP8_H_

#include "Static.h"
#include <fstream>
#include <vector>

void disp_clear();
void mem_clear();
void stack_clear();
void reg_clear();
void ERR();

unsigned char chip8_fontset[80] = { 0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
		};

unsigned char memory[4096];
unsigned short opcode;
unsigned char V[16];
unsigned short I, PC;
unsigned char gfx[32][64];
unsigned char delay_timer, sound_timer;
unsigned short stack[16];
unsigned short SP;
bool drawFlag = false;
unsigned char key;

void ChipInit() {
	PC = 0x200;
	opcode = 0;
	I = 0;
	SP = 0;

	disp_clear();
	mem_clear();
	stack_clear();
	reg_clear();

	for (int i = 0; i < 80; ++i) {
		memory[i] = chip8_fontset[i];
	}
}

void emulateCycle() {
	opcode = memory[PC] << 8 | memory[PC + 1];
	unsigned short NNN = opcode & 0xFFF;
	unsigned char NN = opcode & 0xFF;
	unsigned char N = opcode & 0xF;
	unsigned short X = (opcode & 0x0F00) >> 8;
	unsigned short Y = (opcode & 0x00F0) >> 4;

	switch (opcode & 0xF000) {
	case 0x0000: {
		switch (opcode & 0x00FF) {
		case 0x00E0: {
			disp_clear();
			break;
		}
		case 0x00EE: {
//			00EE 	Flow 	return; 	Returns from a subroutine.
			PC = stack[--SP];
			break;
		}
		default: {
			ERR();
			break;
		}
		}
		break;
	}

	case 0x1000: {
//		1NNN 	Flow 	goto NNN; 	Jumps to address NNN.
		PC = NNN;
		PC -= 2;
		break;
	}

	case 0x2000: {
//		2NNN 	Flow 	*(0xNNN)() 	Calls subroutine at NNN.

		stack[SP] = PC;
		SP++;
		PC = NNN;
		PC -= 2;
		break;
	}

	case 0x3000: {
//		3XNN 	Cond 	if (Vx == NN) 	Skips the next instruction if VX equals NN.
//		(Usually the next instruction is a jump to skip a code block);
		if (V[X] == NN) {
			PC += 2;
		}
		break;
	}

	case 0x4000: {
//		4XNN 	Cond 	if (Vx != NN) 	Skips the next instruction if VX does not equal NN.
		if (V[X] != NN) {
			PC += 2;
		}
		break;
	}

	case 0x5000: {
//		5XY0 	Cond 	if (Vx == Vy) 	Skips the next instruction if VX equals VY.
		if (V[X] == V[Y]) {
			PC += 2;
		}
		break;
	}

	case 0x6000: {
//		6XNN 	Const 	Vx = NN 	Sets VX to NN.
		V[X] = NN;
		break;
	}

	case 0x7000: {
//		7XNN 	Const 	Vx += NN 	Adds NN to VX. (Carry flag is not changed);
		V[X] += NN;
		break;
	}

	case 0x8000: {
		switch (opcode & 0xF) {
		case 0x0: {
//			8XY0 	Assig 	Vx = Vy 	Sets VX to the value of VY.
			V[X] = V[Y];
			break;
		}
		case 0x1: {
//			8XY1 	BitOp 	Vx |= Vy 	Sets VX to VX or VY. (Bitwise OR operation);
			V[X] |= V[Y];
			break;
		}

		case 0x2: {
//			8XY2 	BitOp 	Vx &= Vy 	Sets VX to VX and VY. (Bitwise AND operation);
			V[X] &= V[Y];
			break;
		}

		case 0x03: {
//			8XY3[a] 	BitOp 	Vx ^= Vy 	Sets VX to VX xor VY.
			V[X] ^= V[Y];
			break;
		}
		case 0x04: {
			//VF set to 1 if there's a carry otherwise not
			unsigned short ADD = V[X] + V[Y];
			if (ADD & 0xFF == 0) {
				V[0xF] = 0;
			} else {
				V[0xF] = 1;
			}
			V[X] = ADD;
			break;
		}
		case 0x05: {
			//Vx = Vx - Vy
			unsigned short val = (V[X] - V[Y]);
			if (V[Y] > V[X]) {
				V[0xF] = 0;
			} else {
				V[0xF] = 1;
			}
			V[X] = val;
			break;
		}
		case 0x06: {
//			8XY6[a] 	BitOp 	Vx >>= 1 	Stores the least significant bit of VX in VF and then shifts VX to the right by 1.[b]
			V[0xF] = V[X] & 0x1;
			V[X] >>= 1;
			break;
		}
		case 0x07: {
//			8XY7[a] 	Math 	Vx = Vy - Vx 	Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there is not.
			unsigned short val = V[Y] - V[X];
			if (V[X] > V[Y]) {
				V[0xF] = 0;
			} else {
				V[0xF] = 1;
			}
			V[X] = val;
			break;
		}

		case 0x0E: {
//			8XYE[a] 	BitOp 	Vx <<= 1 	Stores the most significant bit of VX in VF and then shifts VX to the left by 1
			V[0xF] = V[X] >> 7;
			V[X] <<= 1;
			break;
		}

		default: {
			ERR();
			break;
		}
		}

		break;
	}

	case 0x9000: {
//		9XY0 	Cond 	if (Vx != Vy) 	Skips the next instruction if VX does not equal VY.
		if (V[X] != V[Y]) {
			PC += 2;
		}
		break;
	}

	case 0xA000: {
		//ANNN 	MEM 	I = NNN 	Sets I to the address NNN.
		I = NNN;
		break;
	}

	case 0xB000: {
//		BNNN 	Flow 	PC = V0 + NNN 	Jumps to the address NNN plus V0.
		PC = V[0] + NNN;
		PC -= 2;
		break;
	}

	case 0xC000: {
//		CXNN 	Rand 	Vx = rand() & NN 	Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
		V[X] = (rand() % 255) & NN;
		break;
	}

	case 0xD000: {
//		DXYN 	Display 	draw(Vx, Vy, N)
//		Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and
//		a height of N pixels. Each row of 8 pixels is read as bit-coded starting from memory location I;
//		I value does not change after the execution of this instruction. As described above, VF is set to 1
//		if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that does not happen
		unsigned short x = V[X];
		unsigned short y = V[Y];
		unsigned short height = opcode & 0x000F;
		unsigned short pixel;

		V[0xF] = 0;
		for (int yline = 0; yline < height; yline++) {
			pixel = memory[I + yline];
			for (int xline = 0; xline < 8; xline++) {
				if ((pixel & (0x80 >> xline)) != 0) {
					//binary
					int posX = (x + xline) % 64;
					int posY = (y + yline) % 32;
					bool oldIsOne = false;
					if (gfx[posY][posX] == 1) {
						oldIsOne = true;
					}
					gfx[posY][posX] ^= 1;
					if (oldIsOne == true) {
						if (gfx[posY][posX] == 0) {
							V[0xF] = 1;
						}
					}
				}
			}
		}
		drawFlag = true;
		break;
	}

	case 0xE000: {
		switch (opcode & 0xFF) {
		case 0xA1: {
//			EXA1 	KeyOp 	if (key() != Vx) 	Skips the next instruction if the key stored in VX is not pressed.
//			(Usually the next instruction is a jump to skip a code block);
			if (key != V[X]) {
				PC += 2;
			}
			break;
		}
		}
		break;
	}

	case 0xF000: {
		switch (opcode & 0xFF) {
		case 0x0A: {
//			FX0A 	KeyOp 	Vx = get_key()
//			A key press is awaited, and then stored in VX. (Blocking Operation. All instruction halted until next key event);
//			V[X] = get_key();
//			return;
			if (key == 200) {
				return;
			}
			V[X] = key;
			break;
		}
		case 0x07: {
//			FX07 	Timer 	Vx = get_delay() 	Sets VX to the value of the delay timer.
			V[X] = delay_timer;
			break;
		}
		case 0x15: {
//			FX15 	Timer 	delay_timer(Vx) 	Sets the delay timer to VX.
			delay_timer = V[X];
			break;
		}
		case 0x18: {
//			FX18 	Sound 	sound_timer(Vx) 	Sets the sound timer to VX.
			sound_timer = V[X];
			break;
		}
		case 0x1E: {
//			FX1E 	MEM 	I += Vx 	Adds VX to I. VF is not affected.[c]
			I += V[X];
			break;
		}
		case 0x29: {
//			FX29 	MEM 	I = sprite_addr[Vx]
//			Sets I to the location of the sprite for the character in VX.
//			Characters 0-F (in hexadecimal) are represented by a 4x5 font.

			I = V[X] * 5;
			break;
		}
		case 0x33: {
//			FX33 	BCD
//			set_BCD(Vx)
//			*(I+0) = BCD(3);
//			*(I+1) = BCD(2);
//			*(I+2) = BCD(1);
			unsigned char VX = V[X];
			unsigned char hund = (VX / 100);
			unsigned char tent = (VX / 10) % 10;
			unsigned char unit = (VX) % 10;
			memory[I + 0] = hund;
			memory[I + 1] = tent;
			memory[I + 2] = unit;
			break;
		}
		case 0x55: {
//			FX55 	MEM 	reg_dump(Vx, &I)
//			Stores from V0 to VX (including VX) in memory, starting at address I.
//			The offset from I is increased by 1 for each value written, but I itself is left unmodified.[d]
			for (int i = 0; i <= X; i++) {
				memory[i + I] = V[i];
			}
			break;
		}
		case 0x65: {
//			FX65 	MEM 	reg_load(Vx, &I)
//			Fills from V0 to VX (including VX) with values from memory, starting at address I.
//			The offset from I is increased by 1 for each value read, but I itself is left unmodified.[d]
			for (int i = 0; i <= X; i++) {
				V[i] = memory[I + i];
			}
			break;
		}
		default: {
			ERR();
			break;
		}
		}
		break;
	}

	default: {
		ERR();
		break;
	}
	}
	PC += 2;
	if (delay_timer > 0){
		delay_timer--;
	}
	if (sound_timer > 0) {
		if (sound_timer == 1) {
			print("Beep");
		}
		sound_timer--;
	}
}

void drawGraphics(sf::RenderWindow &window) {
	for (int i = 0; i < COLS; i++) {
		for (int j = 0; j < ROWS; j++) {
			unsigned char gf = gfx[j][i];
			sf::RectangleShape rect;
			rect.setPosition(i * SIZE, j * SIZE);
			if (gf == 1) {
				rect.setFillColor(sf::Color::Green);
			} else {
				rect.setFillColor(sf::Color::Black);
			}
			rect.setSize(sf::Vector2f(SIZE, SIZE));
			window.draw(rect);
		}
	}
	drawFlag = false;
}

void loadRom(std::string path) {
	std::ifstream input(path, std::ios::binary);
	std::vector<unsigned char> file(std::istreambuf_iterator<char>(input), { });

	for (int i = 0; i < file.size(); i++) {
		memory[i + 0x200] = file[i];
	}
	input.close();
}

void disp_clear() {
	for (int i = 0; i < COLS; i++) {
		for (int j = 0; j < ROWS; j++) {
			gfx[j][i] = 0;
		}
	}
	drawFlag = true;
}

void mem_clear() {
	for (int i = 0; i < 4096; i++) {
		memory[i] = 0;
	}
}

void stack_clear() {
	for (int i = 0; i < 16; i++) {
		stack[i] = 0;
	}
}

void reg_clear() {
	for (int i = 0; i < 16; i++) {
		V[i] = 0;
	}
}

void ERR() {
	printf("Unknown opcode: #%X\n", opcode);
}

#endif /* CHIP8_H_ */
