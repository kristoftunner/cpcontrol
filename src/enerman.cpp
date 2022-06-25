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
        auto commPort = powerMeter.at("interfacePort").get<std::string>();
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
      /* right now there is no supported inverter */
    }
  }

  /* setup catchpenny */
  if(SetupCatchpenny(devicesConfig) != 0)
    return EnermanReturnCode::ENERMAN_CONFIG_ERR;
  return EnermanReturnCode::ENERMAN_OK;
}

EnermanReturnCode Enerman::Execute()
{
  if(m_deviceContainer.ReadMeasurements<PowerMeterDevice>() == 0)
    return EnermanReturnCode::ENERMAN_OK;
  else
    return EnermanReturnCode::ENERMAN_READ_ERR;
}

int Enerman::SetupCatchpenny(json& config)
{
  CatchpennyConfig cfg = {
    820,
    450,
    12,
    9,
    820,
    460,
    12,
    9,
    10000,
    10000,
    100000
  };
  CellConfig cellCfg = {45,38,60,5};
  Catchpenny catchpenny = Catchpenny(cfg);
  if(config.contains("catchpenny"))
  {
    json deviceConfig = config["catchpenny"];
    auto commPort = deviceConfig.at("interfacePort").get<std::string>();
    
    /* setup the tesla chargers */
    for(auto charger : deviceConfig.at("chargers"))
    {
      int address = charger.at("address").get<int>();
      Battery battery = Battery(50000, 80, cellCfg, address);
      battery.SetCommPort(m_modbusCommPortMap[commPort]);
      catchpenny.AppendBattery(battery);
      Tesla *inverter = new Tesla();
      inverter->SetCommPort(m_modbusCommPortMap[commPort]);
      if(inverter->Initialize(charger) == 0)
      {
        InverterDevice *invDevice = inverter;
        catchpenny.AppendCharger(invDevice);
      }
      else
        return 1;
    }

    /* setup the fronius dischargers */
    for(auto disCharger : deviceConfig.at("dischargers"))
    {
      int address = disCharger.at("address").get<int>();
      FroniusIgPlus *inverter = new FroniusIgPlus();
      inverter->SetCommPort(m_modbusCommPortMap[commPort]);
      if(inverter->Initialize(disCharger) == 0)
      {
        InverterDevice *invDevice = inverter;
        catchpenny.AppendDischarger(invDevice);
      }
      else
        return 1;
    }
  }
  else
  {
    return 1;
  }

  m_catchpenny = catchpenny; 
  return 0;
}