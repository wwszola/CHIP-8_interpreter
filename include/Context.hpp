/*
Class used for interacting with CPU

Example use:
CpuContext context;
context.Reset();
context.LoadProgramFromFile("program.ch8");
context.Step(5);
*/

#pragma once

#include <string>

#include "State.hpp"

/*Some versions of references specify different behaviour
of instructions 0x8XY6, 0x8XYE, 0xFX55, 0xFX65. These differences
result from incorrectly merging S-CHIP version
of language with original one*/
#define S_CHIP_COMPATIBILITY false

class CpuContext {
public:
    CpuContext();

    /*Reset m_State, m_CyclesCount*/
    void Reset();

    /*Load CHIP-8 code into memory
    filename - path to binary file
    returns 0 for success, 1 for error*/
    int LoadProgramFromFile(const std::string& filename);

    /*Debugging purposes, does not perform any checks
    Sets m_State.ProgramCounter = address*/
    void LoadMemoryRaw(BYTE* program, int size, WORD address);
    void LoadMemoryRaw(WORD* program, int size, WORD address);

    /*Step CPU, if error in program then halts and returns a number of remaining cycles
    cyclesToExecute - number of steps*/
    int Step(int cyclesToExecute);

    /*Debug tools*/
    int GetMemoryValue(WORD address, BYTE& output);
    int GetRegisterValue(uint8_t number, BYTE& output);
    WORD GetProgramCounter();
    WORD GetStackPointer();
    WORD GetAddressRegisterI();

private:
    WORD Fetch();
    int Execute(WORD opcode);

    State m_State;
    int m_CyclesCount;
};