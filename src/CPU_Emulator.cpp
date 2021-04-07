//============================================================================
// Name        : CPU_Emulator.cpp
// Author      : Stefan Velbl
// Version     :
// Copyright   : 
// Description : Emulation of CPU in C++
//============================================================================

#include <iostream>
using namespace std;

#define KB            (1024)
#define NUMBER_OF_KB  (64)
#define MEM_RANGE     (NUMBER_OF_KB * KB)
#define TWO_CYCLES    (2)

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

struct Memory
{
  Byte  Data[MEM_RANGE];

  void Initialize()
  {
    uint32_t ByteCnt;
    for( ByteCnt = 0u; ByteCnt < MEM_RANGE; ByteCnt++)\
    {
      Data[ByteCnt] = 0u;
    }
  }

  Byte operator[](uint32_t Address) const
  {
    return Data[Address];
  }
};

struct CPU
{
  Word PC;  // program counter
  Word SP;  // stack pointer
  ProcessorStatus PS;
  IndexRegisters  IR;

  // Helper variables.
  Byte Instruction;
  Byte Data;

  void Reset(Memory& memory)
  {
    PC   = 0xFFFC;
    SP   = 0x0100;
    PS   = {0u};
    IR   = {0u};

    memory.Initialize();
  }

  Byte FetchByte(uint8_t& Cycles, Memory& memory)
  {
    Data = memory[PC];
    PC++;
    Cycles--;
    return Data;
  }

  void Execute(uint8_t Cycles, Memory& memory)
  {
    while(Cycles > 0)
    {
      Instruction = FetchByte(Cycles, memory);
      (void)Instruction;  // Compiler satisfied.
    }
  }

};

int main()
{
  Memory memory;
  CPU    cpu;

  cpu.Reset(memory);
  cpu.Execute(TWO_CYCLES, memory);

  return 0;
}
