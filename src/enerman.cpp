#include "enerman.hpp"

EnermanReturnCode Enerman::BuildDevices(json devicesConfig)
{
  /* build the communication ports for the devices */
  for(auto commPort : devicesConfig.at("commPorts"))
  {
    auto name = commPort.at("name").get<std::string>();
    m_modbusCommPortMap[name] = std::make_shared<ModbusPort>(ModbusPort(commPort));
  }

  /* build the device -> append to the container */
  for(auto powerMeter : devicesConfig.at("powerMeters"))
  {
    auto commPortName = powerMeter.at("interfacePort").get<std::string>();
    PowerMeterDevice *device = new PowerMeterDevice(m_modbusCommPortMap.at(commPortName));
    if(device->Initialize(powerMeter) == 0)
      m_deviceContainer.AppendDevice(device);
    else
      return EnermanReturnCode::ENERMAN_CONFIG_ERR;
  }

  for(auto inverter : devicesConfig.at("inverters"))
  {
    auto commPortName = inverter.at("interfacePort").get<std::string>();
    InverterDevice *device = new InverterDevice(m_modbusCommPortMap.at(commPortName));
    if(device->Initialize(inverter) == 0)
      m_deviceContainer.AppendDevice(device);
    else
      return EnermanReturnCode::ENERMAN_CONFIG_ERR;
  }

  return EnermanReturnCode::ENERMAN_OK;
}

EnermanReturnCode Enerman::Execute()
{

}