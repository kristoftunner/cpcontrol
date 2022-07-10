#ifndef DEVICE_COMM_HPP
#define DEVICE_COMM_HPP

#include <vector>

#include <modbuspp.h>
#include "json.hpp"

using json = nlohmann::json;

class ModbusPort {
private:
  std::unique_ptr<Modbus::Master> m_modbus;
public:
  ModbusPort(){}
  bool Initialize(const json& jsonInput);
  void AddSlave(int address);
  
  template<class T>
  std::vector<T> ReadHoldingRegister(int baseAddress, int number, int slaveAddress);
  
  template<class T>
  bool WriteHoldingRegister(int baseAddress, std::vector<T> values, int slaveAddress);

  bool WriteSingleRegister(int baseAddress, uint16_t value, int slaveAddress);
};

template<class T>
std::vector<T> ModbusPort::ReadHoldingRegister(int baseAddress, int number, int slaveAddres)
{
  /* TODO: do some error handling */
  std::vector<T> result;
  if(m_modbus->open())
  {
    std::vector<Modbus::Data<T, Modbus::EndianBigLittle>> data;
    data.resize(number);
    Modbus::Slave& slv = m_modbus->slave(slaveAddres);

    if(slv.readRegisters(baseAddress, data.data(), number))
    {
      for(auto value : data)
        result.push_back(value.value());
      
      return result;
    }
  }
  return result;
}

template<class T>
bool ModbusPort::WriteHoldingRegister(int baseAddress, std::vector<T> values, int slaveAddress)
{
  if(m_modbus->open())
  {
    std::vector<Modbus::Data<T, Modbus::EndianBigLittle>> data;
    Modbus::Slave& slv = m_modbus->slave(slaveAddress);
    for(auto value : values)
    {
      Modbus::Data<T, Modbus::EndianBigLittle> tempData = value;
      data.push_back(tempData);
    }
    slv.writeRegisters(baseAddress, data.data(), data.size());
    return true;
  }
  else
    return false;
}


#endif //DEVICE_COMM_HPP