// Copyright (c) 2021 Deutsches Elektronen-Synchrotron DESY

#pragma once

#include "CtkRegWrapper.hpp"

class GpioStatus {
    struct __attribute__((packed)) GpioData {
        bool ddr4_init_calib_complete : 1;
        unsigned reserved : 31;
    };

    CtkRegWrapper<GpioData> _gpioStatus;

public:
    GpioStatus() = delete;
    GpioStatus(ChimeraTK::Device& dev)
    : _gpioStatus{ dev, "GPIO.STATUS" } {}

    bool is_ddr4_init_calib_complete() {
        return _gpioStatus.rd().ddr4_init_calib_complete;
    }
};
