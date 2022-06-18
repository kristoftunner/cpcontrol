#include "enerman.hpp"

EnermanReturnCode Enerman::BuildDevices(json devicesConfig)
{
  /* build the communication ports for the devices */
  if(devicesConfig.contains("commPorts"))
  {
    for(auto commPort : devicesConfig.at("commPorts"))
    {
      auto name = commPort.at("name").get<std::string>();
      std::shared_ptr<ModbusPort> portInstance = std::make_shared<ModbusPort>();
      portInstance->Initialize(commPort);
      m_modbusCommPortMap[name] = portInstance;
    }
  }

  /* build the device -> append to the container */
  if(devicesConfig.contains("powerMeters"))
  {
    for(auto powerMeter : devicesConfig.at("powerMeters"))
    {
      if(powerMeter.at("model") == "schneiderPM5110")
      {
        SchneiderPM5110Meter *device = new SchneiderPM5110Meter();
        auto commPort = powerMeter.at("interfacePort");
        device->SetCommPort(m_modbusCommPortMap[commPort]);
        if(device->Initialize(powerMeter) == 0)
        {
          PowerMeterDevice *pmDevice = device;
          m_deviceContainer.AppendDevice(pmDevice);
        }
        else
          return EnermanReturnCode::ENERMAN_CONFIG_ERR;
      }
      else
        return EnermanReturnCode::ENERMAN_CONFIG_ERR;
    }
  }

  if(devicesConfig.contains("inverters"))
  {
    for(auto inverter : devicesConfig.at("inverters"))
    {
      auto commPortName = inverter.at("interfacePort").get<std::string>();
      InverterDevice *device = new InverterDevice();
      if(device->Initialize(inverter) == 0)
        m_deviceContainer.AppendDevice(device);
      else
        return EnermanReturnCode::ENERMAN_CONFIG_ERR;
    }
  }

  return EnermanReturnCode::ENERMAN_OK;
}

EnermanReturnCode Enerman::Execute()
{
  if(m_deviceContainer.ReadMeasurements<PowerMeterDevice>() == 0)
    return EnermanReturnCode::ENERMAN_OK;
  else
    return EnermanReturnCode::ENERMAN_READ_ERR;
}