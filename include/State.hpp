/*
Struct with internal state of a CPU with memory,
    stack pointer, registers, program counter

Memory layout (typical values):
0x000 - 0x1FF reserved for interpreter
0x200 - 0xE9F for use by program
0xEA0 - 0xEFF stack
0xF00 - 0xFFF display refresh

*/

#pragma once

/*BYTE and WORD are used only in context of CPU architecture*/
using BYTE = uint8_t;
using WORD = uint16_t;

struct State {

    const static int c_MemorySize = 0x1000;
    BYTE Memory[c_MemorySize];

    const static int c_StackAddress = 0xEA0;
    WORD StackPointer;

    const static int c_RegisterAmount = 0xF;
    BYTE RegistersV[c_RegisterAmount];
    WORD AddressRegisterI;

    const static int c_ProgramLoadAddress = 0x200;
    WORD ProgramCounter;

    const static int c_DisplayAddress = 0xF00;
};