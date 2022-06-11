#include "device.hpp"
#include <string>

int main()
{
  std::string address = "2";
  MeterDevice *a = new MeterDevice();
  FroniusInverterDevice *f1 = new FroniusInverterDevice();
  DeviceContainer container = DeviceContainer();

  container.AppendDevice(a);
  container.AppendDevice(f1);

  int devices = container.GetDeviceCount();
  for(int i = 0; i < devices; i++)
  {
    BaseDevice *base = container.GetDevice(i);
    base->ReadMeasurements();
  }
}