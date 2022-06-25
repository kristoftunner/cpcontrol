#include <math.h>

#include "catchpenny.hpp"

void Catchpenny::AppendCharger(std::shared_ptr<InverterDevice> charger)
{
  m_chargers.push_back(charger);
}

void Catchpenny::AppendDischarger(std::shared_ptr<InverterDevice> discharger)
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
  for(auto battery : m_battery)
  {
    battery.ReadMeasurements();
  }
}

void Catchpenny::SetPowerSetpoint(float powerSetpoint)
{
  /* TODO: do some error handling */
  if(powerSetpoint < 0 && abs(powerSetpoint) <= m_config.maxDischargingPower)
    m_powerSetpoint = powerSetpoint;
  else if(powerSetpoint > 0 && abs(powerSetpoint) <= m_config.maxChargingPower)
    m_powerSetpoint = powerSetpoint;
  else
    ;
}

bool Catchpenny::UpdateControl()
{
  DoCellProtectionLogic();
  /* check if the catchpenny status is still valid:
  *   - if there was a power request -> attend to it
  *   - check if the battery is still healthy
  *  do the load balancing logic between the inverters and the battery
  */
  switch(m_state)
  {
    case CatchpennyState::CATCHPENNY_HALTED_ON_ERROR:
      /* update the state anyway */
      break;
    case CatchpennyState::CATCHPENNY_CHARGE:
    {
      if(m_powerSetpoint < 0)
      {
        m_state = CatchpennyState::CATCHPENNY_DISCHARGE;
        break;
      }
      else if(m_powerSetpoint > 0)
      {
       /* check if there is enough charge power -> load balance the requested power */
        std::vector<float> availableChargeStorage(m_battery.size());
        std::vector<float> chargePowers(m_chargers.size());
        for(auto battery : m_battery)
        {
          availableChargeStorage.push_back(battery.GetAvailableChargeStorage());
        }
        float sumStorage = 0;
        for(auto storage : availableChargeStorage)
          sumStorage += storage;
        
        if(sumStorage <= std::abs(m_powerSetpoint))
        {
          /* charge power for the one inverter: proportionately divided between the inverters,
            at least 0.5 hour of charge should be possible with this setting*/
          if(sumStorage < std::abs(m_powerSetpoint) / 2)
          {
            /* do some state handling here */
            m_state = CatchpennyState::CATCHPENNY_IDLE;
            break;
          }
          for(auto storage : availableChargeStorage)
          {
            chargePowers.push_back(storage / sumStorage / 2 * std::abs(m_powerSetpoint));
          }
        }
        else
        {
          for(auto storage : availableChargeStorage)
          {
            /* charge power for the one inverter: proportionately divided between the inverters,
              at least 1 hour of charge should be possible with this setting*/
            chargePowers.push_back(storage / sumStorage * std::abs(m_powerSetpoint));
          }
        }
      }
      else if(m_powerSetpoint == 0)
      {
        m_state = CatchpennyState::CATCHPENNY_IDLE;
        for(auto charger : m_chargers)
        {
          charger->UpdatePower(0);
        }
        for(auto discharger : m_dischargers)
        {
          discharger->UpdatePower(0);
        }
      }
      break;
    }
    case CatchpennyState::CATCHPENNY_DISCHARGE:
    {
      if(m_powerSetpoint < 0)
      {
       /* check if there is enough charge power -> load balance the requested power */
        std::vector<float> availableDischargeStorage(m_battery.size());
        std::vector<float> dischargePowers(m_dischargers.size());
        for(auto battery : m_battery)
        {
          availableDischargeStorage.push_back(battery.GetAvailableDischargeStorage());
        }
        float sumStorage = 0;
        for(auto storage : availableDischargeStorage)
          sumStorage += storage;
        
        if(sumStorage >= std::abs(m_powerSetpoint))
        {
          /* discharge power for the one inverter: proportionately divided between the inverters,
            at least 0.5 hour of discharge should be possible with this setting*/
          if(sumStorage < std::abs(m_powerSetpoint) / 2)
          {
            /* do some state handling here */
            m_state = CatchpennyState::CATCHPENNY_IDLE;
            break;
          }
          else
          {
            for(auto storage : availableDischargeStorage)
            {
              dischargePowers.push_back(storage / sumStorage * std::abs(m_powerSetpoint));
            }
          }
        }
        else
        {
          for(auto storage : availableDischargeStorage)
          {
            /* charge power for the one inverter: proportionately divided between the inverters,
              at least 1 hour of charge should be possible with this setting*/
            dischargePowers.push_back(storage / sumStorage * std::abs(m_powerSetpoint));
          }
        }
      }
      else if(m_powerSetpoint > 0)
      {
        m_state = CatchpennyState::CATCHPENNY_CHARGE;
        break;
      }
      else if(m_powerSetpoint == 0)
      {
        m_state = CatchpennyState::CATCHPENNY_IDLE;
        for(auto charger : m_chargers)
        {
          charger->UpdatePower(0);
        }
        for(auto discharger : m_dischargers)
        {
          discharger->UpdatePower(0);
        }
      }
      break;
    }
    case CatchpennyState::CATCHPENNY_IDLE:
    {
      if(m_powerSetpoint > 0)
      {
        m_state = CatchpennyState::CATCHPENNY_CHARGE;
      }
      else if(m_powerSetpoint < 0)
      {
        m_state = CatchpennyState::CATCHPENNY_DISCHARGE;
      }
      else if(m_powerSetpoint = 0)
      {
        m_state = CatchpennyState::CATCHPENNY_IDLE;
      }
      break;
    }
    default:
      break;
  }
}

void Catchpenny::DoCellProtectionLogic()
{
  /* check if the battery is OK, if not -> set state HALTED_ON_ERROR
    TODO: do some error handling here */
  for(auto battery : m_battery)
  {
    if((battery.CheckOverTemperature() && battery.CheckOverVoltage() && battery.CheckUnderTemperature()
      && battery.CheckUnderVoltage()) == false)
    {
      SafetyShutDown();
    }
  }
}

void Catchpenny::SafetyShutDown()
{
  m_state = CatchpennyState::CATCHPENNY_HALTED_ON_ERROR;
  for(auto charger : m_chargers)
  {
    charger->UpdatePower(0);
  }
  for(auto discharger : m_dischargers)
  {
    discharger->UpdatePower(0);
  }
}

bool Battery::CheckOverVoltage()
{
  for(Cell& cell : m_cells)
  {
    if(cell.voltage > m_cellConfig.maxCellVoltage)
      return false;
  }
  return true;
}

bool Battery::CheckUnderVoltage()
{
  for(Cell& cell : m_cells)
  {
    if(cell.voltage < m_cellConfig.minCellVoltage)
      return false;
  }
  return true;
}

bool Battery::CheckOverTemperature()
{
  for(Cell& cell : m_cells)
  {
    if(cell.temperature > m_cellConfig.maxCellTemperature)
      return false;
  }
  return true;
}

bool Battery::CheckUnderTemperature()
{
  for(Cell& cell : m_cells)
  {
    if(cell.temperature < m_cellConfig.minCellTemperature)
      return false;
  }
  return true;
}

const Cell& Battery::GetCell(int index)
{
  return m_cells[index];
}

const BatteryPackMetaData& Battery::GetBatterMetaData()
{
  return m_data;
}

int Battery::ReadMeasurements()
{
  /* read the measurements, update cell data and BatteryMetaData */
  std::vector<uint16_t> cellVoltageValues = m_commPort->ReadHoldingRegister<uint16_t>(cellVoltagesBaseReg,m_cells.size(),m_address);
  std::vector<uint16_t> cellTemperatureValues = m_commPort->ReadHoldingRegister<uint16_t>(cellTemperaturesBaseReg,30,m_address);
  std::vector<uint16_t> cellCapacityValues = m_commPort->ReadHoldingRegister<uint16_t>(cellCapacityBaseReg,m_cells.size(),m_address);
  if(cellVoltageValues.size() != m_cells.size() && cellCapacityValues.size() != m_cells.size())  
  { 
    return 1; /*TODO: some error handling*/
  }
  else
  {
    for(size_t index; auto& voltage : cellVoltageValues)
      m_cells[index].voltage = static_cast<float>(voltage) * 0.001;
    for(size_t index; auto& capacity : cellCapacityValues)
      m_cells[index].capacity = static_cast<float>(capacity) * 0.001;
  }
  if(cellTemperatureValues.size() != 30)
  {
    return 1; /* TODO: do the error handling here */
  }
  else 
  {
    for(int i = 0; i < cellTemperatureValues.size()/3; i++)
    {
      m_cells[i*5].temperature   =  static_cast<float>(cellTemperatureValues[i] >> 8) - 50;
      m_cells[i*5+1].temperature =  static_cast<float>(cellTemperatureValues[i] & 0xff) - 50; 
      m_cells[i*5+2].temperature =  static_cast<float>(cellTemperatureValues[i+1] >> 8) - 50; 
      m_cells[i*5+3].temperature =  static_cast<float>(cellTemperatureValues[i+1] & 0xff) - 50; 
      m_cells[i*5+4].temperature =  static_cast<float>(cellTemperatureValues[i+2] >> 8) - 50; 
    }
  }
  
  return 0;
}

float Battery::GetAvailableChargeStorage()
{
  return (100 - m_data.stateOfCharge) * m_batteryCapacity * 0.01;
}    
float Battery::GetAvailableDischargeStorage()
{
  return m_data.stateOfCharge * m_batteryCapacity * 0.01;
}
