#include <math.h>
#include <limits>

#include "catchpenny.hpp"

void Catchpenny::AppendCharger(InverterDevice* charger)
{
  m_chargers.push_back(charger);
}

void Catchpenny::AppendDischarger(InverterDevice* discharger)
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

int Catchpenny::SetPowerSetpoint(float powerSetpoint)
{
  /* TODO: do some error handling */
  if(powerSetpoint < 0 && abs(powerSetpoint) <= m_config.maxDischargingPower)
    m_powerSetpoint = powerSetpoint;
  else if(powerSetpoint > 0 && abs(powerSetpoint) <= m_config.maxChargingPower)
    m_powerSetpoint = powerSetpoint;
  else
    return 1;
  
  return 0;
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

const BatteryPackMetaData Battery::GetBatterMetaData()
{
  BatteryPackMetaData data;
  m_data.dataMutex->lock_shared();
  data = m_data;
  m_data.dataMutex->unlock_shared();
  return data;
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
  float stateOfCharge = 0; // scaled to the total power between 0-1
  for(const auto& cell : m_cells)
    stateOfCharge += (m_batteryCapacity / m_cells.size()) * cell.capacity;
  
  m_data.stateOfCharge = stateOfCharge;
  m_data.dataMutex->lock();
  /* fill out meta data*/
  m_data.dataMutex->unlock();
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

void CatchpennyModbusTcpServer::Initialize(const std::string& ip, const std::string& port)
{
  /* setup the server and the buffered slave*/
  m_server.setBackend(Modbus::Tcp, ip, port);
  m_server.setDebug();
  m_server.setRecoveryLink();
  m_bufferSlave = &(m_server.addSlave(10));
  /* data registers */
  int ret = m_bufferSlave->setBlock(Modbus::InputRegister, 246, 1000);
  ret = m_bufferSlave->setBlock(Modbus::HoldingRegister, 3, 2000);

  /* write the constant registers */
  std::string systemSerialNumber("1001");
  std::string systemFirmwareVersion("1.0.0");
  std::string systemModel("CP1.0");
  std::string systemVersion("1.0.0");
  ret = m_bufferSlave->writeInputRegisters(1001, reinterpret_cast<const uint16_t*>(systemSerialNumber.data()), systemSerialNumber.size() / 2); 
  ret = m_bufferSlave->writeInputRegisters(1017, reinterpret_cast<uint16_t*>(systemFirmwareVersion.data()), systemFirmwareVersion.size() / 2); 
  ret = m_bufferSlave->writeInputRegisters(1033, reinterpret_cast<uint16_t*>(systemModel.data()), systemModel.size() / 2); 
  ret = m_bufferSlave->writeInputRegisters(1049, reinterpret_cast<uint16_t*>(systemVersion.data()), systemVersion.size() / 2); 
  ret = m_bufferSlave->writeInputRegister(1100, 3); // Inverter phase
  ret = m_bufferSlave->writeInputRegister(1101, 0); // Inverter type 
}

void CatchpennyModbusTcpServer::Process()
{
  if(m_server.open())
  {
    do {
      /* TODO: do the heartbeat logic here */
      Modbus::Data<float> powerSetPoint;
      m_bufferSlave->readRegister(AC_POWER_REQUEST, powerSetPoint);
      if(powerSetPoint.value() != powerSetPoint)
      {
        if(m_catchpenny->SetPowerSetpoint(powerSetPoint) == 0)
          m_catchpennyPowerSetpoint = powerSetPoint;
        else
          m_bufferSlave->writeRegister(AC_POWER_REQUEST, Modbus::Data<float>{m_catchpennyPowerSetpoint});
      }

      UpdateRegisters();
      m_server.poll(100);
    }
    while(m_server.isOpen());
  }
}

void CatchpennyModbusTcpServer::UpdateRegisters()
{
  /* extract all the data structures needed for the registers -> update the registers */
  CatchpennyConfig cpConfig = m_catchpenny->GetCpConfig();
  std::vector<InverterData> chargerData;
  std::vector<InverterData> dischargerData;
  for(int i = 0; i < m_catchpenny->GetNumberOfChargers(); i++)
  {
    chargerData.push_back(m_catchpenny->GetChargerData(i));
  }
  
  for(int i = 0; i < m_catchpenny->GetNumberOfDischargers(); i++)
  {
    dischargerData.push_back(m_catchpenny->GetDischargerData(i));
  }

  /* actually update the registers */
  if(chargerData.size() >= 2 && dischargerData.size() >= 2)
  {
    /* status registers update */
    /* TODO: revisit those status and error registers */
    Modbus::Data<uint32_t> inverterStatusData[4] = {dischargerData[0].inverterStatus,dischargerData[0].inverterError,chargerData[0].inverterStatus,chargerData[0].inverterError};
    m_bufferSlave->writeInputRegisters(DISCHARGING_INVERTER_STAT_REG, inverterStatusData, 4);
    
    /* update inverter data registers */
    /* TODO: check concurrency */
    float inverterPac = 0;
    float inverterP1 = 0;
    float inverterP2 = 0;
    float inverterP3 = 0;
    float voltageL1 = 0;
    float voltageL2 = 0;
    float voltageL3 = 0;
    float currentL1 = 0;
    float currentL2 = 0;
    float currentL3 = 0;
    float acFrequency = 0;
    float dischargingInvTemperature = 0;
    float chargingInvTemperature = 0;

    auto calulateRegisters = [&](const InverterData data){
      inverterPac += data.powerAcTotal;
      inverterP1 += data.powerAcPhase1;
      inverterP2 += data.powerAcPhase2;
      inverterP3 += data.powerAcPhase3;
      voltageL1 += data.voltageAcPhase1 / chargerData.size();
      voltageL2 += data.voltageAcPhase2 / chargerData.size();
      voltageL3 += data.voltageAcPhase3 / chargerData.size();
      currentL1 += data.currentAcPhase1;
      currentL2 += data.currentAcPhase2;
      currentL3 += data.currentAcPhase3;
      acFrequency += data.frequency / chargerData.size();
    };
    if(m_catchpenny->GetState() == CatchpennyState::CATCHPENNY_CHARGE)
    {
      /* count a mean value from the chargers */
      for(const auto& data : chargerData)
      {
        calulateRegisters(data);
        chargingInvTemperature += data.inverterTemperature / chargerData.size();
      }
    }
    else if(m_catchpenny->GetState() == CatchpennyState::CATCHPENNY_DISCHARGE)
    {
      for(const auto& data : dischargerData)
      {
        calulateRegisters(data);
        dischargingInvTemperature += data.inverterTemperature / chargerData.size();
      }
    }
    else  /* halted */
    {

    }

    Modbus::Data<float> inverterMeasurements[16] = {
      inverterPac, inverterP1, inverterP2, inverterP3,
      voltageL1, voltageL2, voltageL3,
      0.f,0.f,0.f,  /* line voltages are not measured */
      currentL1, currentL2, currentL3,
      acFrequency, dischargingInvTemperature, chargingInvTemperature 
    };
    m_bufferSlave->writeInputRegisters(INVERTER_PAC_REG, inverterMeasurements, 16);

    /* Update battery registers */
    std::vector<BatteryPackMetaData> batteryData;
    for(int i = 0; i < m_catchpenny->GetNumberOfBatteries(); i++)
      batteryData.push_back(m_catchpenny->GetBatteryData(i));
    
    if(batteryData.size() == 2)
    {
      Modbus::Data<uint32_t> batterStatus[2] = {batteryData[0].batteryStatus, batteryData[0].batteryError};
      m_bufferSlave->writeInputRegisters(BATTERY_STATUS_REG, batterStatus, 2);

      CatchpennyConfig config = m_catchpenny->GetCpConfig();
      Modbus::Data<float> cpConfigReg[4] = {config.maxChargeVoltage, config.minDischargeVoltage,
                                        config.maxChargeCurrent, config.maxDischargeCurrent};
      m_bufferSlave->writeInputRegisters(BATTERY_MAX_CHARGE_VOLTAGE_REG, cpConfigReg, 4);
      /* TODO: clarify battery modes */
      Modbus::Data<uint32_t> batteryMode[1] = {0};
      m_bufferSlave->writeInputRegisters(BATTERY_MODE, batteryMode, 1);

      float stateOfCharge = 0;
      float storedEnergy = 0;
      float stateOfHealth = 0;
      float maxCellTemperature = std::numeric_limits<float>::min();
      float minCellTemperature = std::numeric_limits<float>::max();
      float averageCellTemperature = 0;
      float stackTemperature = 0;
      float maxCellVoltage = std::numeric_limits<float>::min();
      float minCellVoltage = std::numeric_limits<float>::max();
      float averageCellVoltage = 0;
      float stackVoltage = 0;
      float stackCurrent = 0;
      for(const auto& battery : batteryData)
      {
        stateOfCharge += battery.stateOfCharge / batteryData.size();
        stateOfHealth += battery.stateOfHealth / batteryData.size(); /* TODO: revisit this one */
        storedEnergy += battery.storedEnergy;
        maxCellTemperature = battery.maxCellTemperature > maxCellTemperature ? battery.maxCellTemperature : maxCellTemperature;
        minCellTemperature = battery.minCellTemperature < minCellTemperature ? battery.minCellTemperature : minCellTemperature;
        averageCellTemperature += battery.averageCellTemperature / batteryData.size();
        stackTemperature = averageCellTemperature;
        maxCellVoltage = battery.maxCellVoltage > maxCellVoltage ? battery.maxCellVoltage : maxCellVoltage;
        minCellVoltage = battery.minCellVoltage < minCellVoltage ? battery.minCellVoltage : minCellVoltage;
        averageCellVoltage += battery.averageCellVoltage / batteryData.size();
        if(m_catchpenny->GetState() == CatchpennyState::CATCHPENNY_CHARGE)
        {
          for(const auto& charger : chargerData)
          {
            stackVoltage += charger.voltageDc / chargerData.size();
            stackCurrent += charger.currentDc / chargerData.size();
          }
        }
        else if(m_catchpenny->GetState() == CatchpennyState::CATCHPENNY_DISCHARGE)
        {
          for(const auto& discharger : dischargerData)
          {
            stackVoltage += discharger.voltageDc / chargerData.size();
            stackCurrent += discharger.currentDc / chargerData.size();
          }
        }
      }

      Modbus::Data<float> batteryMeasurements[12] = {
        stateOfCharge, storedEnergy, stateOfHealth,
        maxCellTemperature, minCellTemperature, averageCellTemperature, stackTemperature,
        maxCellVoltage, minCellVoltage, averageCellVoltage, stackVoltage, stackCurrent
      };
      m_bufferSlave->writeInputRegisters(BATTERY_STATE_OF_CHARGE, batteryMeasurements, 12);

      /* assume that all the batteries are configured the same */
      Modbus::Data<float> batteryConfigReg[4] = {
        cpConfig.maxChargeCurrent, cpConfig.maxDischargeCurrent, cpConfig.maxDischargingPower, cpConfig.maxDischargingPower};
      
      m_bufferSlave->writeInputRegisters(MAX_CHARGING_DC_CURRENT_REG, batteryConfigReg, 4);
    }
  } 
  else
    return;
}
