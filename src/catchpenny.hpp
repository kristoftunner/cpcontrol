#ifndef CATCHPENNY_HPP
#define CATCHPENNY_HPP

#include <memory>
#include <vector>

#include "device.hpp"

enum class CatchpennyState{
  CATCHPENNY_CHARGE,
  CATCHPENNY_DISCHARGE,
  CATCHPENNY_IDLE,
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

struct CellConfig {
  float maxCellVoltage, minCellVoltage, maxCellTemperature, minCellTemperature;
};

class Battery {
private:
  std::shared_ptr<ModbusPort> m_commPort;
  std::vector<Cell> m_cells;
  BatteryPackMetaData m_data;
  int m_address;
  float m_batteryCapacity;
  CellConfig m_cellConfig;
public:
  Battery(float batteryCapacity, int cellNumber, CellConfig cellConfig, int address) : m_cellConfig(cellConfig), m_batteryCapacity(batteryCapacity), m_address(address)
  {m_cells.resize(cellNumber);}
  bool CheckOverVoltage();
  bool CheckUnderVoltage();
  bool CheckOverTemperature();
  bool CheckUnderTemperature();
  const Cell& GetCell(int index);
  const BatteryPackMetaData& GetBatterMetaData();
  int ReadMeasurements();
  float GetAvailableChargeStorage();    // in kWh
  float GetAvailableDischargeStorage(); // in kWh
  void SetCommPort(std::shared_ptr<ModbusPort> commPort){m_commPort = commPort;};

private:
  static constexpr int cellVoltagesBaseReg = 0;
  static constexpr int cellTemperaturesBaseReg = 80;
  static constexpr int cellCapacityBaseReg = 110;
};

/**
 * @brief class for controlling the catchpenny with the following functions:
 *  - UpdatePower()
 *  - ReadMeasurements()
 *  - GetConfig() 
 *  - Initialize()
 */
class Catchpenny {
private:
  std::vector<InverterDevice*> m_chargers;
  std::vector<InverterDevice*> m_dischargers;
  std::vector<Battery> m_battery;
  CatchpennyConfig m_config;
  float m_powerSetpoint;
  float m_actualPower;
  CatchpennyState m_state;
  void DoCellProtectionLogic();
public:
  Catchpenny(){}
  Catchpenny(CatchpennyConfig config) : m_config(config){}
  void AppendCharger(InverterDevice* charger);
  void AppendDischarger(InverterDevice* discharger);
  void AppendBattery(Battery battery){m_battery.push_back(battery);}
  void ReadMeasurements();
  bool UpdateControl();
  void SetPowerSetpoint(float powerSetpoint);
  void SafetyShutDown();
};

#endif //CATCHPENNY_HPP