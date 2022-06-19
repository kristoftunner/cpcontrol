#ifndef ENERMAN_HPP
#define ENERMAN_HPP
#include <vector>
#include <map>

#include "device.hpp"

enum class EnermanReturnCode{
  ENERMAN_OK,
  ENERMAN_CONFIG_ERR,
  ENERMAN_READ_ERR
};

/**
 * @brief energy managment system controller class
 * 
 */
class Enerman {
private:
  DeviceContainer m_deviceContainer;
  std::map<std::string, std::shared_ptr<ModbusPort>> m_modbusCommPortMap;
public:
  EnermanReturnCode BuildDevices(json devicesConfig);
  EnermanReturnCode Execute();
  EnermanReturnCode ExtractMeasurements();
};

#endif //ENERMAN_HPP