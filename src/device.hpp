#ifndef DEVICE_HPP
#define DEVICE_HPP

#include <iostream>
#include <vector>
#include <map>

#include "device_comm.hpp"

enum class Devicetype {
  powerMeter,
  inverter,
  heatpump
};

struct PowerMeterData {
public:
  PowerMeterData() : 
    powerAcTotal(0),
    powerAcPhase1(0),
    powerAcPhase2(0),
    powerAcPhase3(0),
    apparentPowerTotal(0),
    apparentPowerPhase1(0),
    apparentPowerPhase2(0),
    apparentPowerPhase3(0),
    reactivePowerTotal(0),
    reactivePowerPhase1(0),
    reactivePowerPhase2(0),
    reactivePowerPhase3(0),
    currentAcPhase1(0),
    currentAcPhase2(0),
    currentAcPhase3(0),
    voltageAcPhase1(0),
    voltageAcPhase2(0),
    voltageAcPhase3(0),
    frequency(0){}

  std::string assetId;
  float powerAcTotal;
  float powerAcPhase1;
  float powerAcPhase2;
  float powerAcPhase3;
  
  float apparentPowerTotal;
  float apparentPowerPhase1;
  float apparentPowerPhase2;
  float apparentPowerPhase3;
  
  float reactivePowerTotal;
  float reactivePowerPhase1;
  float reactivePowerPhase2;
  float reactivePowerPhase3;
  
  float currentAcPhase1;
  float currentAcPhase2;
  float currentAcPhase3;
  float voltageAcPhase1; 
  float voltageAcPhase2; 
  float voltageAcPhase3; 

  float frequency;
};

struct InverterData {
public:
  InverterData(){
    powerDc = 0;
    currentDc = 0;
    voltageDc = 0;

    powerAcTotal = 0;
    powerAcPhase1 = 0;
    powerAcPhase2 = 0;
    powerAcPhase3 = 0;

    currentAcPhase1 = 0;
    currentAcPhase2 = 0;
    currentAcPhase3 = 0;
    voltageAcPhase1 = 0; 
    voltageAcPhase2 = 0; 
    voltageAcPhase3 = 0; 

    inverterTemperature = 0;
    inverterStatus = 0;
    inverterError = 0;
    frequency = 0;
  }
  std::string assetId;
  float powerDc;
  float currentDc;
  float voltageDc;

  float powerAcTotal;
  float powerAcPhase1;
  float powerAcPhase2;
  float powerAcPhase3;

  float currentAcPhase1;
  float currentAcPhase2;
  float currentAcPhase3;
  float voltageAcPhase1; 
  float voltageAcPhase2; 
  float voltageAcPhase3; 

  float frequency;

  float inverterTemperature;
  uint32_t inverterStatus;
  uint32_t inverterError;
};

class BaseDevice
{
protected:
  Devicetype m_type;
public:
  virtual int ReadMeasurements() = 0;
};

class PowerMeterDevice : public BaseDevice {
protected:
  PowerMeterData m_data;
public:
  PowerMeterDevice() {m_type = Devicetype::powerMeter;}
  virtual int Initialize(json& config) = 0;
  virtual int ReadMeasurements() override {};
  virtual PowerMeterData GetPowerMeterData() {return m_data;};
};

class SchneiderPM5110Meter : public PowerMeterDevice 
{
private:
/* define the register addresses here: */
  const uint32_t currentPhase1Reg = 3000;
  const uint32_t currentPhase2Reg = 3002;
  const uint32_t currentPhase3Reg = 3004;
  const uint32_t voltagePhase1Reg = 3028;
  const uint32_t voltagePhase2Reg = 3030;
  const uint32_t voltagePhase3Reg = 3032;
  const uint32_t powerAcPhase1Reg = 3054;
  const uint32_t powerAcPhase2Reg = 3056;
  const uint32_t powerAcPhase3Reg = 3058;
  const uint32_t powerAcTotalReg  = 3060;
  const uint32_t powerReactivePhase1Reg = 3062;
  const uint32_t powerReactivePhase2Reg = 3064;
  const uint32_t powerReactivePhase3Reg = 3066;
  const uint32_t powerReactiveTotalReg = 3068;
  const uint32_t powerApparentPhase1Reg = 3070;
  const uint32_t powerApparentPhase2Reg = 3072;
  const uint32_t powerApparentPhase3Reg = 3074;
  const uint32_t powerApparentTotalReg = 3076;
  const uint32_t frequencyReg = 3110;

  std::shared_ptr<ModbusPort> m_commPort;
  int m_address;
public:
  SchneiderPM5110Meter(){m_type = Devicetype::powerMeter;}
  virtual int Initialize(json& config) override;
  virtual int ReadMeasurements() override;
  virtual PowerMeterData GetPowerMeterData() {return m_data;}

  void SetCommPort(std::shared_ptr<ModbusPort> commPort) {m_commPort = commPort;}
};

class InverterDevice : public BaseDevice {
protected:
  InverterData m_data;
public:
  InverterDevice() {m_type = Devicetype::inverter;}

  virtual int Initialize(json config) = 0;
  virtual int ReadMeasurements() override {};
  virtual InverterData GetInverterData() {return m_data;}
  virtual void UpdatePower(float powerSetpoint);
};

class DeviceContainer {
private:
  std::vector<PowerMeterDevice*> m_powerMeterContainer;
  std::vector<InverterDevice*> m_inverterContainer;
  
  template<int (PowerMeterDevice::*functor)()>
  int ForEachDevice();
  
  template<int (InverterDevice::*functor)()>
  int ForEachDevice();

public:
  DeviceContainer(){}
  void AppendDevice(PowerMeterDevice *device) {
    m_powerMeterContainer.push_back(device);
  }
  
  void AppendDevice(InverterDevice *device) {
    m_inverterContainer.push_back(device);
  }

  template<class T>
  int ReadMeasurements(){ return ForEachDevice<&T::ReadMeasurements>();}
};

template<int (PowerMeterDevice::*functor)()>
int DeviceContainer::ForEachDevice()
{
  int containerSize = m_powerMeterContainer.size();
  for(int i = 0; i < containerSize; i++)
  {
    PowerMeterDevice *device = m_powerMeterContainer[i];
    (device->*functor)();
  }

  return 0;
}

template<int (InverterDevice::*functor)()>
int DeviceContainer::ForEachDevice()
{
  int containerSize = m_inverterContainer.size();
  for(int i = 0; i < containerSize; i++)
  {
    InverterDevice *device = m_inverterContainer[i];
    (device->*functor)();
  }
  
  return 0;
}

#endif //DEVICE_HPP