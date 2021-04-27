// Copyright (c) 2021 Deutsches Elektronen-Synchrotron DESY

#include <ios>

#include <boost/log/core/core.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/trivial.hpp>

#include "TrafficGen.hpp"

void TrafficGen::start(uint16_t nr_pkts, uint32_t pkt_size, uint16_t pkt_pause) {
    BOOST_LOG_SEV(_slg, blt::severity_level::trace)
        << "TrafficGen: start, nr pkts = " << nr_pkts << ", pkt size = " << pkt_size;

    stop();

    _cfg.wr({
        .ranlen = 0, .randly = 0, .etkts = 0, .rsvd7_3 = 0, .tdest = 0, .pdly = pkt_pause
    });

    const auto num_beats_reg = pkt_size - 1;
    _trLen.wr(nr_pkts << 16 | (num_beats_reg & 0xffff));
    _exTrLen.wr(num_beats_reg >> 16);

    auto st_ctrl = _ctrl.rd();
    st_ctrl.done = 0;
    st_ctrl.stren = 1;
    _ctrl.wr(st_ctrl);
}

void TrafficGen::stop() {
    auto st_ctrl = _ctrl.rd();
    st_ctrl.stren = 0;
    if (st_ctrl.done) {
        BOOST_LOG_SEV(_slg, blt::severity_level::trace) << "TrafficGen: clearing done bit";
        // W1C – Write 1 to Clear 
        st_ctrl.done = 1;
    }
    _ctrl.wr(st_ctrl);
}

void TrafficGen::print_version() {
    BOOST_LOG_SEV(_slg, blt::severity_level::info)
        << "TrafficGen: version = 0x" << std::hex << _ctrl.rd().version << std::dec;
}
