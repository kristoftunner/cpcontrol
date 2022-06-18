#include "device.hpp"

template<int (PowerMeterDevice::*functor)()>
int DeviceContainer::ForEachDevice()
{
  int containerSize = m_powerMeterContainer.size();
  for(int i = 0; i < containerSize; i++)
  {
    PowerMeterDevice *device = m_powerMeterContainer[i];
    (device->*functor)();
  }
}

template<int (InverterDevice::*functor)()>
int DeviceContainer::ForEachDevice()
{
  int containerSize = m_inverterContainer.size();
  for(int i = 0; i < containerSize; i++)
  {
    InverterDevice *device = m_inverterContainer[i];
    (device->*functor)();
  }
}


int SchneiderPM5110Meter::Initialize(json& config) 
{
  m_address = config.at("address").get<int>();
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

