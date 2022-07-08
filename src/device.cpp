#include "device.hpp"

int SchneiderPM5110Meter::Initialize(const json& config) 
{
  m_address = config.at("address").get<int>();
  return 0;
}

int SchneiderPM5110Meter::ReadMeasurements()
{
  std::vector<float> currentValues = m_commPort->ReadHoldingRegister<float>(currentPhase1Reg,3,m_address);
  if(currentValues.size() != 3)
  {
    //const Error error = {ErrorType::ERROR_MODBUS_READ, ErrorSeverityLevel::ERROR_WARNING};
    //m_et->PushBackError(error);
    return 1;
  }
  else
  {
    m_data.currentAcPhase1 = currentValues[0];
    m_data.currentAcPhase2 = currentValues[1];
    m_data.currentAcPhase3 = currentValues[2];
  }
  
  std::vector<float> voltageValues = m_commPort->ReadHoldingRegister<float>(voltagePhase1Reg,3,m_address);
  if(voltageValues.size() != 3)
  {
    //const Error error = {ErrorType::ERROR_MODBUS_READ, ErrorSeverityLevel::ERROR_WARNING};
    //m_et->PushBackError(error);
    return 1;
  }
  else
  {
    m_data.voltageAcPhase1 = voltageValues[0];
    m_data.voltageAcPhase2 = voltageValues[1];
    m_data.voltageAcPhase3 = voltageValues[2];
  }

  std::vector<float> powerValues = m_commPort->ReadHoldingRegister<float>(powerAcPhase1Reg,12,m_address);
  if(powerValues.size() != 12)
  {
    //const Error error = {ErrorType::ERROR_MODBUS_READ, ErrorSeverityLevel::ERROR_WARNING};
    //m_et->PushBackError(error);
    return 1;
  }
  else
  {
    m_data.powerAcPhase1 = powerValues[0];
    m_data.powerAcPhase2 = powerValues[1];
    m_data.powerAcPhase3 = powerValues[2];
    m_data.powerAcTotal = powerValues[3];
    m_data.reactivePowerPhase1 = powerValues[4];
    m_data.reactivePowerPhase2 = powerValues[5];
    m_data.reactivePowerPhase3 = powerValues[6];
    m_data.reactivePowerTotal = powerValues[7];
    m_data.apparentPowerPhase1 = powerValues[8];
    m_data.apparentPowerPhase2 = powerValues[9];
    m_data.apparentPowerPhase3 = powerValues[10];
    m_data.apparentPowerTotal = powerValues[11];
  }

  std::vector<float> frequency = m_commPort->ReadHoldingRegister<float>(frequencyReg,1,m_address);
  if(frequency.size() != 1)
  {
    //const Error error = {ErrorType::ERROR_MODBUS_READ, ErrorSeverityLevel::ERROR_WARNING};
    //m_et->PushBackError(error);
    return 1;
  }
  else
  {
    m_data.frequency = frequency[0];
  }

  return 0;      
}

int FroniusIgPlus::Initialize(const json& config)
{
  m_address = config.at("address").get<int>();
  return 0;
}

int FroniusIgPlus::ReadMeasurements() 
{
  std::vector<uint16_t> currentValues = m_commPort->ReadHoldingRegister<uint16_t>(acCurrentRegBase,5,m_address);
  if(currentValues.size() != 5)
  {
    //const Error error = {ErrorType::ERROR_MODBUS_READ, ErrorSeverityLevel::ERROR_WARNING};
    //m_et->PushBackError(error);
    return 1;
  }
  else
  {
    float scaleFactor = static_cast<float>(static_cast<int16_t>(currentValues[4]));
    m_data.currentAcPhase1 = static_cast<float>(currentValues[1]) * pow(10, scaleFactor);
    m_data.currentAcPhase2 = static_cast<float>(currentValues[2]) * pow(10, scaleFactor);
    m_data.currentAcPhase3 = static_cast<float>(currentValues[3]) * pow(10, scaleFactor);
  }

  std::vector<uint16_t> voltageAndPowerValues = m_commPort->ReadHoldingRegister<uint16_t>(acVoltageBase,14,m_address);
  if(voltageAndPowerValues.size() != 14)
  {
    //const Error error = {ErrorType::ERROR_MODBUS_READ, ErrorSeverityLevel::ERROR_WARNING};
    //m_et->PushBackError(error);
    return 1;
  }
  else
  {
    /* scale values */
    float voltageScale = static_cast<float>(static_cast<int16_t>(voltageAndPowerValues[3]));
    float acPowerScale = static_cast<float>(static_cast<int16_t>(voltageAndPowerValues[5]));
    float acFrequencyScale = static_cast<float>(static_cast<int16_t>(voltageAndPowerValues[7]));
    float acApparentPowerScale = static_cast<float>(static_cast<int16_t>(voltageAndPowerValues[9]));
    float acReactivePowerScale = static_cast<float>(static_cast<int16_t>(voltageAndPowerValues[11]));
    float powerFactorScale = static_cast<float>(static_cast<int16_t>(voltageAndPowerValues[13]));
  
    m_data.voltageAcPhase1 = static_cast<float>(voltageAndPowerValues[0]) * pow(10, voltageScale);
    m_data.voltageAcPhase2 = static_cast<float>(voltageAndPowerValues[1]) * pow(10, voltageScale);
    m_data.voltageAcPhase3 = static_cast<float>(voltageAndPowerValues[2]) * pow(10, voltageScale);
    m_data.powerAcTotal = static_cast<float>(voltageAndPowerValues[4]) * pow(10, acPowerScale);
    m_data.powerAcPhase1 = m_data.powerAcTotal / 3;
    m_data.powerAcPhase2 = m_data.powerAcTotal / 3;
    m_data.powerAcPhase3 = m_data.powerAcTotal / 3;
    m_data.frequency = static_cast<float>(voltageAndPowerValues[6]) * pow(10, acFrequencyScale);
    m_data.powerFactor = static_cast<float>(voltageAndPowerValues[12]) * pow(10, powerFactorScale);
  }
  
  std::vector<uint16_t> dc1 = m_commPort->ReadHoldingRegister<uint16_t>(dcValues1Base,2,m_address);
  if(dc1.size() != 2)
  {
    //const Error error = {ErrorType::ERROR_MODBUS_READ, ErrorSeverityLevel::ERROR_WARNING};
    //m_et->PushBackError(error);
    return 1;
  }
  std::vector<uint16_t> dc2 = m_commPort->ReadHoldingRegister<uint16_t>(dcValues2Base,2,m_address);
  if(dc1.size() != 2)
  {
    //const Error error = {ErrorType::ERROR_MODBUS_READ, ErrorSeverityLevel::ERROR_WARNING};
    //m_et->PushBackError(error);
    return 1;
  }

  m_data.currentDc = static_cast<float>(dc1[0]) + static_cast<float>(dc2[0]);
  m_data.voltageDc = (static_cast<float>(dc1[1]) + static_cast<float>(dc2[1])) / 2;
  m_data.powerDc = static_cast<float>(dc1[0]) * static_cast<float>(dc1[1]) + static_cast<float>(dc2[0]) * static_cast<float>(dc2[1]);
  return 0;
}

void FroniusIgPlus::UpdatePower(float powerSetpoint)
{
  /* update trhottle -> enable control */
  if(powerSetpoint > maxContinuousPower)
  {
    //const Error error = {ErrorType::ERROR_CONTROL_WRONG_PWR_REQUEST, ErrorSeverityLevel::ERROR_WARNING};
    //m_et->PushBackError(error);
  }  
  else
  {
    uint16_t percentSetPoint = static_cast<uint16_t>(powerSetpoint / maxContinuousPower * 100);
    if(m_commPort->WriteSingleRegister(powerSetpointRegBase, percentSetPoint, m_address) == false)
    {
      //const Error error = {ErrorType::ERROR_MODBUS_WRITE, ErrorSeverityLevel::ERROR_WARNING};
      //m_et->PushBackError(error);
    }
    if(m_commPort->WriteSingleRegister(throttleEnableRegBase, 1, m_address) == false)
    {
      //const Error error = {ErrorType::ERROR_MODBUS_WRITE, ErrorSeverityLevel::ERROR_WARNING};
      //m_et->PushBackError(error);
    }
  }
  return ;
}

int Tesla::Initialize(const json& config)
{
  m_address = config.at("address").get<int>();
  return 0;
} 

int Tesla::ReadMeasurements()
{
  /* read current, voltages and power values */
  std::vector<uint16_t> statusMeasValues = m_commPort->ReadHoldingRegister<uint16_t>(statusMeasurementRegBase,12,m_address);
  if(statusMeasValues.size() != 12)
  {
    //const Error error = {ErrorType::ERROR_MODBUS_READ, ErrorSeverityLevel::ERROR_WARNING};
    //m_et->PushBackError(error);
    return 1;
  }
  else
  {
    m_data.voltageAcPhase1 = static_cast<float>(statusMeasValues[2]) * 0.01;
    m_data.voltageAcPhase2 = static_cast<float>(statusMeasValues[3]) * 0.01;
    m_data.voltageAcPhase3 = 0;
    m_data.currentAcPhase1 = static_cast<float>(statusMeasValues[4]) * 0.001;
    m_data.currentAcPhase2 = static_cast<float>(statusMeasValues[5]) * 0.001;
    m_data.currentAcPhase3 = 0;
    m_data.voltageDc = static_cast<float>((statusMeasValues[6] + statusMeasValues[7])/2) * 0.01;
    m_data.currentDc = static_cast<float>(statusMeasValues[8] + statusMeasValues[9]) * 0.001; 
    m_data.frequency = 50; /* TODO: later think about it*/
    m_data.inverterTemperature = (statusMeasValues[10] > statusMeasValues[11] ? statusMeasValues[10] : statusMeasValues[11]) - 50;
    m_data.powerAcPhase1 = m_data.voltageAcPhase1 * m_data.currentAcPhase1 * pow(3,0.5); 
    m_data.powerAcPhase2 = m_data.voltageAcPhase2 * m_data.currentAcPhase2 * pow(3,0.5); 
    m_data.powerAcPhase1 = m_data.voltageAcPhase1 * m_data.currentAcPhase1 * pow(3,0.5); 
    m_data.powerAcTotal = m_data.powerAcPhase1 + m_data.powerAcPhase2 + m_data.powerAcPhase3;
    m_data.powerFactor = m_data.powerAcTotal / (m_data.currentDc * m_data.voltageDc);
    m_data.inverterError = 0;
    m_data.inverterStatus = 0; /* TODO: revisit this status thing */
    return 0;
  }
} 

void Tesla::UpdatePower(float powerSetpoint)
{
  if(powerSetpoint > maxContinuousPower)
    ;
  else
  {
    uint16_t chargeCurrent = (powerSetpoint / ((m_data.voltageAcPhase1 + m_data.voltageAcPhase2) / 2)) / 2;
    if(m_commPort->WriteSingleRegister(chargeCurrentBase, chargeCurrent, m_address) == false)
    {
      //const Error error = {ErrorType::ERROR_MODBUS_WRITE, ErrorSeverityLevel::ERROR_WARNING};
      //m_et->PushBackError(error);
    }
  }
  return;
} 

bool DeviceContainer::ContainsPowerMeterDevice(const std::string& assetId)
{
  for(const auto device : m_powerMeterContainer)
  {
    if(device->GetAssetId() == assetId)
      return true;
  }

  return false;
}

bool DeviceContainer::ContainsInverterDevice(const std::string& assetId)
{
  for(const auto device : m_inverterContainer)
  {
    if(device->GetAssetId() == assetId)
      return true;
  }

  return false;
}

const PowerMeterData DeviceContainer::GetPowerMeterDeviceData(const std::string& assetId)
{
  for(const auto device : m_powerMeterContainer)
  {
    if(device->GetAssetId() == assetId)
      return device->GetPowerMeterData();
  }
  PowerMeterData data;
  return data;
}

const InverterData DeviceContainer::GetInverterDeviceData(const std::string& assetId)
{
  for(const auto device : m_inverterContainer)
  {
    if(device->GetAssetId() == assetId)
      return device->GetInverterData();
  }

  InverterData data;
  return data;
}
  
void DeviceConnectionChecker::OnDevicesAccess(bool eventType)
{
  switch(m_connection)
  {
    case DeviceConnection::DISCONNECTED:
    {
      if(eventType) /* if good acces comes in, start the good access timer, counter, save the start time and go to pending state */
      {
        m_goodAccesses = 1;
        m_goodAccessElapsedTime = std::chrono::duration<float>();
        m_goodAccessStartTime = std::chrono::steady_clock::now();
        m_connection = DeviceConnection::CONNECTION_PENDING;
      }
      else /* if bad access comes in then just update the bad access counter and timer */ 
      {
        m_badAccesses++; 
        m_badAccessElapsedTime = std::chrono::steady_clock::now() - m_badAccessStartTime;
      }
      break;
    }
    case DeviceConnection::CONNECTION_PENDING:
    {
      if(eventType) /* update the access counter and the timer, check if condition for connected is true */
      {
        m_goodAccesses++;
        m_goodAccessElapsedTime = std::chrono::steady_clock::now() - m_goodAccessStartTime;
        if(m_goodAccessElapsedTime.count() > m_config.minGoodSeconds && m_goodAccesses > m_config.minGoodAccessThreshold)
        {
          m_connection = DeviceConnection::CONNECTED;
        }
      } 
      else  /* if bad acces comes in, we should fall back to disconnected and start the bad access timer and counter*/ 
      {
        m_badAccesses = 1;
        m_badAccessStartTime = std::chrono::steady_clock::now();
        m_badAccessElapsedTime = std::chrono::duration<float>();
        m_connection = DeviceConnection::DISCONNECTED;
      }
    }
    case DeviceConnection::CONNECTED:
    {
      if(eventType) /* if good access comes in just update the counter and timer */
      {
        m_goodAccesses++;
        m_goodAccessElapsedTime = std::chrono::steady_clock::now() - m_goodAccessStartTime;
      }
      else /* if bad access comes in start the bad acces counter, timer, save the time and fall back to disconnect pending */
      {
        m_badAccesses = 1;
        m_badAccessStartTime = std::chrono::steady_clock::now();
        m_badAccessElapsedTime = std::chrono::duration<float>();
        m_connection = DeviceConnection::DISCONNECTION_PENDING;
      }
    }
    case DeviceConnection::DISCONNECTION_PENDING:
    {
      if(eventType) /* if good access comes in  just fall back to connected and update the counters*/
      {
        m_goodAccesses = 1;
        m_goodAccessStartTime = std::chrono::steady_clock::now();
        m_goodAccessElapsedTime = std::chrono::duration<float>();
        m_connection = DeviceConnection::CONNECTED;
      }
      else /* update the counter and timer and check if we can go to disconnected */
      {
        m_badAccesses++;
        m_badAccessElapsedTime = std::chrono::steady_clock::now() - m_badAccessStartTime;
        if(m_badAccessElapsedTime.count() > m_config.minBadSeconds && m_badAccesses > m_config.minBadAccessThreshold)
        {
          m_connection = DeviceConnection::DISCONNECTED;
        }
      }
    }
    default:
      break;
  }
}
