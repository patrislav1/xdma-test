// Copyright (c) 2021 Deutsches Elektronen-Synchrotron DESY

#include "MemSgdma.hpp"

MemSgdma::MemSgdma(ChimeraTK::Device& dev) {
    // Initialize ChimeraTK accessors for sgdma buffers & descriptors
    uintptr_t offs_buf = 0;
    uintptr_t offs_desc = 0;
    for (int i = 0; i < 32; i++) {
        BOOST_LOG_SEV(_slg, blt::severity_level::info)
            << "MemSgdma: creating accessor #" << i;
        _buffers.emplace_back(
            dev.getOneDRegisterAccessor<int32_t>(
                "FPGA_MEM.SGDMA_BUF",
                BUF_LEN / sizeof(int32_t),
                offs_buf / sizeof(int32_t),
                {ChimeraTK::AccessMode::raw}
            )
        );
        _descs.emplace_back(
            dev.getOneDRegisterAccessor<int32_t>(
                "FPGA_MEM.SGDMA_DESC",
                sizeof(S2mmDesc) / sizeof(int32_t),
                offs_desc / sizeof(int32_t),
                {ChimeraTK::AccessMode::raw}
            )
        );
        _dst_buf_addrs.push_back(fpga_mem_phys_addr + offs_buf);
        offs_buf += BUF_LEN;
        offs_desc += DESC_ADDR_STEP;
    };
};

MemSgdma::S2mmDesc MemSgdma::_rd_desc(size_t i)
{
    _descs[i].read();
    std::vector<int32_t> tmp(_descs[i].getNElements());
    _descs[i].swap(tmp);

    return *reinterpret_cast<S2mmDesc*>(&tmp[0]);
}

void MemSgdma::_wr_desc(size_t i, const S2mmDesc& val)
{
    const int32_t *src = reinterpret_cast<const int32_t*>(&val);

    std::vector<int32_t> tmp(
        src, src + (sizeof(val) / sizeof(int32_t))
    );
    _descs[i].swap(tmp);
    _descs[i].write();
}

void MemSgdma::init_cyc_mode() {
    // Initialize the sgdma descriptors
    _nr_cyc_desc = _dst_buf_addrs.size();
    _next_readable_buf = 0;
    size_t i = 0;
    for (auto dst_buf_addr : _dst_buf_addrs) {
        BOOST_LOG_SEV(_slg, blt::severity_level::trace)
            << "MemSgdma: dest buf addr = 0x" << std::hex << dst_buf_addr << std::dec;

        uintptr_t nxtdesc = (bram_ctrl_0_base | pcie_axi4l_offset) +  ((i + 1) % _nr_cyc_desc) * DESC_ADDR_STEP;

#pragma GCC diagnostic push // We're OK that everything not listed is zero-initialized.
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
        _wr_desc(i++, {
            .nxtdesc = nxtdesc,
            .buffer_addr = dst_buf_addr,
            .control = S2mmDescControl{.buffer_len = BUF_LEN},
            .status = {0},
            .app = {0}
        });
#pragma GCC diagnostic pop
    }
}

void MemSgdma::print_desc(const S2mmDesc &desc) {
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

void MemSgdma::print_descs() {
    for (size_t i = 0; i < _nr_cyc_desc; i++) {
        print_desc(_rd_desc(i));
    }
}

uintptr_t MemSgdma::get_first_desc_addr() const {
    return bram_ctrl_0_base | pcie_axi4l_offset;
}

std::ostream &operator<<(std::ostream &os, const UioRegion &buf_info) {
    os << "UioRegion{0x" << std::hex << buf_info.addr << ", 0x" << buf_info.size << std::dec
       << "}";
    return os;
}

std::vector<int32_t> MemSgdma::get_full_buffers() {
    std::vector<int32_t> result;

    for (size_t i = 0; i < _nr_cyc_desc; i++) {
        S2mmDesc desc = _rd_desc(_next_readable_buf);
        if (!desc.status.cmpit) {
            break;
        }

        _buffers[i].read();
        std::vector<int32_t> tmp(_buffers[i].getNElements());
        _buffers[i].swap(tmp);
        BOOST_LOG_SEV(_slg, blt::severity_level::trace) <<
            "save buf #" << _next_readable_buf << " @ 0x" << std::hex << desc.buffer_addr;

        result.insert(result.end(), tmp.begin(), tmp.begin() + desc.status.buffer_len / sizeof(tmp[0]));

        desc.status.cmpit = 0;
        _wr_desc(_next_readable_buf, desc);

        _next_readable_buf++;
        _next_readable_buf %= _nr_cyc_desc;
    }

    return result;
}
