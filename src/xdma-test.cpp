#include <ChimeraTK/Device.h>
#include <ChimeraTK/Utilities.h>
#include <iostream>

#include "GpioStatus.hpp"

int main() {
  ChimeraTK::setDMapFilePath("example.dmap");
  ChimeraTK::Device zupExample("ZUP_EXAMPLE_APP");

  zupExample.open();

  GpioStatus gpioStatus{zupExample};

  std::cout << "DDR init: " << gpioStatus.is_ddr4_init_calib_complete() << std::endl;

  zupExample.close();

  return 0;
}
