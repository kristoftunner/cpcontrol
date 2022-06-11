#include "device.hpp"

void MeterDevice::Initialize(std::string address)
{
  this->address = std::stoi(address);
  std::cout << "hello from MeterDevice " << std::endl;
}

void MeterDevice::ReadMeasurements() {
  std::cout << "read meter measurements" << std::endl;
}

void MeterDevice::ParseMessages(){}

std::vector<std::string> MeterDevice::GetParsedMessages()
{
  std::vector<std::string> temp; 
  return temp;
}

FroniusInverterDevice::FroniusInverterDevice(){}

void FroniusInverterDevice::Initialize(std::string address)
{
  this->address = std::stoi(address);
}

void FroniusInverterDevice::ReadMeasurements() {
  std::cout << "read fronius measurements" << std::endl;
}

void FroniusInverterDevice::ParseMessages(){}

std::vector<std::string> FroniusInverterDevice::GetParsedMessages()
{
  std::vector<std::string> temp; 
  return temp;
}

void DeviceContainer::AppendDevice(BaseDevice* const device)
{
  if(device != nullptr)
    m_storedDevices.push_back(device);
}

int DeviceContainer::GetDeviceCount()
{
  return m_storedDevices.size();
}

BaseDevice* DeviceContainer::GetDevice(int number)
{
  if(number < m_storedDevices.size())
    return m_storedDevices[number];
  else
    return nullptr;
}

