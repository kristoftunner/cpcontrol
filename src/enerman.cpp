#include "enerman.hpp"
#include <thread>

using namespace std::chrono_literals;

EnermanReturnCode Enerman::BuildDevices(const json& devicesConfig)
{
  /* build the communication ports for the devices */
  if(devicesConfig.contains("commPorts"))
  {
    for(const auto& commPort : devicesConfig.at("commPorts"))
    {
      const auto& name = commPort.at("name").get<std::string>();
      std::shared_ptr<ModbusPort> portInstance = std::make_shared<ModbusPort>();
      portInstance->Initialize(commPort);
      m_modbusCommPortMap[name] = portInstance;
    }
  }

  /* mutex for the devices containted in the containe */
  std::shared_ptr<std::shared_mutex> containerDataMutex = std::make_shared<std::shared_mutex>();

  /* build the device -> append to the container */
  if(devicesConfig.contains("powerMeters"))
  {
    for(const auto& powerMeter : devicesConfig.at("powerMeters"))
    {
      if(powerMeter.at("model") == "schneiderPM5110")
      {
        SchneiderPM5110Meter *device = new SchneiderPM5110Meter();
        const auto& commPort = powerMeter.at("interfacePort").get<std::string>();
        if(m_modbusCommPortMap.contains(commPort))
        {
          device->SetCommPort(m_modbusCommPortMap[commPort]);
        }
        else
        {
          return EnermanReturnCode::ENERMAN_CONFIG_ERR;
        }
        if(device->Initialize(powerMeter) == 0)
        {
          device->SetDataMutex(containerDataMutex);
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
    for(const auto& inverter : devicesConfig.at("inverters"))
    {
      /* right now there is no supported inverter */
    }
  }

  /* setup catchpenny */
  if(SetupCatchpenny(devicesConfig, containerDataMutex) != 0)
    return EnermanReturnCode::ENERMAN_CONFIG_ERR;
  return EnermanReturnCode::ENERMAN_OK;
}

void Enerman::Execute()
{
  while(true)
  {
    if(m_deviceContainer.ReadMeasurements<PowerMeterDevice>() == 0)
      ;
    else
      ;
    /* control the catchpenny */
    m_catchpenny->ReadMeasurements();
    m_catchpenny->UpdateControl();
    std::this_thread::sleep_for(500ms);
  }
}

int Enerman::SetupCatchpenny(const json& config, std::shared_ptr<std::shared_mutex> mutex)
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
  std::shared_ptr<Catchpenny> catchpenny = std::make_shared<Catchpenny>(cfg);
  if(config.contains("catchpenny"))
  {
    json deviceConfig = config["catchpenny"];
    const auto& commPort = deviceConfig.at("interfacePort").get<std::string>();
    
    /* setup the tesla chargers */
    for(const auto& charger : deviceConfig.at("chargers"))
    {
      int address = charger.at("address").get<int>();
      Battery battery = Battery(50000, 80, cellCfg, address);
      battery.SetCommPort(m_modbusCommPortMap[commPort]);
      battery.SetDataMutex(mutex);
      catchpenny->AppendBattery(battery);
      Tesla *inverter = new Tesla();
      inverter->SetCommPort(m_modbusCommPortMap[commPort]);
      if(inverter->Initialize(charger) == 0)
      {
        inverter->SetDataMutex(mutex);
        InverterDevice *invDevice = inverter;
        catchpenny->AppendCharger(invDevice);
      }
      else
        return 1;
    }
    m_catchpenny = catchpenny;

    /* setup the fronius dischargers */
    for(const auto& disCharger : deviceConfig.at("dischargers"))
    {
      int address = disCharger.at("address").get<int>();
      FroniusIgPlus *inverter = new FroniusIgPlus();
      inverter->SetCommPort(m_modbusCommPortMap[commPort]);
      if(inverter->Initialize(disCharger) == 0)
      {
        inverter->SetDataMutex(mutex);
        InverterDevice *invDevice = inverter;
        catchpenny->AppendDischarger(invDevice);
      }
      else
        return 1;
    }

    /* setup the modbusTCP register map */
  if(config.contains("commPorts"))
  {
    for(const auto& commPort : config.at("commPorts"))
    {
      if(commPort.at("mode").get<std::string>() == "tcp")
      {
        const std::string host = commPort.at("connection");
        const std::string port = commPort.at("port");
        m_catchpennyModbusServer.Initialize(host, port);
        m_catchpennyModbusServer.SetCatchpenny(m_catchpenny);
      }
    }
  }
  }
  else
  {
    return 1;
  }

  m_catchpenny = catchpenny; 
  return 0;
}