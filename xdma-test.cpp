#include <ChimeraTK/Device.h>
#include <ChimeraTK/Utilities.h>
#include <iostream>

int main() {
  ChimeraTK::setDMapFilePath("example.dmap");
  ChimeraTK::Device zupExample("ZUP_EXAMPLE_APP");

  zupExample.open();
  ChimeraTK::ScalarRegisterAccessor<uint32_t> gpioStatus =
      zupExample.getScalarRegisterAccessor<uint32_t>("GPIO.STATUS");

  gpioStatus.read();
  std::cout << "Current gpio status is " << gpioStatus << std::endl;

  zupExample.close();

  return 0;
}
