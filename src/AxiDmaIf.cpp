//---------------------------------------------------------------------------//
//        ____  _____________  __    __  __ _           _____ ___   _        //
//       / __ \/ ____/ ___/\ \/ /   |  \/  (_)__ _ _ __|_   _/ __| /_\       //
//      / / / / __/  \__ \  \  /    | |\/| | / _| '_/ _ \| || (__ / _ \      //
//     / /_/ / /___ ___/ /  / /     |_|  |_|_\__|_| \___/|_| \___/_/ \_\     //
//    /_____/_____//____/  /_/      T  E  C  H  N  O  L  O  G  Y   L A B     //
//                                                                           //
//---------------------------------------------------------------------------//

// Copyright (c) 2021 Deutsches Elektronen-Synchrotron DESY

#include <ios>

#include <boost/log/core/core.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/trivial.hpp>

#include "AxiDmaIf.hpp"

void AxiDmaIf::start(uintptr_t start_desc) {
    BOOST_LOG_SEV(_slg, blt::severity_level::debug)
        << "AxiDmaIf: start, start_desc = " << std::hex << start_desc << std::dec;

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    _ctrlReg.wr({
        .Reset = 1
    });

    _curDesc.wr(start_desc & ((1ULL << 32) - 1));
    _curDescMsb.wr(start_desc >> 32);

    BOOST_LOG_SEV(_slg, blt::severity_level::trace)
        << "AxiDmaIf: DMA ctrl = 0x" << std::hex << _ctrlReg.rd_int() << std::dec;

    auto tmp = _ctrlReg.rd();
    tmp.RS = 1;
    tmp.Cyc_bd_en = 1;
    tmp.IOC_IrqEn = 1;
    _ctrlReg.wr(tmp);

    _tailDesc.wr(0x50);                     // for circular
    _tailDescMsb.wr(start_desc >> 32);      // for circular

    BOOST_LOG_SEV(_slg, blt::severity_level::trace)
        << "AxiDmaIf: DMA ctrl = 0x" << std::hex << _ctrlReg.rd_int() << std::dec;
}