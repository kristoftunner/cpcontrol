#include <thread>

#include "enerman.hpp"
#include "config.hpp"

using namespace std::chrono_literals;

EnermanReturnCode Enerman::BuildDevices(const json& devicesConfig)
{
  m_deviceContainer = std::make_shared<DeviceContainer>();
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
        const auto& assetId = powerMeter.at("assetId").get<std::string>();
        device->SetAssetId(assetId);
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
          m_deviceContainer->AppendDevice(pmDevice);
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
  if(SetupCatchpenny(devicesConfig.at("catchpenny"), containerDataMutex) != 0)
    return EnermanReturnCode::ENERMAN_CONFIG_ERR;
  
  /* setup the mqtt port of the system */
  MqttMessageResponder responder;
  responder.SetCathpenny(m_catchpenny);
  responder.SetContainer(m_deviceContainer);
  PahoMqttPort port;
  port.Initailize(catchpenny_config::mqttConfig);
  port.SetMqttResponder(responder);
  /* TODO: all the config informations to a config file */
  port.Subscribe(catchpenny_config::subrscribeTopics);
  m_port = port;
  return EnermanReturnCode::ENERMAN_OK;
}

int Enerman::SetupCatchpenny(const json& config, std::shared_ptr<std::shared_mutex> mutex)
{
  int numberOfChargers,numberOfDischargers;
  numberOfChargers = config.at("chargers").size();
  numberOfDischargers = config.at("dischargers").size();
  
  CatchpennyConfig cfg = {
    numberOfChargers,
    numberOfDischargers,
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

  std::shared_ptr<Catchpenny> catchpenny = std::make_shared<Catchpenny>(cfg, catchpenny_config::systemInfo);
  m_catchpenny = catchpenny;
  const auto& commPort = config.at("interfacePort").get<std::string>();
    
  /* setup the tesla chargers */
  for(const auto& charger : config.at("chargers"))
  {
    int address = charger.at("address").get<int>();
    Battery battery = Battery(50000, 80, catchpenny_config::cellCfg, address);
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

  /* setup the fronius dischargers */
  for(const auto& disCharger : config.at("dischargers"))
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
  if(config.contains("tcpRegisterInterface"))
  {
    const auto& tcpPort = config.at("tcpRegisterInterface");
    const std::string host = tcpPort.at("host");
    const std::string port = tcpPort.at("port");
    m_catchpennyModbusServer.SetCatchpenny(m_catchpenny);
    m_catchpennyModbusServer.Initialize(host, port);
  }
  else
  {
    return 1;
  }

  return 0;
}

void Enerman::ExecEnergyManagment()
{
  while(true)
  {
    if(m_deviceContainer->ReadMeasurements<PowerMeterDevice>() == 0)
      ;
    else
      ;
    /* control the catchpenny */
    m_catchpenny->ReadMeasurements();
    m_catchpenny->UpdateControl();
    std::this_thread::sleep_for(500ms);
  }
}

void Enerman::ExecCatchpennyModbusTcp()
{
  m_catchpennyModbusServer.Process();
}

void Enerman::ExecCatchpennyMqtt()
{
  m_port.Listen();
}
