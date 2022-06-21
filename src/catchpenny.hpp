#ifndef CATCHPENNY_HPP
#define CATCHPENNY_HPP

#include <memory>
#include <vector>

#include "device.hpp"

enum class CatchpennyState{
  CATCHPENNY_CHARGE,
  CATCHPENNY_DISCHARGE,
  CATCHPENNY_STOPPED,
  CATCHPENNY_HALTED_ON_ERROR
};

struct CatchpennyConfig{
  float maxChargeVoltage;
  float minChargeVoltage;
  float maxChargeCurrent;
  float minChargeCurrent;
  float maxDischargeVoltage;
  float minDischargeVoltage;
  float maxDischargeCurrent;
  float minDischargeCurrent;
  float maxChargingPower;
  float maxDischargingPower;
  float maxStoredEnergy;
};

struct Cell {
  float temperature, voltage, capacity;
};

struct BatteryPackMetaData {
  float minCellVoltage = 0;
  float maxCellVoltage = 0;
  float averageCellVoltage = 0;
  float minCellTemperature = 0;
  float maxCellTemperature = 0;
  float averageCellTemperature = 0;
  float minCellCapacity = 0;
  float maxCellCapacity = 0;
  float averageCellCapacity = 0;
  float stateOfHealth = 0;
  float stateOfCharge = 0;
  uint32_t batteryStatus = 0;
  uint32_t batteryError = 0;
};

template<int cellNumber>
class Battery {
private:
  std::vector<Cell> m_cells(cellNumber);
  BatteryPackMetaData m_data;
public:
  bool CheckOverVoltage();
  bool CheckUnderVoltage();
  bool CheckOverTemperature();
  bool CheckUnderTemperature();
  const Cell& GetCell(int index);
  const BatteryDataPack& GetBatteryDataPack();
  int ReadMeasurements();
};

/**
 * @brief class for controlling the catchpenny with the following functions:
 *  - UpdatePower()
 *  - ReadMeasurements()
 *  - GetConfig() 
 *  - Initialize()
 */
template<typename PARAMETERS>
class Catchpenny {
private:
  std::vector<std::shared_ptr<Inverter>> m_chargers;
  std::vector<std::shared_ptr<Inverter>> m_dischargers;
  Battery<80> m_battery;
  CatchpennyConfig m_config;
  float m_powerSetpoint;
  CatchpennyState m_state;
  void DoCellProtectionLogic();
public:
  Catchpenny(CatchpennyConfig config) : m_config(config){}
  void AppendCharger(std::shared_ptr<Inverter> charger);
  void AppendDischarger(std::shared_ptr<Inverter> discharger);
  void ReadMeasurements();
  bool UpdateControl();
  void SetPowerSetpoint(float powerSetpoint);

private:
  void DoCellProtectionLogic();
};
#endif //CATCHPENNY_HPP