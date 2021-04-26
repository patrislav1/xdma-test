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
    : _accessor{dev.getScalarRegisterAccessor<uint32_t>(name)} {}

    T rd() {
        _accessor.read();
        RegCast tmp = {
            .raw = _accessor
        };
        return tmp.data;
    }

    void wr(const T val) {
        RegCast tmp = {
            .data = val
        };
        _accessor = tmp.raw;
        _accessor.write();
    }
};
