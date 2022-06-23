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

  std::vector<uint16_t> voltageAndPowerValues = m_commPort->ReadHoldingRegister<uint16_t>(acVoltagesBase,14,m_address);
  if(voltageAndPowerValues.size() != 14)
    return 1;
  else
  {
    /* scale values */
    float voltageScale = static_cast<float>(static_cast<int16_t>(voltaAndPowerValues[3]));
    float acPowerScale = static_cast<float>(static_cast<int16_t>(voltaAndPowerValues[5]));
    float acFrequencyScale = static_cast<float>(static_cast<int16_t>(voltaAndPowerValues[7]));
    float acApparentPowerScale = static_cast<float>(static_cast<int16_t>(voltaAndPowerValues[9]));
    float acReactivePowerScale = static_cast<float>(static_cast<int16_t>(voltaAndPowerValues[11]));
    float powerFactorScale = static_cast<float>(static_cast<int16_t>(voltaAndPowerValues[13]));
  
    m_data.voltageAcPhase1 = static_cast<float>(voltaAndPowerValues[0]) * pow(10, voltageScale);
    m_data.voltageAcPhase2 = static_cast<float>(voltaAndPowerValues[1]) * pow(10, voltageScale);
    m_data.voltageAcPhase3 = static_cast<float>(voltaAndPowerValues[2]) * pow(10, voltageScale);
    m_data.powerAcTotal = static_cast<float>(voltaAndPowerValues[4]) * pow(10, acPowerScale);
    m_data.powerAcPhase1 = m_data.powerAcTotal / 3;
    m_data.powerAcPhase2 = m_data.powerAcTotal / 3;
    m_data.powerAcPhase3 = m_data.powerAcTotal / 3;
    m_data.frequency = static_cast<float>(voltaAndPowerValues[6]) * pow(10, acFrequencyScale);
    m_data.powerFactor = static_cast<float>(voltaAndPowerValues[12]) * pow(10, powerFactorScale);
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

}
