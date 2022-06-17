#ifndef DEVICE_COMM_HPP
#define DEVICE_COMM_HPP

#include <vector>

#include <modbuspp.h>
#include "json.hpp"

using json = nlohmann::json;

class ModbusPort {
private:
  Modbus::Master m_modbus;
public:
  ModbusPort(){}
  bool Initialize(json& jsonInput);
  
  template<class T>
  std::vector<T> ReadHoldingRegister(int baseAddress, int number);
  
  template<class T>
  bool WriteHoldingRegister(int baseAddress, std::vector<T> values);

  bool WriteSingleRegister(int baseAddress, uint16_t value);
};

#endif //DEVICE_COMM_HPP