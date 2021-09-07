// Copyright (c) 2021 Deutsches Elektronen-Synchrotron DESY

#pragma once

#include "CtkRegWrapper.hpp"

namespace blt = boost::log::trivial;

class AxiDmaIf {
    struct __attribute__((packed)) S2mmDmaControlReg {
        bool RS : 1;
        uint32_t rsvd1 : 1;
        bool Reset : 1;
        bool Keyhole : 1;
        bool Cyc_bd_en : 1;
        uint32_t rsvd11_5 : 7;
        bool IOC_IrqEn : 1;
        bool Dly_IrqEn : 1;
        bool Err_IrqEn : 1;
        uint32_t rsvd15 : 1;
        uint32_t IRQThreshold : 8;
        uint32_t IRQDelay : 8;
    };

    struct __attribute__((packed)) S2mmDmaStatusReg {
        bool Halted : 1;
        bool Idle : 1;
        uint32_t rsvd2 : 1;
        bool SGIncld : 1;
        bool DMAIntErr : 1;
        bool DMASlvErr : 1;
        bool DMADecErr : 1;
        uint32_t rsvd7 : 1;
        bool SGIntErr : 1;
        bool SGSlvErr : 1;
        bool SGDecErr : 1;
        uint32_t rsvd11 : 1;
        bool IOC_Irq : 1;
        bool Dly_Irq : 1;
        bool Err_Irq : 1;
        uint32_t rsvd15 : 1;
        uint32_t RQThresholdSts : 8;
        uint32_t IRQDelaySts : 8;
    };

    static_assert(sizeof(S2mmDmaControlReg) == 4);
    static_assert(sizeof(S2mmDmaStatusReg) == 4);

    CtkRegWrapper<S2mmDmaControlReg> _ctrlReg;
    CtkRegWrapper<S2mmDmaStatusReg> _statusReg;
    CtkRegWrapper<uint32_t> _curDesc;
    CtkRegWrapper<uint32_t> _curDescMsb;
    CtkRegWrapper<uint32_t> _tailDesc;
    CtkRegWrapper<uint32_t> _tailDescMsb;

    mutable boost::log::sources::severity_logger<blt::severity_level> _slg;

public:
    AxiDmaIf() = delete;
    AxiDmaIf(ChimeraTK::Device& dev)
    : _ctrlReg{ dev, "S2MM.DMACR" }
    , _statusReg{ dev, "S2MM.DMASR" }
    , _curDesc{ dev, "S2MM.CURDESC" }
    , _curDescMsb{ dev, "S2MM.CURDESC_MSB" }
    , _tailDesc{ dev, "S2MM.TAILDESC" }
    , _tailDescMsb{ dev, "S2MM.TAILDESC_MSB" } {}

    void start(uintptr_t start_desc);

    // this has to be done in the backend?
    //  void arm_interrupt();
    //  uint32_t clear_interrupt();
};
