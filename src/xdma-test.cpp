#include <ChimeraTK/Device.h>
#include <ChimeraTK/Utilities.h>
#include <iostream>

#include "GpioStatus.hpp"
#include "MemSgdma.hpp"
#include "AxiDmaIf.hpp"
#include "TrafficGen.hpp"

int main() {
    ChimeraTK::setDMapFilePath("example.dmap");
    ChimeraTK::Device zupExample("ZUP_EXAMPLE_APP");

    zupExample.open();

    GpioStatus gpioStatus{zupExample};
    if (!gpioStatus.is_ddr4_init_calib_complete()) {
        throw std::runtime_error("DDR4 init calib is not complete");
    }

    AxiDmaIf axi_dma{zupExample};
    MemSgdma mem_sgdma{zupExample};
    TrafficGen traffic_gen{zupExample};

    mem_sgdma.init_cyc_mode();
    axi_dma.start(mem_sgdma.get_first_desc_addr());
    traffic_gen.start(1, 1024, 15000);

    zupExample.close();

    return 0;
}
