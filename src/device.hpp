#ifndef DEVICE_HPP
#define DEVICE_HPP

#include <iostream>

#include <vector>
#include <memory>
#include <string>

class BaseDevice
{
public:
  virtual void Initialize(std::string config) = 0;
  virtual void ReadMeasurements() = 0;
  virtual void ParseMessages() = 0;
  virtual std::vector<std::string> GetParsedMessages() = 0;
private:
  std::vector<std::string> topics;
  std::vector<std::string> parsedMessages;
};

class MeterDevice : public BaseDevice
{
public:
  MeterDevice(){};
  virtual void Initialize(std::string address) override;
  virtual void ReadMeasurements() override;
  virtual void ParseMessages() override;
  virtual std::vector<std::string> GetParsedMessages() override;
private:
  unsigned int address;
};

class FroniusInverterDevice : public BaseDevice {
public:
  FroniusInverterDevice();
  virtual void Initialize(std::string address) override;
  virtual void ReadMeasurements() override;
  virtual void ParseMessages() override;
  virtual std::vector<std::string> GetParsedMessages() override;
  
  /* Getter functions for the register map implementation */
  float GetPhaseL1Voltage(){ return phaseVoltageL1;}
  float GetPhaseL2Voltage(){ return phaseVoltageL2;}
  float GetPhaseL3Voltage(){ return phaseVoltageL3;}
  float GetL1toL2Voltage() { return l1Tol2Voltage;}
  float GetL2toL3Voltage() { return l2Tol3Voltage;}
  float GetL1toL3Voltage() { return l1Tol3Voltage;}
  float GetPhaseL1Current(){ return phaseCurrentL1;}
  float GetPhaseL2Current(){ return phaseCurrentL2;}
  float GetPhaseL3Current(){ return phaseCurrentL3;}
  float GetAcFrequency(){ return acFrequency;}
  float GetInverterTemperature(){ return inverterTemperature;}
  unsigned int GetInverterStatus(){ return inverterStatus;}
  unsigned int GetInverterErrorCode() { return inverterErrorCode;}
  float GetDcStackVoltage(){ return dcStackVoltage;}
  float GetDcStackCurrent() { return dcStackCurrent;}
private:
  unsigned int address;
  float phaseVoltageL1;
  float phaseVoltageL2;
  float phaseVoltageL3;
  float l1Tol2Voltage;
  float l2Tol3Voltage;
  float l1Tol3Voltage;
  float phaseCurrentL1;
  float phaseCurrentL2;
  float phaseCurrentL3;
  float acFrequency;
  float inverterTemperature;
  unsigned int inverterStatus;
  unsigned int inverterErrorCode;
  float dcStackVoltage;
  float dcStackCurrent;
};

class DeviceContainer {
private:
  std::vector<BaseDevice*> m_storedDevices;
public:
  DeviceContainer(){};
  void AppendDevice(BaseDevice* const device);
  int GetDeviceCount();
  BaseDevice* GetDevice(int number);
};

#endif //DEVICE_HPP