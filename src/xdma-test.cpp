#include <ChimeraTK/Device.h>
#include <ChimeraTK/Utilities.h>
#include <iostream>

#include <chrono>
#include <thread>
#include <optional>

#include "GpioStatus.hpp"
#include "MemSgdma.hpp"
#include "AxiDmaIf.hpp"
#include "TrafficGen.hpp"
#include "AxiTrafficGenLfsr.hpp"

using namespace std::chrono_literals;

static void vec_dump(const std::vector<int32_t> &vec)
{
    constexpr size_t wordsPerLine = 8;
    size_t n;
    for (n = 0; n < vec.size(); n++) {
        if (!(n % wordsPerLine)) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << n * sizeof(vec[0]) << ":";
        }
        std::cout << " " << std::hex << std::setw(8) << std::setfill('0') << vec[n];
        if ((n % wordsPerLine) == wordsPerLine - 1) {
            std::cout << "\n";
        }
    }
    if ((n % wordsPerLine) != wordsPerLine) {
        std::cout << "\n";
    }
}

bool check_vals(const std::vector<int32_t> &words)
{
//    vec_dump(words);

    static std::optional<AxiTrafficGenLfsr> lfsr;

    const uint16_t *vals = reinterpret_cast<const uint16_t *>(&words[0]);
    const size_t numBytes = words.size() * sizeof(words[0]);

    if (!lfsr) {
        uint16_t seed = vals[0];
        lfsr = AxiTrafficGenLfsr{seed};
    }

    for (unsigned int i = 0; i < numBytes / sizeof(vals[0]) / 8; i++) {
        uint16_t exp_val = lfsr->get();
        for (int j = 0; j < 8; j++) {
            uint16_t recv_val = vals[i * 8 + j];
            if (exp_val != recv_val) {
                std::cerr
                    << "mismatch, at " << i * 8 + j << " recv = " << std::hex << recv_val
                    << ", exp = " << exp_val << "\n";
                return false;
            }
        }
        lfsr->advance();
    }
    return true;
}

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
    traffic_gen.start(8, 1024, 65000);

    int i = 100;
    while (i--) {
        std::this_thread::sleep_for(1ms);
        auto res = mem_sgdma.get_full_buffers();
        if (!res.empty()) {
            std::cout << std::dec << "res: " << res.size() * sizeof(res[0]) << std::endl;
            if (!check_vals(res)) {
                break;
            }
        }
    }
    zupExample.close();

    return 0;
}
