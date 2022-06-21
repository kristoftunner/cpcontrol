#include <math.h>

#include "catchpenny.hpp"

void Catchpenny::AppendCharger(std::shared_ptr<Inverter> charger)
{
  m_chargers.push_back(charger);
}

void Catchpenny::AppendDischarger(std::shared_ptr<Inverter> discharger)
{
  m_dischargers.push_back(discharger);
}

void Catchpenny::ReadMeasurements()
{
  /* TODO: do some error handling */
  for(auto charger : m_chargers)
  {
    charger->ReadMeasurements();
  }

  for(auto discharger : m_dischargers)
  {
    discharger->ReadMeasurements(); 
  }
  m_battery.ReadMeasurements();
}

void Catchpenny::SetPowerSetpoint(float powerSetpoint)
{
  /* TODO: do some error handling */
  if(powerSetPoint < 0 && abs(powerSetPoint) <= m_config.maxDischargingPower)
    m_powerSetpoint = powerSetpoint;
  else if(powerSetpoint > 0 && abs(powerSetpoint) <= m_config.maxChargingPower)
    m_powerSetpoint = powerSetpoint;
  else if()
  else
    ;
}

bool Catchpenny::UpdateControl()
{
  DoCellProtectionLogic();
  /* do the load balancing logic between the inverters and the battery*/
  switch(m_status)
  {
    case CATCHPENNY_HALTED_ON_ERROR:
      break;
    case CATCHPENNY_CHARGE:
      break;
    case CATCHPENNY_DISCHARGE:
      break;
  }
}

bool Battery::CheckOverVoltage()
{

}

bool Battery::CheckUnderVoltage()
{

}

bool Battery::CheckOvertTemperature()
{

}

bool Battery::CheckUnderTemperature()
{

}

const Cell& Battery::GetCell(int index)
{

}

const BatteryDataPack& Battery::GetBatteryDataPack()
{

}

void Battery::UpdateCell(int index, float temperature, float voltage, float capacity)
{

}

int Battery::ReadMeasurements()
{

}
