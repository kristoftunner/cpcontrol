#ifndef DEVICE_HPP
#define DEVICE_HPP

#include <iostream>
#include <vector>
#include <map>

enum class Devicetype {
  meter,
  inverter,
  heatpump
};

class BaseDevice
{
protected:
  Devicetype m_type;
public:
  virtual int Initialize(std::map<std::string,std::string> config) = 0;
  virtual int ReadMeasurements() = 0;
  virtual ~BaseDevice(){}
};

class MeterDevice : public BaseDevice
{
public:
  MeterDevice(){};
  virtual int Initialize(std::map<std::string,std::string> config) override;
  virtual int ReadMeasurements() override;
};

class InverterDevice : public BaseDevice {
public:
  InverterDevice(){
    m_type = Devicetype::inverter;
  }

  virtual int Initialize(std::map<std::string,std::string> config) override;
  virtual int ReadMeasurements() override;
  virtual ~InverterDevice() override;
};

class DeviceContainer {
private:
  std::vector<BaseDevice*> m_container;
  
  template<int (BaseDevice::*functor)()>
  int ForEachDevice();

public:
  DeviceContainer(){}
  void AppendDevice(BaseDevice *device) {
    m_container.push_back(device);
  }

  int ReadMeasurements(){ return ForEachDevice<&BaseDevice::ReadMeasurements>();}
};

#endif //DEVICE_HPP