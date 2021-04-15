#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"


#include "CpuContext.hpp"

TEST_CASE("Single instruction tests") {
    CpuContext context;
    context.Reset();

    WORD address = 0x200;

    SECTION("Flow 0x1NNN") {
        WORD program[] = {
            0x1310  //jump to 0x310
        };
        context.LoadMemoryRaw(program, 2 * 1, address);

        context.Step(1);
        REQUIRE(context.GetProgramCounter() == 0x310);
    }
    SECTION("Const 0x6XNN 0x7XNN") {
        WORD program[] = {
            0x6CFF, //VC=0xFF
            0x7C11  //VC+=0x11 -> VC=0x10
        };
        context.LoadMemoryRaw(program, 2 * 2, address);
        BYTE output;

        context.Step(1);
        context.GetRegisterValue(0xC, output);
        REQUIRE(output == 0xFF);

        context.Step(1);
        context.GetRegisterValue(0xC, output);
        REQUIRE(output == 0x10);
    }

    /*TODO
    Rewrite this section into separate ones*/
    SECTION("BitOP and Math 0x8XY0-0x8XY7 0x8XYE") {
        WORD program[] = {
            0x6CF0, //VC=0xF0
            0x6E0F, //VE=0x0F
            0x8CE0, //VC=VE -> VC=0x0F

            0x6CF0, //VC=0xF0
            0x8CE1, //VC|=VE -> VC=0xFF

            0x8CE2, //VC&=VE -> VC=0x0F

            0x8CE3, //VC^=VE -> VC=0x00

            0x6C09, //VC=0x09
            0x8CE4, //VC+=VE -> VC=0x18 VF=0
            0x6EF0, //VE=0xF0
            0x8CE4, //VC+=VE -> VC=0x08 VF=1

            0x6E0B, //VE=0x0B
            0x8CE5, //VC-=VE -> VC=0xFD VF=0
            0x6C11, //VC=0x11
            0x8CE5, //VC-=VE -> VC=0x06 VF=1

            0x8C06, //VC>>=1 -> VC=0x03 VF=0
            0x8C06, //VC>>=1 -> VC=0x01 VF=1

            0x6C0D, //VC=0x0D
            0x8CE7, //VC=VE-VC -> VC=0xFE VF=0
            0x6EFF, //VE=0xFF
            0x8CE7, //VC=VE-VC -> VC=0x01 VF=1

            0x8C0E, //VC<<=1 -> VC=0x02 VF=0
            0x6CA1, //VC=0xA1
            0x8C0E, //VC<<=1 -> VC=0x42 VF=1
        };
        context.LoadMemoryRaw(program, 2 * 24, address);
        BYTE output;
        BYTE flagOutput;

        context.Step(3);
        context.GetRegisterValue(0xC, output);
        REQUIRE(output == 0x0F);

        context.Step(2);
        context.GetRegisterValue(0xC, output);
        REQUIRE(output == 0xFF);

        context.Step(1);
        context.GetRegisterValue(0xC, output);
        REQUIRE(output == 0x0F);

        context.Step(1);
        context.GetRegisterValue(0xC, output);
        REQUIRE(output == 0x00);

        context.Step(2);
        context.GetRegisterValue(0xC, output);
        context.GetRegisterValue(0xF, flagOutput);
        REQUIRE(output == 0x18);
        REQUIRE(flagOutput == 0x00);

        context.Step(2);
        context.GetRegisterValue(0xC, output);
        context.GetRegisterValue(0xF, flagOutput);
        REQUIRE(output == 0x08);
        REQUIRE(flagOutput == 0x01);

        context.Step(2);
        context.GetRegisterValue(0xC, output);
        context.GetRegisterValue(0xF, flagOutput);
        REQUIRE(output == 0xFD);
        REQUIRE(flagOutput == 0x00);

        context.Step(2);
        context.GetRegisterValue(0xC, output);
        context.GetRegisterValue(0xF, flagOutput);
        REQUIRE(output == 0x06);
        REQUIRE(flagOutput == 0x01);

        context.Step(1);        
        /*TODO
        Fix for S-CHIP compatibility problems*/
        // context.GetRegisterValue(0xC, output);
        // context.GetRegisterValue(0xF, flagOutput);
        // REQUIRE(output == 0x03);
        // REQUIRE(flagOutput == 0x00);

        context.Step(1);
        /*TODO
        Fix for S-CHIP compatibility problems*/
        // context.GetRegisterValue(0xC, output);
        // context.GetRegisterValue(0xF, flagOutput);
        // REQUIRE(output == 0x01);
        // REQUIRE(flagOutput == 0x01);

        context.Step(2);
        context.GetRegisterValue(0xC, output);
        context.GetRegisterValue(0xF, flagOutput);
        REQUIRE(output == 0xFE);
        REQUIRE(flagOutput == 0x00);

        context.Step(2);
        context.GetRegisterValue(0xC, output);
        context.GetRegisterValue(0xF, flagOutput);
        REQUIRE(output == 0x01);
        REQUIRE(flagOutput == 0x01);

        context.Step(1);
        /*TODO
        Fix for S-CHIP compatibility problems*/
        // context.GetRegisterValue(0xC, output);
        // context.GetRegisterValue(0xF, flagOutput);
        // REQUIRE(output == 0x02);
        // REQUIRE(flagOutput == 0x00);

        context.Step(2);
        /*TODO
        Fix for S-CHIP compatibility problems*/
        // context.GetRegisterValue(0xC, output);
        // context.GetRegisterValue(0xF, flagOutput);
        // REQUIRE(output == 0x42);
        // REQUIRE(flagOutput == 0x01);
    }
    SECTION("Flow subroutine 0x2NNN 0x00EE") {
        WORD program[] = {
            0x1206, //0x200 jump to 0x206
            0x6001, //0x202 subroutine start -> V0=0x01
            0x00EE, //0x204 return from subroutine, -> PC=0x208 SP=0xEA0
            0x2202  //0x206 call subroutine at 0x202 -> PC=0x202 SP=0xEA2
        };
        context.LoadMemoryRaw(program, 2 * 4, address);
        BYTE output;

        context.Step(2);
        REQUIRE(context.GetProgramCounter() == 0x202);
        REQUIRE(context.GetStackPointer() == 0xEA2);

        context.Step(2);
        REQUIRE(context.GetProgramCounter() == 0x208);
        REQUIRE(context.GetStackPointer() == 0xEA0);
        context.GetRegisterValue(0, output);
        REQUIRE(output == 0x01);

    }
    SECTION("Conditional 3XNN") {
        WORD program[] = {
            0x6212, //V2=0x12
            0x3212, //0x202 skip if V2==0x12 (should skip first time, not second)
            0x6215, //V2=0x15
            0x6200, //V2=0x00
            0x1202  //jump to 0x202
        };
        context.LoadMemoryRaw(program, 2 * 5, address);
        BYTE output;

        context.Step(2);
        context.GetRegisterValue(2, output);
        REQUIRE(output == 0x12);
        REQUIRE(context.GetProgramCounter() == 0x206);

        context.Step(2);
        context.GetRegisterValue(2, output);
        REQUIRE(output == 0x00);
        REQUIRE(context.GetProgramCounter() == 0x202);

        context.Step(2);
        context.GetRegisterValue(2, output);
        REQUIRE(output == 0x15);
    }
    SECTION("Conditional 4XNN") {
        WORD program[] = {
            0x6200, //V2=0x00
            0x4212, //0x202 skip if V2!=0x12 (should skip first time, not second)
            0x6215, //V2=0x15
            0x6212, //V2=0x12
            0x1202  //jump to 0x202
        };
        context.LoadMemoryRaw(program, 2 * 5, address);
        BYTE output;

        context.Step(2);
        context.GetRegisterValue(2, output);
        REQUIRE(output == 0x00);
        REQUIRE(context.GetProgramCounter() == 0x206);

        context.Step(2);
        context.GetRegisterValue(2, output);
        REQUIRE(output == 0x12);
        REQUIRE(context.GetProgramCounter() == 0x202);

        context.Step(2);
        context.GetRegisterValue(2, output);
        REQUIRE(output == 0x15);
    }
    SECTION("Conditional 5XY0") {
        WORD program[] = {
            0x6105, //V1=0x05
            0x6205, //V2=0x05
            0x5120, //0x204 skip if V1==V2 (should skip first time, not second)
            0x6215, //V2=0x15
            0x6212, //V2=0x12
            0x1204  //jump to 0x204
        };
        context.LoadMemoryRaw(program, 2 * 6, address);
        BYTE output;

        context.Step(3);
        context.GetRegisterValue(2, output);
        REQUIRE(output == 0x05);
        REQUIRE(context.GetProgramCounter() == 0x208);

        context.Step(2);
        context.GetRegisterValue(2, output);
        REQUIRE(output == 0x12);
        REQUIRE(context.GetProgramCounter() == 0x204);

        context.Step(2);
        context.GetRegisterValue(2, output);
        REQUIRE(output == 0x15);
    }
    SECTION("Conditional 9XY0") {
        WORD program[] = {
            0x6105, //V1=0x05
            0x6209, //V2=0x09
            0x9120, //0x204 skip if V1!=V2 (should skip first time, not second)
            0x6215, //V2=0x15
            0x6205, //V2=0x05
            0x1204  //jump to 0x204
        };
        context.LoadMemoryRaw(program, 2 * 6, address);
        BYTE output;

        context.Step(3);
        context.GetRegisterValue(2, output);
        REQUIRE(output == 0x09);
        REQUIRE(context.GetProgramCounter() == 0x208);

        context.Step(2);
        context.GetRegisterValue(2, output);
        REQUIRE(output == 0x05);
        REQUIRE(context.GetProgramCounter() == 0x204);

        context.Step(2);
        context.GetRegisterValue(2, output);
        REQUIRE(output == 0x15);
    }
    SECTION("Memory ANNN") {
        WORD program[] = {
            0xA123  //set I to address 0x123
        };
        context.LoadMemoryRaw(program, 2 * 1, address);

        context.Step(1);
        REQUIRE(context.GetAddressRegisterI() == 0x123);
    }
    SECTION("Flow BNNN") {
        WORD program[] = {
            0x6011, //V0=0x11
            0xB205  //PC=V0+0x205
        };
        context.LoadMemoryRaw(program, 2 * 2, address);

        context.Step(2);
        REQUIRE(context.GetProgramCounter() == 0x216);
    }
    SECTION("Memory FX1E") {
        WORD program[] = {
            0x6244, //V2=0x44
            0xA005, //I=0x005
            0xF21E  //I+=V2 -> I=0x049
        };
        context.LoadMemoryRaw(program, 2 * 3, address);

        context.Step(3);
        REQUIRE(context.GetAddressRegisterI() == 0x049);
    }
    SECTION("BCD FX33") {
        WORD program[] = {
            0x62A2, //V2=0xA2
            0xA300, //I=0x300
            0xF233  //store BCD of value V2 at address I
        };
        context.LoadMemoryRaw(program, 2 * 3, address);
        BYTE output;

        context.Step(3);
        context.GetMemoryValue(0x300, output);
        REQUIRE(output == 1);
        context.GetMemoryValue(0x300 + 1, output);
        REQUIRE(output == 6);
        context.GetMemoryValue(0x300 + 2, output);
        REQUIRE(output == 2);
    }
    /*TODO
    Check for S-CHIP compatibility*/
    SECTION("Memory FX55") {
        WORD program[] = {
            0x6002, //V0=0x02
            0x6104, //V1=0x04
            0x6206, //V2=0x06
            0x6308, //V3=0x08
            0xA300, //I=0x300
            0xF355  //registers V0-V3 dump at address I 
        };
        context.LoadMemoryRaw(program, 2 * 6, address);
        BYTE output;

        context.Step(6);
        context.GetMemoryValue(0x300, output);
        REQUIRE(output == 0x02);
        context.GetMemoryValue(0x300 + 1, output);
        REQUIRE(output == 0x04);
        context.GetMemoryValue(0x300 + 2, output);
        REQUIRE(output == 0x06);
        context.GetMemoryValue(0x300 + 3, output);
        REQUIRE(output == 0x08);
    }
    /*TODO
    Check for S-CHIP compatibility*/
    SECTION("Memory FX65") {
        WORD program[] = {
            0x1206, //jump to 0x206
            0x0306, //0x202 stored values 0x03, 0x06
            0x090C, //stored values 0x09, 0x0C
            0xA202, //0x206 I=0x202
            0xF365  //registers V0-V3 load from address I
        };
        context.LoadMemoryRaw(program, 2 * 5, address);
        BYTE output;

        context.Step(5);
        context.GetRegisterValue(0, output);
        REQUIRE(output == 0x03);
        context.GetRegisterValue(1, output);
        REQUIRE(output == 0x06);
        context.GetRegisterValue(2, output);
        REQUIRE(output == 0x09);
        context.GetRegisterValue(3, output);
        REQUIRE(output == 0x0C);
    }
}