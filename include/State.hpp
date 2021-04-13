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

#include <cstdint>


struct State {
    using BYTE = std::uint8_t;
    using WORD = std::uint16_t;

    const static int c_MemorySize = 0x1000;
    BYTE Memory[c_MemorySize];

    const static int c_StackAddress = 0xEA0;
    WORD StackPointer;

    BYTE RegistersV[16];
    WORD AddressRegisterI;

    const static int c_ProgramLoadAddress = 0x200;
    WORD ProgramCounter;

    const static int c_DisplayAddress = 0xF00;
};