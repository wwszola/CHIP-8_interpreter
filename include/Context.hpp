/*
Class used for interacting with CPU

Example use:
Context context;
context.Reset();
context.LoadProgramFromFile("program.ch8");
context.Step(5);
*/

#pragma once

#include <string>

#include "State.hpp"

class Context {
public:
    Context();

    /*Reset m_State, m_CyclesCount*/
    void Reset();

    /*Load CHIP-8 code into memory
    filename - path to binary file
    returns 0 for success, 1 for error*/
    int LoadProgramFromFile(const std::string& filename);

    /*Step CPU, if error in program then halts and returns a number of remaining cycles
    cyclesToExecute - number of steps*/
    int Step(int cyclesToExecute);

private:
    State::WORD Fetch();
    int Execute(State::WORD opcode);

    State m_State;
    int m_CyclesCount;
};