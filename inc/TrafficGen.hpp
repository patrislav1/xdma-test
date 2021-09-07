// Copyright (c) 2021 Deutsches Elektronen-Synchrotron DESY

#pragma once

#include <cstdint>

#include "CtkRegWrapper.hpp"

namespace blt = boost::log::trivial;

class TrafficGen {
    struct __attribute__((packed)) StControl {
        bool stren : 1;
        bool done : 1;
        uint32_t rsvd : 22;
        uint32_t version : 8;
    };

    struct __attribute__((packed)) StConfig {
        bool ranlen : 1;
        bool randly : 1;
        bool etkts : 1;
        uint32_t rsvd7_3 : 5;
        uint32_t tdest : 8;
        uint32_t pdly : 16;
    };

    CtkRegWrapper<StControl> _ctrl;
    CtkRegWrapper<StConfig> _cfg;
    CtkRegWrapper<uint32_t> _trLen;
    CtkRegWrapper<uint32_t> _exTrLen;

    mutable boost::log::sources::severity_logger<blt::severity_level> _slg;

public:
    TrafficGen() = delete;
    TrafficGen(ChimeraTK::Device& dev)
    : _ctrl{ dev, "TRAFFIC_GEN.ST_CTRL" }
    , _cfg{ dev, "TRAFFIC_GEN.ST_CONFIG" }
    , _trLen{ dev, "TRAFFIC_GEN.TR_LEN" }
    , _exTrLen{ dev, "TRAFFIC_GEN.EX_TR_LEN" } {}

    void start(uint16_t nr_pkts, uint32_t pkt_size, uint16_t pkt_pause);
    void stop();

    void print_version();
};
