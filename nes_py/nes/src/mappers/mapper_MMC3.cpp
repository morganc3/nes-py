//  Program:      nes-py
//  File:         mapper_MMC3.cpp
//  Description:  An implementation of the MMC3 mapper
//
//  Copyright (c) 2019 Christian Kauten. All rights reserved.
//

#include "mappers/mapper_MMC3.hpp"
#include "log.hpp"

namespace NES {

MapperMMC3::MapperMMC3(Cartridge* cart,
                       std::function<void(void)> mirroring_cb,
                       std::function<void(void)> irq_cb) :
    Mapper(cart),
    mirroring_callback(mirroring_cb),
    irq_callback(irq_cb),
    mirroring(static_cast<NameTableMirroring>(cart->getNameTableMirroring())),
    has_character_ram(cart->getVROM().size() == 0),
    bank_select(0),
    prg_mode(0),
    chr_mode(0),
    irq_latch(0),
    irq_counter(0),
    irq_reload(false),
    irq_enabled(false),
    irq_active(false),
    prev_a12(false),
    a12_low_counter(0),
    prg_ram_enabled(true),
    prg_ram_write_protect(false)
{
    for (int i = 0; i < 8; ++i)
        bank_registers[i] = 0;
    if (has_character_ram) {
        character_ram.resize(0x2000);
        LOG(Info) << "Uses character RAM" << std::endl;
    }
    update_banks();
}

NES_Byte MapperMMC3::readPRG(NES_Address address) {
    std::size_t offset = address & 0x1fff;
    if (address < 0xa000)
        return cartridge->getROM()[prg_banks[0] + offset];
    else if (address < 0xc000)
        return cartridge->getROM()[prg_banks[1] + offset];
    else if (address < 0xe000)
        return cartridge->getROM()[prg_banks[2] + offset];
    else
        return cartridge->getROM()[prg_banks[3] + offset];
}

void MapperMMC3::writePRG(NES_Address address, NES_Byte value) {
    switch (address & 0xe001) {
        case 0x8000:  // bank select
            bank_select = value & 0x7;
            prg_mode = (value >> 6) & 1;
            chr_mode = (value >> 7) & 1;
            update_banks();
            break;
        case 0x8001:  // bank data
            bank_registers[bank_select] = value;
            update_banks();
            break;
        case 0xa000:  // mirroring
            mirroring = (value & 1) ? HORIZONTAL : VERTICAL;
            mirroring_callback();
            break;
        case 0xa001:  // PRG-RAM protect
            prg_ram_write_protect = value & 0x40;
            prg_ram_enabled = value & 0x80;
            break;
        case 0xc000:  // IRQ latch
            irq_latch = value;
            break;
        case 0xc001:  // IRQ reload
            irq_counter = 0;
            irq_reload = true;
            break;
        case 0xe000:  // IRQ disable
            irq_enabled = false;
            irq_active = false;  // acknowledge pending IRQ
            break;
        case 0xe001:  // IRQ enable
            irq_enabled = true;
            break;
        default:
            // IRQ related addresses are ignored in this implementation
            break;
    }
}

NES_Byte MapperMMC3::readCHR(NES_Address address) {
    clock_irq(address);
    if (has_character_ram)
        return character_ram[address];
    std::size_t bank = address >> 10;  // 1KB
    return cartridge->getVROM()[chr_banks[bank] + (address & 0x3ff)];
}

void MapperMMC3::writeCHR(NES_Address address, NES_Byte value) {
    clock_irq(address);
    if (has_character_ram) {
        character_ram[address] = value;
    } else {
        LOG(Info) << "Read-only CHR memory write attempt at " << std::hex << address << std::endl;
    }
}

void MapperMMC3::update_banks() {
    std::size_t prg_size = cartridge->getROM().size();
    std::size_t last_bank = prg_size - 0x2000;
    std::size_t second_last_bank = prg_size - 0x4000;

    std::size_t prg_banks_total = prg_size / 0x2000;
    NES_Byte r6 = bank_registers[6] % prg_banks_total;
    NES_Byte r7 = bank_registers[7] % prg_banks_total;

    if (prg_mode == 0) {
        prg_banks[0] = 0x2000 * r6;
        prg_banks[1] = 0x2000 * r7;
        prg_banks[2] = second_last_bank;
    } else {
        prg_banks[0] = second_last_bank;
        prg_banks[1] = 0x2000 * r7;
        prg_banks[2] = 0x2000 * r6;
    }
    prg_banks[3] = last_bank;

    // update CHR banks
    std::size_t chr_size = has_character_ram ? character_ram.size()
                                             : cartridge->getVROM().size();
    std::size_t chr_banks_total = chr_size / 0x400;
    NES_Byte r0 = bank_registers[0] % chr_banks_total;
    NES_Byte r1 = bank_registers[1] % chr_banks_total;
    NES_Byte r2 = bank_registers[2] % chr_banks_total;
    NES_Byte r3 = bank_registers[3] % chr_banks_total;
    NES_Byte r4 = bank_registers[4] % chr_banks_total;
    NES_Byte r5 = bank_registers[5] % chr_banks_total;

    if (chr_mode == 0) {
        chr_banks[0] = 0x400 * (r0 & 0xfe);
        chr_banks[1] = chr_banks[0] + 0x400;
        chr_banks[2] = 0x400 * (r1 & 0xfe);
        chr_banks[3] = chr_banks[2] + 0x400;
        chr_banks[4] = 0x400 * r2;
        chr_banks[5] = 0x400 * r3;
        chr_banks[6] = 0x400 * r4;
        chr_banks[7] = 0x400 * r5;
    } else {
        chr_banks[4] = 0x400 * (r0 & 0xfe);
        chr_banks[5] = chr_banks[4] + 0x400;
        chr_banks[6] = 0x400 * (r1 & 0xfe);
        chr_banks[7] = chr_banks[6] + 0x400;
        chr_banks[0] = 0x400 * r2;
        chr_banks[1] = 0x400 * r3;
        chr_banks[2] = 0x400 * r4;
        chr_banks[3] = 0x400 * r5;
    }
}

void MapperMMC3::clock_irq(NES_Address address) {
    bool a12 = address & 0x1000;
    if (!a12) {
        if (a12_low_counter < 3)
            ++a12_low_counter;
    }
    if (a12 && !prev_a12 && a12_low_counter >= 3) {
        if (irq_counter == 0 || irq_reload) {
            irq_counter = irq_latch;
            irq_reload = false;
        } else {
            --irq_counter;
        }
        if (irq_counter == 0) {
            if (irq_enabled && !irq_active) {
                irq_active = true;
                irq_callback();
            }
        }
    }
    if (a12)
        a12_low_counter = 0;
    prev_a12 = a12;
}

}  // namespace NES

