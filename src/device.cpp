#include "device.hpp"

int SchneiderPM5110Meter::Initialize(json& config) 
{
  m_address = config.at("address").get<int>();
  return 0;
}

int SchneiderPM5110Meter::ReadMeasurements()
{
  std::vector<float> currentValues = m_commPort->ReadHoldingRegister<float>(currentPhase1Reg,3,m_address);
  if(currentValues.size() != 3)
    return 1;
  else
  {
    m_data.currentAcPhase1 = currentValues[0];
    m_data.currentAcPhase2 = currentValues[1];
    m_data.currentAcPhase3 = currentValues[2];
  }
  
  std::vector<float> voltageValues = m_commPort->ReadHoldingRegister<float>(voltagePhase1Reg,3,m_address);
  if(voltageValues.size() != 3)
    return 1;
  else
  {
    m_data.voltageAcPhase1 = voltageValues[0];
    m_data.voltageAcPhase2 = voltageValues[1];
    m_data.voltageAcPhase3 = voltageValues[2];
  }

  std::vector<float> powerValues = m_commPort->ReadHoldingRegister<float>(powerAcPhase1Reg,12,m_address);
  if(powerValues.size() != 12)
    return 1;
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
    return 1;
  else
  {
    m_data.frequency = frequency[0];
  }

  return 0;      
}

int FroniusIgPlus::Initialize(json& config)
{
  m_address = config.at("address").get<int>();
  return 0;
}

int FroniusIgPlus::ReadMeasurements() 
{
  std::vector<uint16_t> currentValues = m_commPort->ReadHoldingRegister<uint16_t>(acCurrentRegBase,5,m_address);
  if(currentValues.size() != 5)
    return 1;
  else
  {
    float scaleFactor = static_cast<float>(static_cast<int16_t>(currentValues[4]));
    m_data.currentAcPhase1 = static_cast<float>(currentValues[1]) * pow(10, scaleFactor);
    m_data.currentAcPhase2 = static_cast<float>(currentValues[2]) * pow(10, scaleFactor);
    m_data.currentAcPhase3 = static_cast<float>(currentValues[3]) * pow(10, scaleFactor);
  }

  std::vector<uint16_t> voltageAndPowerValues = m_commPort->ReadHoldingRegister<uint16_t>(acVoltageBase,14,m_address);
  if(voltageAndPowerValues.size() != 14)
    return 1;
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
    return 1;
  std::vector<uint16_t> dc2 = m_commPort->ReadHoldingRegister<uint16_t>(dcValues2Base,2,m_address);
  if(dc1.size() != 2)
    return 1;

  m_data.currentDc = static_cast<float>(dc1[0]) + static_cast<float>(dc2[0]);
  m_data.voltageDc = (static_cast<float>(dc1[1]) + static_cast<float>(dc2[1])) / 2;
  m_data.powerDc = static_cast<float>(dc1[0]) * static_cast<float>(dc1[1]) + static_cast<float>(dc2[0]) * static_cast<float>(dc2[1]);
  return 0;
}

void FroniusIgPlus::UpdatePower(float powerSetpoint)
{
  /* update trhottle -> enable control
    TODO: do some error handling here*/
  if(powerSetpoint > maxContinuousPower)
    ;
  else
  {
    uint16_t percentSetPoint = static_cast<uint16_t>(powerSetpoint / maxContinuousPower * 100);
    if(m_commPort->WriteSingleRegister(powerSetpointRegBase, percentSetPoint, m_address) == false)
    {
      ;
    }
    if(m_commPort->WriteSingleRegister(throttleEnableRegBase, 1, m_address) == false)
    {
      ;
    }
  }
  return ;
}

int Tesla::Initialize(json& config)
{
  m_address = config.at("address").get<int>();
  return 0;
} 

int Tesla::ReadMeasurements()
{
  /* read current, voltages and power values */
  std::vector<uint16_t> statusMeasValues = m_commPort->ReadHoldingRegister<uint16_t>(statusMeasurementRegBase,12,m_address);
  if(statusMeasValues.size() != 12)
    return 1;
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
      ; /* TODO: do the error handling */
    }
  }
  return;
} 

