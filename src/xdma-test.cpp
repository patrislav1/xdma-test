#include <ChimeraTK/Device.h>
#include <ChimeraTK/Utilities.h>
#include <iostream>

#include "GpioStatus.hpp"
#include "MemSgdma.hpp"

int main() {
    ChimeraTK::setDMapFilePath("example.dmap");
    ChimeraTK::Device zupExample("ZUP_EXAMPLE_APP");

    zupExample.open();

    GpioStatus gpioStatus{zupExample};
    std::cout << "DDR init: " << gpioStatus.is_ddr4_init_calib_complete() << std::endl;

    MemSgdma mem_sgdma(zupExample);
    mem_sgdma.init_cyc_mode();

    zupExample.close();

    return 0;
}
