#include "Context.hpp"

#include <algorithm>
#include <fstream>

Context::Context() {
    Reset();
}

void Context::Reset() {
    std::fill(m_State.Memory, m_State.Memory + State::c_MemorySize, 0x0);
    std::fill(m_State.RegistersV, m_State.RegistersV + 16, 0x0);
    m_State.AddressRegisterI = 0x0;
    m_State.StackPointer = State::c_StackAddress;
    m_State.ProgramCounter = State::c_ProgramLoadAddress;

    m_CyclesCount = 0;
}

int Context::LoadProgramFromFile(const std::string& filename) {
    std::fstream file(filename.c_str(), std::fstream::in | std::fstream::binary);
    if (!file.is_open())
        return 1;

    file.seekg(0, file.end);
    int size = file.tellg();
    file.seekg(0, file.beg);

    /*TODO: Log warning
    if(size>State::c_StackAddress - State::c_ProgramLoadAddress)
        ;
    */

    file.read(reinterpret_cast<char*>(m_State.Memory + State::c_ProgramLoadAddress), size);

    return 0;

}

void Context::LoadMemoryRaw(BYTE* program, int size, WORD address) {
    std::copy(program, program + size, m_State.Memory + address);
    m_State.ProgramCounter = address;
}

int Context::Step(int cyclesToExecute) {
    while (cyclesToExecute > 0) {
        WORD opcode = Fetch();
        if (Execute(opcode) > 0) {
            /*TODO LOG ERROR*/
            break;
        }

        --cyclesToExecute;
        ++m_CyclesCount;
    }
    return cyclesToExecute;
}

int Context::GetMemoryValue(WORD address, BYTE& output) {
    if (address < State::c_MemorySize) {
        output = m_State.Memory[address];
        return 0;
    }
    /*TODO LOG DEBUG*/
    return 1;
}

int Context::GetRegisterValue(uint8_t number, BYTE& output) {
    if (number < State::c_RegisterAmount) {
        output = m_State.RegistersV[number];
        return 0;
    }
    /*TODO LOG DEBUG*/
    return 1;
}

WORD Context::GetProgramCounter(){
    return m_State.ProgramCounter;
}

WORD Context::Fetch() {
    WORD opcode;
    opcode = m_State.Memory[m_State.ProgramCounter++];
    opcode <<= 8;
    opcode |= m_State.Memory[m_State.ProgramCounter++];
    return opcode;
}

int Context::Execute(WORD opcode) {
    uint8_t firstNibble = (opcode & 0xF000) >> 12;
    uint8_t secondNibble = (opcode & 0x0F00) >> 8;
    uint8_t thirdNibble = (opcode & 0x00F0) >> 4;

    switch (firstNibble) {
    case 0x1: {
        //jump to address
        m_State.ProgramCounter = opcode & 0x0FFF;
        break;
    }
    case 0x6: {
        //store const in register
        m_State.RegistersV[secondNibble] = opcode & 0x00FF;
        break;
    }
    case 0x7: {
        //add const to register(without a carry)
        m_State.RegistersV[secondNibble] += opcode & 0x00FF;
        break;
    }
    case 0x8: {
        switch (opcode & 0x000F) {
        case 0x0: {
            //store register in register
            m_State.RegistersV[secondNibble] = m_State.RegistersV[thirdNibble];
            break;
        }
        case 0x1: {
            //bitwise OR
            m_State.RegistersV[secondNibble] |= m_State.RegistersV[thirdNibble];
            break;
        }
        case 0x2: {
            //bitwise AND
            m_State.RegistersV[secondNibble] &= m_State.RegistersV[thirdNibble];
            break;
        }
        case 0x3: {
            //bitwise XOR
            m_State.RegistersV[secondNibble] ^= m_State.RegistersV[thirdNibble];
            break;
        }
        case 0x4: {
            //add
            bool overflow = m_State.RegistersV[secondNibble] > (0xFF - m_State.RegistersV[thirdNibble]);
            m_State.RegistersV[0xF] = overflow ? 0x01 : 0x00;
            m_State.RegistersV[secondNibble] += m_State.RegistersV[thirdNibble];
            break;
        }
        case 0x5: {
            //subtract
            bool underflow = m_State.RegistersV[secondNibble] <= m_State.RegistersV[thirdNibble];
            m_State.RegistersV[0xF] = underflow ? 0x00 : 0x01;
            m_State.RegistersV[secondNibble] -= m_State.RegistersV[thirdNibble];
            break;
        }
        case 0x6: {
            //shift right
            m_State.RegistersV[0xF] = m_State.RegistersV[secondNibble] & 0x01;
            m_State.RegistersV[secondNibble] >>= 1;
            break;
        }
        case 0x7: {
            //subtract (different order)
            bool underflow = m_State.RegistersV[thirdNibble] <= m_State.RegistersV[secondNibble];
            m_State.RegistersV[0xF] = underflow ? 0x00 : 0x01;
            m_State.RegistersV[secondNibble] = m_State.RegistersV[thirdNibble] - m_State.RegistersV[secondNibble];
            break;
        }
        case 0xE: {
            //shift left
            m_State.RegistersV[0xF] = m_State.RegistersV[secondNibble] & 0x80;
            m_State.RegistersV[secondNibble] <<= 1;
            break;
        }
        default: {
            return 1;
        }
        }
    }
    default: {
        return 1;
    }
    }
    return 0;
}