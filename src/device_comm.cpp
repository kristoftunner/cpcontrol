#include "device_comm.hpp"

bool ModbusPort::Initialize(const json& jsonInput)
{
  /* the following parameters of ModbusMaster should be set:
  * - mode, settings, connection -> via the constructor - required
  * - recovery-link for TCP - optional
  * - responseTimeout, byteTimeout for both RTU and TCP - optional
  * - debug can be set for idk reason - optional
  * - serialMode and rts for RTU - required
  */
  if(jsonInput.at("mode") == "rtu")
  {
    const auto& port = jsonInput.at("port").get<std::string>();
    const auto& settings = jsonInput.at("settings").get<std::string>();
    m_modbus = std::make_unique<Modbus::Master>(Modbus::Rtu, port, settings);
    
    if(jsonInput.contains("serialMode")) /* optional parameter*/
    {
      auto serialMode = jsonInput.at("serialMode").get<std::string>();
      Modbus::SerialMode serialModeconfig;    
      if(serialMode == "rs232")
        serialModeconfig = Modbus::Rs232;
      else if(serialMode == "rs485")
        serialModeconfig = Modbus::Rs485;
      else
        return false;
      
      m_modbus->rtu().setSerialMode(serialModeconfig);
    }
    
    if(jsonInput.contains("rts")) /* optional parameter */
    {
      auto rts = jsonInput.at("rts").get<std::string>();
      Modbus::SerialRts rtsConfig;
      if(rts == "rtsNone")
        rtsConfig = Modbus::RtsNone;
      else if(rts =="rtsUp")
        rtsConfig = Modbus::RtsUp;
      else if(rts == "rtsDown")
        rtsConfig = Modbus::RtsDown;
      else 
        return false;

      m_modbus->rtu().setRts(rtsConfig);
    }

    if(jsonInput.contains("responseTimeout"))
    {
      auto responseTimeout = jsonInput.at("responseTimeout").get<int>();
      m_modbus->setResponseTimeout(responseTimeout);
    }
    if(jsonInput.contains("byteTimeout"))
    {
      auto byteTimeout = jsonInput.at("byteTimeout").get<int>();
      m_modbus->setByteTimeout(byteTimeout);
    }

    return true;
  }
  else if(jsonInput.at("mode") == "tcp")
  {
    auto port = jsonInput.at("port").get<std::string>();
    auto connection = jsonInput.at("host").get<std::string>();
    m_modbus = std::make_unique<Modbus::Master>(Modbus::Tcp, connection, port);
    
    if(jsonInput.contains("responseTimout"))
    {
      auto responseTimeout = jsonInput.at("responseTimeout").get<int>();
      m_modbus->setResponseTimeout(responseTimeout);
    }
    if(jsonInput.contains("byteTimeout"))
    {
      auto byteTimeout = jsonInput.at("byteTimeout").get<int>();
      m_modbus->setByteTimeout(byteTimeout);
    }

    return true;
  }
  else
    return false;
}

void ModbusPort::AddSlave(int address)
{
  /* right now this only works for RS485 slaves, TODO: support for modbustcp(ONLY IF NEEDED) */
  m_modbus->addSlave(address);
}

bool ModbusPort::WriteSingleRegister(int baseAddress, uint16_t value, int slaveAddress)
{
  if(m_modbus->open())
  {
    Modbus::Slave& slv = m_modbus->slave(slaveAddress);
    slv.writeRegister(baseAddress, value);
    return true;
  }
  else
    return false;
}
