//  Program:      nes-py
//  File:         cartridge.cpp
//  Description:  This class houses the logic and data for an NES cartridge
//
//  Copyright (c) 2019 Christian Kauten. All rights reserved.
//

#include <fstream>
#include "cartridge.hpp"
#include "log.hpp"

namespace NES {

void Cartridge::loadFromFile(std::string path) {
    // create a stream to load the ROM file
    std::ifstream romFile(path, std::ios_base::binary | std::ios_base::in);
    // create a byte vector for the iNES header
    std::vector<NES_Byte> header;
    header.resize(0x10);
    romFile.read(reinterpret_cast<char*>(&header[0]), 0x10);
    // read internal data
    name_table_mirroring = header[6] & 0xB;
    mapper_number = ((header[6] >> 4) & 0xf) | (header[7] & 0xf0);
    // determine the number of PRG-RAM banks; when the header reports zero
    // the iNES spec implies a single 8KB bank for backwards compatibility
    prg_ram_banks = static_cast<std::size_t>(header[8]);
    if (prg_ram_banks == 0)
        prg_ram_banks = 1;
    // extended RAM availability depends on the finalized PRG-RAM bank count
    // and the battery-backed RAM flag in mapper byte 6
    has_extended_ram = (prg_ram_banks > 0) || (header[6] & 0x2);
    // read PRG-ROM 16KB banks
    NES_Byte banks = header[4];
    prg_rom.resize(0x4000 * banks);
    romFile.read(reinterpret_cast<char*>(&prg_rom[0]), 0x4000 * banks);
    // read CHR-ROM 8KB banks
    NES_Byte vbanks = header[5];
    if (!vbanks)
        return;
    chr_rom.resize(0x2000 * vbanks);
    romFile.read(reinterpret_cast<char*>(&chr_rom[0]), 0x2000 * vbanks);
}

}  // namespace NES
