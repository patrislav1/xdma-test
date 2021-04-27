// Copyright (c) 2021 Deutsches Elektronen-Synchrotron DESY

#include "MemSgdma.hpp"

MemSgdma::S2mmDesc &MemSgdma::_desc(size_t i) const
{
    assert(i < _nr_cyc_desc && "desc index out of range");
    char *addr = static_cast<char *>(0 /*_mem*/) + (i * DESC_ADDR_STEP);
    return *(reinterpret_cast<S2mmDesc *>(addr));
}

void MemSgdma::write_cyc_mode(const std::vector<uint64_t> &dst_buf_addrs) {
    _nr_cyc_desc = dst_buf_addrs.size();
    _next_readable_buf = 0;
    size_t i = 0;
    for (auto dst_buf_addr : dst_buf_addrs) {
        BOOST_LOG_SEV(_slg, blt::severity_level::trace)
            << "MemSgdma: dest buf addr = 0x" << std::hex << dst_buf_addr << std::dec;

        uintptr_t nxtdesc = 0 /*_region.addr */ +  ((i + 1) % _nr_cyc_desc) * DESC_ADDR_STEP;

#pragma GCC diagnostic push // We're OK that everything not listed is zero-initialized.
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
        _desc(i++) = {
            .nxtdesc = nxtdesc,
            .buffer_addr = dst_buf_addr,
            .control = S2mmDescControl{.buffer_len = BUF_LEN, .rxeof = 0, .rxsof = 0, .rsvd = 0},
            .status = {0},
            .app = {0}
        };
#pragma GCC diagnostic pop
    }
}

void MemSgdma::print_desc(const S2mmDesc &desc) const {
#define BLI BOOST_LOG_SEV(_slg, blt::severity_level::info) << "MemSgdma: "
    BLI << "S2mmDesc {";
    BLI << "  next desc   = 0x" << std::hex << desc.nxtdesc;
    BLI << "  buffer addr = 0x" << std::hex << desc.buffer_addr;
    BLI << "  control";
    BLI << "    buffer_len = " << std::dec << desc.control.buffer_len;
    BLI << "    sof        = " << std::dec << desc.control.rxsof;
    BLI << "    eof        = " << std::dec << desc.control.rxeof;
    BLI << "  status";
    BLI << "    buffer_len = " << std::dec << desc.status.buffer_len;
    BLI << "    sof        = " << std::dec << desc.status.rxsof;
    BLI << "    eof        = " << std::dec << desc.status.rxeof;
    BLI << "    dmainterr  = " << std::dec << desc.status.dmainterr;
    BLI << "    dmaslverr  = " << std::dec << desc.status.dmaslverr;
    BLI << "    dmadecerr  = " << std::dec << desc.status.dmadecerr;
    BLI << "    cmpit      = " << std::dec << desc.status.cmpit;
    BLI << "}" << std::dec;
}

void MemSgdma::print_descs() const {
    for (size_t i = 0; i < _nr_cyc_desc; i++) {
        print_desc(_desc(i));
    }
}

uintptr_t MemSgdma::get_first_desc_addr() const {
    return 0; // _region.addr;
}

std::ostream &operator<<(std::ostream &os, const UioRegion &buf_info) {
    os << "UioRegion{0x" << std::hex << buf_info.addr << ", 0x" << buf_info.size << std::dec
       << "}";
    return os;
}

std::vector<UioRegion> MemSgdma::get_full_buffers() {
    std::vector<UioRegion> bufs;

    for (size_t i = 0; i < _nr_cyc_desc; i++) {
        S2mmDesc &desc = _desc(_next_readable_buf);
        if (!desc.status.cmpit) {
            break;
        }
        desc.status.cmpit = 0;
        bufs.emplace_back(UioRegion{
                desc.buffer_addr,
                desc.status.buffer_len
            });
        BOOST_LOG_SEV(_slg, blt::severity_level::trace) <<
            "save buf #" << _next_readable_buf << " @ 0x" << std::hex << desc.buffer_addr;

        _next_readable_buf++;
        _next_readable_buf %= _nr_cyc_desc;
    }

    return bufs;
}
