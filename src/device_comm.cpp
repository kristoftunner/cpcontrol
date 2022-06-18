#include "device_comm.hpp"

bool ModbusPort::Initialize(json& jsonInput)
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
    auto port = jsonInput.at("port").get<std::string>();
    auto settings = jsonInput.at("settings").get<std::string>();
    m_modbus = std::make_unique<Modbus::Master>(Modbus::Rtu, port, settings);
    
    auto serialMode = jsonInput.at("serialMode").at("mode").get<Modbus::SerialMode>();
    auto rts = jsonInput.at("rts").get<Modbus::SerialRts>();
    m_modbus->rtu().setRts(rts);
    m_modbus->rtu().setSerialMode(serialMode);

    if(jsonInput.contains("responseTimout"))
    {
      auto responseTimeout = jsonInput.at("responseTimout").get<double>();
      m_modbus->setResponseTimeout(responseTimeout);
    }
    if(jsonInput.contains("byteTimeout"))
    {
      auto byteTimeout = jsonInput.at("byteTimout").get<double>();
      m_modbus->setByteTimeout(byteTimeout);
    }

    return true;
  }
  else if(jsonInput.at("mode") == "tcp")
  {
    auto port = jsonInput.at("port").get<std::string>();
    auto connection = jsonInput.at("connection").get<std::string>();
    m_modbus = std::make_unique<Modbus::Master>(Modbus::Tcp, connection, port);
    
    if(jsonInput.contains("responseTimout"))
    {
      auto responseTimeout = jsonInput.at("responseTimout").get<double>();
      m_modbus->setResponseTimeout(responseTimeout);
    }
    if(jsonInput.contains("byteTimeout"))
    {
      auto byteTimeout = jsonInput.at("byteTimout").get<double>();
      m_modbus->setByteTimeout(byteTimeout);
    }

    return true;
  }
  else
    return false;
}

void ModbusPort::AddSlave(int address)
{
  /* right now this only works for RS485 slaves, TODO: support for modbustcp */
  m_modbus->addSlave(address);
}

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
  else
  {
    return result;
  }
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
