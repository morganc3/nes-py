//  Program:      nes-py
//  File:         mapper.hpp
//  Description:  An abstract factory for mappers
//
//  Copyright (c) 2019 Christian Kauten. All rights reserved.
//

#ifndef MAPPER_FACTORY_HPP
#define MAPPER_FACTORY_HPP

#include "mapper.hpp"
#include "mappers/mapper_NROM.hpp"
#include "mappers/mapper_SxROM.hpp"
#include "mappers/mapper_UxROM.hpp"
#include "mappers/mapper_CNROM.hpp"
#include "mappers/mapper_MMC3.hpp"

namespace NES {

/// an enumeration of mapper IDs
enum class MapperID : NES_Byte {
    NROM  = 0,
    SxROM = 1,
    UxROM = 2,
    CNROM = 3,
    MMC3  = 4,
};

/// Create a mapper for the given cartridge with optional callbacks
///
/// @param game the cartridge to initialize a mapper for
/// @param mirroring_cb callback used when mirroring mode changes
/// @param irq_cb callback used to raise IRQ interrupts
///
Mapper* MapperFactory(Cartridge* game,
                      std::function<void(void)> mirroring_cb,
                      std::function<void(void)> irq_cb) {
    switch (static_cast<MapperID>(game->getMapper())) {
        case MapperID::NROM:
            return new MapperNROM(game);
        case MapperID::SxROM:
            return new MapperSxROM(game, mirroring_cb);
        case MapperID::UxROM:
            return new MapperUxROM(game);
        case MapperID::CNROM:
            return new MapperCNROM(game);
        case MapperID::MMC3:
            return new MapperMMC3(game, mirroring_cb, irq_cb);
        default:
            return nullptr;
    }
}

}  // namespace NES

#endif  // MAPPER_FACTORY_HPP
