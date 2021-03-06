#ifndef ENERMAN_HPP
#define ENERMAN_HPP
#include <vector>
#include <map>

#include "device.hpp"
#include "catchpenny.hpp"
#include "mqtt_port.hpp"
#include "error_tracker.hpp"

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
  std::shared_ptr<DeviceContainer> m_deviceContainer;
  std::map<std::string, std::shared_ptr<ModbusPort>> m_modbusCommPortMap;
  std::shared_ptr<Catchpenny> m_catchpenny;
  CatchpennyModbusTcpServer m_catchpennyModbusServer;
  std::shared_ptr<ErrorTracker> m_et;
  PahoMqttPort m_port;
  
  int SetupCatchpenny(const json& config, std::shared_ptr<std::shared_mutex> mutex);
public:
  Enerman(std::shared_ptr<ErrorTracker> tracker) :m_et(tracker){}
  EnermanReturnCode BuildDevices(const json& devicesConfig);
  /**
   * @brief this function should be run in a separate thread 
   * 
   * @return EnermanReturnCode 
   */
  void ExecEnergyManagment();
  void ExecCatchpennyModbusTcp();
  void ExecCatchpennyMqtt();
  CatchpennyModbusTcpServer& GetCatchpennyModbusServer(){return m_catchpennyModbusServer;}
};

#endif //ENERMAN_HPP