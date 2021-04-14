#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"


#include "Context.hpp"

TEST_CASE("Single instruction tests") {
    Context context;
    context.Reset();

    WORD address = 0x200;

    SECTION("Flow 0x1NNN") {
        WORD program[] = {
            0x1310  //jump to 0x310
        };
        context.LoadMemoryRaw(program, 2*1, address);
        
        context.Step(1);
        REQUIRE(context.GetProgramCounter() == 0x310);
    }
    SECTION("Const 0x6XNN 0x7XNN") {
        WORD program[] = {
            0x6CFF, //VC=0xFF
            0x7C11  //VC+=0x11 -> VC=0x10
        };
        context.LoadMemoryRaw(program, 2*2, address);
        BYTE output;

        context.Step(1);
        context.GetRegisterValue(0xC, output);
        REQUIRE(output == 0xFF);
        
        context.Step(1);
        context.GetRegisterValue(0xC, output);
        REQUIRE(output == 0x10);
    }
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
        context.LoadMemoryRaw(program, 2*24, address);
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
        context.GetRegisterValue(0xC, output);
        context.GetRegisterValue(0xF, flagOutput);
        REQUIRE(output == 0x03);
        REQUIRE(flagOutput == 0x00);

        context.Step(1);
        context.GetRegisterValue(0xC, output);
        context.GetRegisterValue(0xF, flagOutput);
        REQUIRE(output == 0x01);
        REQUIRE(flagOutput == 0x01);

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
        context.GetRegisterValue(0xC, output);
        context.GetRegisterValue(0xF, flagOutput);
        REQUIRE(output == 0x02);
        REQUIRE(flagOutput == 0x00);

        context.Step(2);
        context.GetRegisterValue(0xC, output);
        context.GetRegisterValue(0xF, flagOutput);
        REQUIRE(output == 0x42);
        REQUIRE(flagOutput == 0x01);
    }
}