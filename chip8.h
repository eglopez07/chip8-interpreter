#ifndef CHIP8
#define CHIP8
#include <stdio.h>

class chip8 {
public:
	chip8();
	~chip8();
	
	void chip8::initialize() {
		
		pc = 0x200; // System expects application to be loaded at this memory location
		opcode = 0; // Reset current opcode
		I = 0; // Reset index register
		sp = 0; // Reset stack pointer
		
		// Clear display
		memset (gfx, 0, 2048);
		// Clear stack
		memset (stack, 0, 16);
		// Clear registers V0-VF
		memset (V, 0, 16);
		// Clear memory
		memset (memory, 0, 4096);
		
		// Load fontset
		for(int i = 0; i < 80; ++i)
			memory[i] = chip8_fontset[i];
		
		// Reset timers
		delay_timer = 0;
		sound_timer = 0;
	}
	
	void chip8::loadGame(char *gameTitle) {
		
		// Load game file
		char buffer[BUFSIZ];
		FILE *gameFile;
		gameFile = fopen (gameTitle, "rb");
		
		for(int i = 0; i < BUFSIZ; ++i)
			memory[i + 512] = buffer[i];
	}
	
	char chip8::get_delay() { return delay_timer; }
	
	char chip8::get_sound() { return sound_timer; }
	
	// Emulate a cycle of the CHIP8 CPU
	void chip8::emulateCycle() {
		// Fetch Opcode
		opcode = memory[pc] << 8 | memory[pc+1];
		
		// Decode Opcode
		
		/* Check opcode with opcode table
		 * 35 opcodes implemented with 35 switch cases
		 * Invalid opcode will likely result in game crash
		 */
		switch(opcode & 0xF000) {
			case 0x0000: // 0NNN
				switch(opcode & 0x000F) {
					case 0x0000: // 00E0
						// Clear display
						memset (gfx, 0, 2048);
					break;
				
					case 0x000E: // 00EE
						// Return from a subroutine
						// Set program counter to address at top of the stack
						// Subtract 1 from stack pointer
						pc = stack[sp];
						--sp;
					break;
					
					default:
						printf ("Unknown opcode [0x0000]: 0x%X\n", opcode);
				}
			break;
			
			case 0x1000: // 1NNN
				// Jump to address 1NNN
				pc = opcode & 0x0FFF;
			break;
				
			case 0x2000: // 2NNN
				// Call subroutine at 2NNN
				stack[sp] = pc;
				++sp;
				pc = opcode & 0x0FFF;
			break;
				
			case 0x3000: // 3XNN
				// Skip next instruction if V[X] equals NN
				if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
					// Skip next instruction
					pc += 2;
				}
				pc += 2;
			break;
				
			case 0x4000: // 4XNN
				// Skip next instruction if V[X] does not equal NN
				if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
					// Skip next instruction
					pc += 2;
				}
				pc += 2;
			break;
			
			case 0x5000: // 5XY0
				// Skip next instruction if V[X] equals V[Y]
				if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4]) {
					// Skip next instruction
				}
				pc += 2;
			break;
			
			case 0x6000: // 6XNN
				// Set V[X] to NN
				V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
				pc += 2;
			break;
			
			case 0x7000: // 7XNN
				// Add NN to V[X]
				V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
				pc += 2;
			break;
			
			case 0x8000: // 8XY0
				switch (opcode & 0x00FF) {
					case 0x0000:
						// Set V[X] to value in V[Y]
						V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
						pc += 2;
					break;
				
					case 0x0001: // 8XY1
						// Set V[X] to value of V[X]|V[Y]
						V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] | V[(opcode & 0x00F0) >> 4];
					break;
				
					case 0x0002: // 8XY2
						// Set V[X] to value of V[X]&V[Y]
					break;
				
					case 0x0003: // 8XY3
						// Set V[X] to value of V[X]^V[Y]
					break;
				
					case 0x0004: // 8XY4
						// Add V[Y] to V[X]. VF set to 1 when there's a carry, 0 otherwise
						if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
							V[0xF] = 1; // Carry Flag
						else
							V[0xF] = 0;
						
						V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
						pc += 2;
					break;
				
					case 0x0005: // 8XY5
						// Subtract V[Y] from V[X]. VF set to 0 when there's a borrow, 1 otherwise
					break;
				
					case 0x0006: // 8XY6
						// Shift V[Y] right by one and store value to V[X]. VF set to least significant bit of V[Y] before shift
					break;
				
					case 0x0007: // 8XY7
						// Set V[X] to value of V[Y] minus V[X]. VF set to 0 when there's a borrow, 1 otherwise
					break;
				
					case 0x000E: // 8XYE
						// Shift V[Y] left by one and copy result to VX. VF set to most significant bit of V[Y] before shift
					break;
					
					default:
						printf ("Unknown opcode [0x8000]: 0x%X\n", opcode);
				}
			break;
			
			case 0x9000: // 9XY0
				// Skip next instruction if V[X] does not equal V[Y].
			break;
			
			case 0xA000: // ANNN
				// Set I to the address NNN
				I = opcode & 0x0FFF;
				pc += 2;
			break;
				
			case 0xB000: // BNNN
				// Jump to address (NNN plus V0)
			break;
			
			case 0xC000: // CXNN
				// Set V[X] to result of a bitwise and operation on a random number and NN
			break;
			
			case 0xD000: // DXYN
				// Draw sprite at coordinate (V[X], V[Y]) that has a width of 8 pixels and height of N pixels.
				// Each row of 8 pixels is read as bit-coded starting from memory[I]
				// VF set to 1 if any screen pixels are flipped from set to unset when sprite is drawn, and 0 otherwise
			break;
			
			case 0xE000:
				switch (opcode & 0x00FF) {
					case 0xE09E: // EX9E
						// Skip next instruction if the key stored in V[X] is pressed.
					break;
			
					case 0xE0A1: // EXA1
						// Skip next instruction if the key stored in V[X] is not pressed.
					break;
					
					default:
						printf ("Unknown Opcode 0xE000: 0x%X\n", opcode);
				}
			break;
			
			case 0xF000:
				switch (opcode & 0x00FF) {
					case 0x0007: // FX07
						// Set V[X] to value of delay_timer
						V[(opcode & 0x0F00) >> 8] = get_delay();
					break;
			
					case 0x000A: // FX0A
						// Wait for key press then store key in V[X]. Halt all instruction until next key press.
					break;	
			
					case 0x0015: // FX15
						// Set delay_timer to V[X]
						delay_timer = V[(opcode & 0x0F00) >> 8];
					break;	
			
					case 0x0018: // FX18
						// Set sound_timer to V[X]
						sound_timer = V[(opcode & 0x0F00) >> 8];
					break;	
			
					case 0x001E: // FX1E
						// Add V[X] to I
						I += V[(opcode & 0x0F00) >> 8];
					break;
			
					case 0x0029: // FX29
						// Set I to the location of the sprite for the character in V[X].
						// Characters 0-F (in hexadecimal) represented by 4x5 font
					break;
			
					case 0x0033: // FX33
						// Store binary-coded decimal representation of V[X]
						// take the decimal representation of VX
						// place the hundreds digit in memory at location in I
						// the tens digit at location I+1
						// the ones digit at location I+2
					break;
			
					case 0x0055: // FX55
						// Store V0 to V[X] (including V[X]) in memory starting at address I
						// Increment I by 1 for each value written
					break;
			
					case 0x0065: // FX65
						// Fill V0 to V[X] (including V[X]) with values from memory starting at address I
						// Increment I by 1 for each value written
					break;
					
					default:
						printf ("Unknown opcode [0xF000]: 0x%X\n", opcode);
				}
			break;
			
			default:
				printf ("Unknown opcode: 0x%X\n", opcode);
		}

		// Update timers
		if (delay_timer > 0)
			--delay_timer;

		if(sound_timer > 0) {
			if(sound_timer == 1)
				printf("BEEP!\n");
			--sound_timer;
		}
	}

	void chip8::setKeys() {
		// Map key bindings to emulated HEX based keypad
	}
	

	
private:
	unsigned short opcode;
	unsigned char memory[4096];
	unsigned char V[16];
	unsigned short I; // index register
	unsigned short pc; // program counter
	unsigned char gfx[2048]; // screen size 2048 pixels
	unsigned char delay_timer;
	unsigned char sound_timer;
	unsigned short stack[16];
	unsigned short sp; // stack pointer
	unsigned char key[16];
}
#endif