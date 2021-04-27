// Copyright (c) 2021 Deutsches Elektronen-Synchrotron DESY

#pragma once

#include <ChimeraTK/Device.h>
#include <ChimeraTK/Utilities.h>

#include "MemSgdma.hpp"

class FpgaMemBuffer {
    uintptr_t _phys_addr;

public:
    explicit FpgaMemBuffer(ChimeraTK::Device& dev, uintptr_t phys_addr)
    : _phys_addr{phys_addr} {}

    uintptr_t get_phys_addr() const {
        return _phys_addr;
    }

    void copy_from_buf(const UioRegion &buf_info, std::vector<uint8_t> &out) const {
        size_t old_size = out.size();
        size_t new_size = old_size + buf_info.size;
        out.resize(new_size);
//        lseek(_dma_fd, buf_info.addr, SEEK_SET);
//        ssize_t rc = read(_dma_fd, out.data() + old_size, buf_info.size);
//        if (rc < static_cast<ssize_t>(buf_info.size)) {
            // TODO: error handling
//        }
    }
};
