#include "tools.hpp"
#include "catch2.hpp"
#include "common.hpp"

#define private public
#include "c64/c64.hpp"

static const char petscii[] = {
        '?','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o',
        'p','q','r','s','t','u','v','w','x','y','z','?','?','?','?','?',
        ' ','!','"','#','$','%','&','\'','(',')','*','+',',','-','.','/',
        '0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?',
        '@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
        'P','Q','R','S','T','U', 'V','W','X','Y','Z','[','?',']',' ',' ',
};

static std::vector<uint8_t> read_rom(std::string filepath) {
    std::ifstream ifs(filepath, std::ios::binary | std::ios::ate);

    if (!ifs)
        throw std::runtime_error("Unable to open: " + filepath);

    auto end = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    auto size = std::size_t(end - ifs.tellg());

    if (size == 0) // avoid undefined behavior
        throw std::runtime_error(filepath + " is empty!");

    std::vector<uint8_t> data(size);
    ifs.read((char *) data.data(), size);
    return data;
}

TEST_CASE("C64") {
    auto c64 = C64();

    SECTION("Test BASIC ROM") {
        auto basic = read_rom(std::string("roms/basic.bin"));

        c64.write(0x0001, 0b011);
        REQUIRE(basic.size() == 0x2000);

        for (uint16_t i = 0; i < basic.size(); i++) {
            REQUIRE(basic[i] == c64.read(0xA000 + i, true));
        }
    }

    SECTION("Test CHAR ROM") {
        auto char_rom = read_rom(std::string("roms/char.bin"));

        c64.write(0x0001, 0b011);
        REQUIRE(char_rom.size() == 0x1000);

        for (uint16_t i = 0; i < char_rom.size(); i++) {
            REQUIRE(char_rom[i] == c64.read(0xD000 + i, true));
        }
    }

    SECTION("Test KERNAL ROM") {
        auto kernal = read_rom(std::string("roms/kernal.bin"));

        c64.write(0x0001, 0b010);
        REQUIRE(kernal.size() == 0x2000);

        for (uint16_t i = 0; i < kernal.size(); i++) {
            REQUIRE(kernal[i] == c64.read(0xE000 + i, true));
        }
    }

    SECTION("Run C64") {
        c64.reset();

        int count = 1;
        while (true) {
            while(!c64.cpu.complete()) {
                c64.cpu.clock(c64);
            }

//            std::string status = nestools::full_cpu_status_as_string(c64.cpu, c64);
//            printf("%d -> %s\n", count, status.c_str());
            c64.cpu.clock(c64);

            if(c64.read(0x04CD, true) == 0x2E) {
                printf("Screen completed after %d instructions.\n", count);
                break;
            }
            count++;
        }

        for(int y = 0; y < 25; y++) {
            printf("$%04X |", 0x0400 + (y * 40));
            for(int x = 0; x < 40; x++) {
                auto value = c64.read(0x0400 + (y * 40) + x, true);
//                printf("%02X", value);
                printf("%c", petscii[value]);
            }
            printf("|\n");
        }
    }
}