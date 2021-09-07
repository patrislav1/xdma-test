#pragma once

#include <ChimeraTK/Device.h>
#include <ChimeraTK/Utilities.h>

template<typename T>
class CtkRegWrapper {
    union RegCast {
        uint32_t raw;
        T data;
    };

    ChimeraTK::ScalarRegisterAccessor<uint32_t> _accessor;

public:
    CtkRegWrapper() = delete;
    CtkRegWrapper(ChimeraTK::Device& dev, const std::string& name)
    : _accessor{ dev.getScalarRegisterAccessor<uint32_t>(name) } {}

    uint32_t rd_int() {
        _accessor.read();
        return _accessor;
    }

    void wr_int(uint32_t val) {
        _accessor = val;
        _accessor.write();
    }

    T rd() {
        RegCast tmp = {
            .raw = rd_int()
        };
        return tmp.data;
    }

    void wr(const T val) {
        RegCast tmp = {
            .data = val
        };
        wr_int(tmp.raw);
    }
};
