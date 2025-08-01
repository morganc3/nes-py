//  Program:      nes-py
//  File:         mapper_MMC3.hpp
//  Description:  An implementation of the MMC3 mapper
//
//  Copyright (c) 2019 Christian Kauten. All rights reserved.
//

#ifndef MAPPERMMC3_HPP
#define MAPPERMMC3_HPP

#include <vector>
#include "common.hpp"
#include "mapper.hpp"

namespace NES {

class MapperMMC3 : public Mapper {
 private:
    /// The mirroring callback on the PPU
    std::function<void(void)> mirroring_callback;
    /// Callback used to issue IRQs on the CPU
    std::function<void(void)> irq_callback;
    /// the mirroring mode on the device
    NameTableMirroring mirroring;
    /// whether the cartridge uses character RAM
    bool has_character_ram;
    /// the bank select register
    NES_Byte bank_select;
    /// the bank data registers
    NES_Byte bank_registers[8];
    /// PRG mode flag
    NES_Byte prg_mode;
    /// CHR mode flag
    NES_Byte chr_mode;
    /// pointers to PRG banks
    std::size_t prg_banks[4];
    /// pointers to CHR banks
    std::size_t chr_banks[8];
    /// Character RAM if present
    std::vector<NES_Byte> character_ram;
    /// IRQ latch register
    NES_Byte irq_latch;
    /// IRQ counter value
    NES_Byte irq_counter;
    /// whether the counter should be reloaded on next tick
    bool irq_reload;
    /// whether IRQs are enabled
    bool irq_enabled;
    /// whether an IRQ is currently active/pending
    bool irq_active;
    /// previous state of address bit 12
    bool prev_a12;
    /// count of consecutive cycles with A12 low
    int a12_low_counter;
    /// whether PRG-RAM is enabled
    bool prg_ram_enabled;
    /// whether PRG-RAM is write protected
    bool prg_ram_write_protect;

    /// update the cached bank pointers
    void update_banks();
    /// clock the IRQ counter on an address access
    void clock_irq(NES_Address address);

 public:
    /// Create a new mapper with a cartridge.
    ///
    /// @param cart a reference to a cartridge for the mapper to access
    /// @param mirroring_cb the callback to change mirroring modes on the PPU
    /// @param irq_cb the callback to trigger an IRQ on the CPU
    ///
    MapperMMC3(Cartridge* cart, std::function<void(void)> mirroring_cb,
               std::function<void(void)> irq_cb);

    /// Read a byte from the PRG RAM.
    ///
    /// @param address the 16-bit address of the byte to read
    /// @return the byte located at the given address in PRG RAM
    ///
    NES_Byte readPRG(NES_Address address);

    /// Write a byte to an address in the PRG RAM.
    ///
    /// @param address the 16-bit address to write to
    /// @param value the byte to write to the given address
    ///
    void writePRG(NES_Address address, NES_Byte value);

    /// Read a byte from the CHR RAM.
    ///
    /// @param address the 16-bit address of the byte to read
    /// @return the byte located at the given address in CHR RAM
    ///
    NES_Byte readCHR(NES_Address address);

    /// Write a byte to an address in the CHR RAM.
    ///
    /// @param address the 16-bit address to write to
    /// @param value the byte to write to the given address
    ///
    void writeCHR(NES_Address address, NES_Byte value);

    /// Return the name table mirroring mode of this mapper.
    inline NameTableMirroring getNameTableMirroring() { return mirroring; }

    /// Return whether PRG-RAM is enabled.
    inline bool isPRGRAMEnabled() override { return prg_ram_enabled; }

    /// Return whether PRG-RAM is write protected.
    inline bool isPRGRAMWriteProtected() override { return prg_ram_write_protect; }
};

}  // namespace NES

#endif  // MAPPERMMC3_HPP
