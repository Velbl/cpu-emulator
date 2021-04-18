//============================================================================
// Name        : CPU_Emulator.cpp
// Author      : Stefan Velbl
// Version     :
// Copyright   : 
// Description : Emulation of CPU in C++
//============================================================================

#include <iostream>
#include <string.h>
using namespace std;


#define KB                   (1024)
#define NUMBER_OF_KB         (64)
#define MEM_RANGE            (NUMBER_OF_KB * KB)
#define TWO_CYCLES           (2)

using Byte = uint8_t;
using Word = uint16_t;

struct ProcessorStatus
{
	Byte C : 1;  // carry flag
	Byte Z : 1;  // zero flag
	Byte I : 1;  // interrupt disable
	Byte D : 1;  // decimal mode
	Byte B : 1;  // break command
	Byte O : 1;  // overflow flag
	Byte N : 1;  // negative flag
};

struct IndexRegisters
{
	Byte A;  // accumulator
	Byte X;  // index register X
	Byte Y;  // index register Y
};

struct Mem
{
	Byte  b_data[MEM_RANGE];

	void Initialize()
	{
		uint32_t byte_cnt;

		for(byte_cnt = 0u; byte_cnt < MEM_RANGE; byte_cnt++)
		{
			b_data[byte_cnt] = 0u;
		}
	}

	// read 1 byte
	Byte operator[](uint32_t address) const
	{
		// assert here address is < MEM_RANGE
		return b_data[address];
	}

	// write 1 byte
	// We must return by reference in function because an expression like �arr[i]� can be used an lvalue.
	// This function will:
	//   - not be able to modify any b_data memorybers (except mutable ones)
	//   - will not be able to call any non-const memoryber functions
	Byte& operator[](uint32_t address)
	{
		// assert here address is < MEM_RANGE
		return b_data[address];
	}
};

struct CPU
{
	Word PC;  // program counter
	Word SP;  // stack pointer
	ProcessorStatus PS;
	IndexRegisters  IR;

	// helper variables
	Byte instruction;
	Byte b_data;
	Byte value;
	Byte zero_page_address;
	Word w_data;

	// possible system endianness
	static constexpr Byte LITTLE_ENDIAN_SYSTEM = 0u;
	static constexpr Byte BIG_ENDIAN_SYSTEM    = 1u;

	// opcodes
	static constexpr Byte INS_LDA_IM  = 0xA9;  // load accumulator immediate
	static constexpr Byte INS_LDA_ZP  = 0xA5;  // load accumulator zero page
	static constexpr Byte INS_LDA_ZPX = 0xB5;  // load accumulator zero page x
	static constexpr Byte INS_JST     = 0x20;  // jump to subroutine

	Byte CheckEndianness()
	{
		int  num = 1;
		printf("Checking for system system...\n");

		if (*(char *)&num == 1)
		{
			printf(" -Little-Endian system detected.\n");
			return LITTLE_ENDIAN_SYSTEM;
		}
		else
		{
			printf(" -Big-Endian system detected.\n");
			return BIG_ENDIAN_SYSTEM;
		}
	}

	void Reset(Mem& memory)
	{
		printf("CPU reset.\n");

		PC   = 0xFFFC;
		SP   = 0x0100;
		PS   = {0u};
		IR   = {0u};

		printf("Memory initialization.\n");
		memory.Initialize();
	}

	Byte FetchByte(uint8_t& cycles, Mem& memory)
	{
		b_data = memory[PC];
		PC++;
		cycles--;
		return b_data;
	}

	// conversion from big endian to little endian in one word (16bit)
	void SwapBytesInWord(Word* w_data)
	{
		uint8_t FirstByte, SecondByte;

		FirstByte  = (*w_data & 0xFF00) >> 8;
		SecondByte = (*w_data & 0x00FF) >> 0;

		printf("Before conversion (big endian): %x\n", *w_data);

		(void)memset((void*)w_data    , FirstByte , sizeof(Byte));
		(void)memset((void*)w_data + 1, SecondByte, sizeof(Byte));

		printf("FirstByte:  %x   \n", FirstByte);
		printf("SecondByte: %x   \n", SecondByte);

		printf("After conversion (little endian): %x\n", *w_data);

	}

	Byte FetchWord(uint8_t& cycles, Mem& memory)
	{
		Byte system;
		system = CheckEndianness();

		// 6502 is little endian
		// first - read least significant byte
		w_data = memory[PC];
		PC++;

		// second - read high significant byte
		w_data |= (memory[PC] << 8);
		PC++;

		if ( system == BIG_ENDIAN_SYSTEM )
		{
			SwapBytesInWord(&w_data);
		}

		cycles-=2;
		return b_data;
	}

	Byte ReadByte(uint8_t& cycles,Byte zero_page_address, Mem& memory)
	{
		b_data = memory[zero_page_address];
		cycles--;
		return b_data;
	}

	void LDASetStatus()
	{
		PS.Z = (IR.A == 0u);
		PS.N = (IR.A & 0b10000000) > 0;
	}

	void Execute(uint8_t cycles, Mem& memory)
	{
		while(cycles > 0)
		{
			instruction = FetchByte(cycles, memory);

			switch(instruction)
			{
			case INS_LDA_IM:
				value = FetchByte(cycles, memory);
				IR.A = value;
				LDASetStatus();
				break;
			case INS_LDA_ZP:
				zero_page_address = FetchByte(cycles, memory);
				IR.A = ReadByte(cycles, zero_page_address, memory);
				LDASetStatus();
				break;
			case INS_LDA_ZPX:
				zero_page_address = FetchByte(cycles, memory);
				zero_page_address += IR.X;
				cycles--;
				IR.A = ReadByte(cycles, zero_page_address, memory);
				LDASetStatus();
				break;
			case INS_JST:
				break;
			default:
				printf("Instruction not handled %d \n.", instruction);
				break;
			}
		}
	}
};

int main()
{
	Mem memory;
	CPU cpu;

	cpu.Reset(memory);

	cpu.CheckEndianness();

	// start - inline a little program
	memory[0xFFFC] = CPU::INS_LDA_ZP;
	memory[0xFFFD] = 0x42;
	memory[0x0042] = 0x84;
	// end - inline a little program

	cpu.Execute(3, memory);

	return 0;
}
