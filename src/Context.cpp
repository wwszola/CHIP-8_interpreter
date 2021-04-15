#include "Context.hpp"

#include <algorithm>
#include <fstream>

/*Macro setting n-th bit to value x in num*/
#define SET_BIT(num, n, x) num = num & ~(1 << n) | (x << n); 

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

void Context::LoadMemoryRaw(WORD* program, int size, WORD address) {
    BYTE* start = reinterpret_cast<BYTE*>(program);
    BYTE* end = start + size;
    int i = 0;
    while (start + i != end) {
        //changes byte order in big endian WORD
        *(m_State.Memory + address + i) = *(start + i + 1 - 2 * (i % 2));
        ++i;
    }
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

WORD Context::GetProgramCounter() {
    return m_State.ProgramCounter;
}

WORD Context::GetStackPointer() {
    return m_State.StackPointer;
}

WORD Context::GetAddressRegisterI() {
    return m_State.AddressRegisterI;
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
    case 0x0: {
        switch (opcode & 0x0FFF) {
        case 0xEE: {
            //returns from subroutine
            m_State.ProgramCounter = m_State.Memory[--m_State.StackPointer];
            m_State.ProgramCounter += m_State.Memory[--m_State.StackPointer] << 8;
            break;
        }
        default: {
            return 1;
        }
        }
        break;
    }
    case 0x1: {
        //jump to address
        m_State.ProgramCounter = opcode & 0x0FFF;
        break;
    }
    case 0x2: {
        //call subroutine
        m_State.Memory[m_State.StackPointer++] = (m_State.ProgramCounter & 0xFF00) >> 8;
        m_State.Memory[m_State.StackPointer++] = m_State.ProgramCounter & 0x00FF;
        m_State.ProgramCounter = opcode & 0x0FFF;
        break;
    }
    case 0x3: {
        //skip if register==const
        if (m_State.RegistersV[secondNibble] == (opcode & 0x00FF)) {
            m_State.ProgramCounter += 2;
        }
        break;
    }
    case 0x4: {
        //skip if register!=const
        if (m_State.RegistersV[secondNibble] != (opcode & 0x00FF)) {
            m_State.ProgramCounter += 2;
        }
        break;
    }
    case 0x5: {
        switch (opcode & 0x000F) {
        case 0x0: {
            //skip if two registers equal
            if (m_State.RegistersV[secondNibble] == m_State.RegistersV[thirdNibble]) {
                m_State.ProgramCounter += 2;
            }
            break;
        }
        default: {
            return 1;
        }
        }
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
#if S_CHIP_COMPATIBILITY
            m_State.RegistersV[0xF] = m_State.RegistersV[secondNibble] & 0x01;
            m_State.RegistersV[secondNibble] >>= 1;
#else 
            m_State.RegistersV[0xF] = m_State.RegistersV[thirdNibble] & 0x01;
            m_State.RegistersV[secondNibble] = m_State.RegistersV[thirdNibble] >> 1;
#endif
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
#if S_CHIP_COMPATIBILITY
            m_State.RegistersV[0xF] = (m_State.RegistersV[secondNibble] & 0x80) >> 7;
            m_State.RegistersV[secondNibble] <<= 1;
#else
            m_State.RegistersV[0xF] = (m_State.RegistersV[thirdNibble] & 0x80) >> 7;
            m_State.RegistersV[secondNibble] = m_State.RegistersV[thirdNibble] << 1;
#endif
            break;
        }
        default: {
            return 1;
        }
        }
        break;
    }
    case 0x9: {
        switch (opcode & 0x000F) {
        case 0x0: {
            //skip if two registers not equal
            if (m_State.RegistersV[secondNibble] != m_State.RegistersV[thirdNibble]) {
                m_State.ProgramCounter += 2;
            }
            break;
        }
        default: {
            return 1;
        }
        }
        break;
    }
    case 0xA: {
        //set address register to const
        m_State.AddressRegisterI = opcode & 0x0FFF;
        break;
    }
    case 0xB: {
        //jump to address V0+const
        m_State.ProgramCounter = m_State.RegistersV[0] + opcode & 0X0FFF;
        break;
    }
    case 0xF: {
        switch (opcode & 0x00FF) {
        case 0x1E: {
            //add value of register to I
            m_State.AddressRegisterI += m_State.RegistersV[secondNibble];
            break;
        }
        case 0x33: {
            //stores 3 digit BCD of register value at I, I+1, I+2
            //algorithm thanks to https://my.eng.utah.edu/~nmcdonal/Tutorials/BCDTutorial/BCDConversion.html
            uint8_t value = m_State.RegistersV[secondNibble];
            uint8_t digits[3] = { 0x0,0x0,0x0 };
            int i, j;
            for (i = 0; i < 8; ++i) {
                for (j = 0; j < 3; ++j) {
                    if (digits[j] >= 5)digits[j] += 3;
                }
                digits[0] = (digits[0] << 1) & 0x0F;
                SET_BIT(digits[0], 0, ((digits[1] & 0b1000) >> 3));
                digits[1] = (digits[1] << 1) & 0x0F;
                SET_BIT(digits[1], 0, ((digits[2] & 0b1000) >> 3));
                digits[2] = (digits[2] << 1) & 0x0F;
                SET_BIT(digits[2], 0, ((value & 0x80) >> 7));
                value <<= 1;
            }
            m_State.Memory[m_State.AddressRegisterI] = digits[0];
            m_State.Memory[m_State.AddressRegisterI + 1] = digits[1];
            m_State.Memory[m_State.AddressRegisterI + 2] = digits[2];
            break;
        }
        case 0x55: {
            //stores multiple registers at I, I+1, ...
#if S_CHIP_COMPATIBILITY
            int i;
            for (i = 0; i <= secondNibble; ++i) {
                m_State.Memory[m_State.AddressRegisterI + i] = m_State.RegistersV[i];
            }
#else
            int i;
            for (i = 0; i <= secondNibble; ++i) {
                m_State.Memory[m_State.AddressRegisterI++] = m_State.RegistersV[i];
            }
#endif
            break;
        }
        case 0x65: {
            //loads multiple registers from I, I+1, ...
#if S_CHIP_COMPATIBILITY
            int i;
            for (i = 0; i <= secondNibble; ++i) {
                m_State.RegistersV[i] = m_State.Memory[m_State.AddressRegisterI + i];
            }
#else
            int i;
            for (i = 0; i <= secondNibble; ++i) {
                m_State.RegistersV[i] = m_State.Memory[m_State.AddressRegisterI++];
            }
#endif
            break;
        }
        default: {
            return 1;
        }
        }
        break;
    }
    default: {
        return 1;
    }
    }
    return 0;
}