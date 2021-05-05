#include <ChimeraTK/Device.h>
#include <ChimeraTK/Utilities.h>
#include <iostream>

#include <chrono>
#include <thread>
#include <optional>
#include <boost/program_options.hpp>

#include "GpioStatus.hpp"
#include "MemSgdma.hpp"
#include "AxiDmaIf.hpp"
#include "TrafficGen.hpp"
#include "AxiTrafficGenLfsr.hpp"

namespace bpo = boost::program_options;

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

int main(int argc, char *argv[]) {
    bpo::options_description desc("AXI DMA demo");
    bool debug, trace;
    uint16_t pkt_pause;
    uint16_t nr_pkts;
    uint32_t pkt_len;

    // clang-format off
    desc.add_options()
    ("help,h", "this help")
    ("debug", bpo::bool_switch(&debug), "enable verbose output (debug level)")
    ("trace", bpo::bool_switch(&trace), "enable even more verbose output (trace level)")
    ("pkt_pause", bpo::value<uint16_t>(&pkt_pause)->default_value(10), "pause between pkts - see AXI TG user's manual")
    ("nr_pkts", bpo::value<uint16_t>(&nr_pkts)->default_value(1), "number of packets to generate - see AXI TG user's manual")
    ("pkt_len", bpo::value<uint32_t>(&pkt_len)->default_value(1024), "packet length - see AXI TG user's manual")
    ;
    // clang-format on

    bpo::variables_map vm;
    bpo::store(bpo::parse_command_line(argc, argv, desc), vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 0;
    }

    bpo::notify(vm);

    ChimeraTK::setDMapFilePath("example.dmap");
    ChimeraTK::Device zupExample("ZUP_EXAMPLE_APP");

    zupExample.open();

    GpioStatus gpioStatus{zupExample};
    if (!gpioStatus.is_ddr4_init_calib_complete()) {
        throw std::runtime_error("DDR4 init calib is not complete");
    }

    AxiDmaIf axi_dma{zupExample};
    MemSgdma mem_sgdma{zupExample, 16 * pkt_len};
    TrafficGen traffic_gen{zupExample};

    mem_sgdma.init_cyc_mode();
    axi_dma.start(mem_sgdma.get_first_desc_addr());
    traffic_gen.start(nr_pkts, pkt_len, pkt_pause);

    int i = 10000;
    while (i--) {
        auto res = mem_sgdma.get_full_buffers();
        if (!res.empty()) {
            if (!check_vals(res)) {
                break;
            }
        }
    }
    zupExample.close();

    return 0;
}
