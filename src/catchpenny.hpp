#ifndef CATCHPENNY_HPP
#define CATCHPENNY_HPP

#include <memory>
#include <vector>
#include <mutex>
#include <shared_mutex>

#include "device.hpp"

enum class CatchpennyState{
  CATCHPENNY_CHARGE,
  CATCHPENNY_DISCHARGE,
  CATCHPENNY_IDLE,
  CATCHPENNY_HALTED_ON_ERROR
};

struct CatchpennyConfig{
  int   numberOfChargers;
  int   numberOfDischargers;
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

struct SystemInfo{
  const std::string serialNumber;
  const std::string firmwareVersion;
  const std::string systemModel;
  const std::string systemVersion;
};

struct Cell {
  float temperature, voltage, capacity;
};

/* TODO: use these error and status codes */
enum class BatteryStatusCode {
  CONNECTED_OK,
  CONNECTED_ERROR,
  DISCONNECTED
};

enum class BatteryErrorCode {
  NOERROR,
  OVERVOLTAGE_ERROR,
  UNDERVOLTAGE_ERROR,
  OVERTEMPERATURE_ERROR,
  UNDERTEMPERATURE_ERROR
};

struct BatteryPackMetaData {
  std::string deviceModel = "cona";
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
  float storedEnergy = 0;
  float stateOfCharge = 0;
  BatteryStatusCode batteryStatus = BatteryStatusCode::DISCONNECTED;
  BatteryErrorCode batteryError = BatteryErrorCode::NOERROR;
  std::shared_ptr<std::shared_mutex> dataMutex;
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
  DeviceConnectionChecker m_connChecker;
public:
  Battery(const float batteryCapacity, const int cellNumber, const CellConfig cellConfig, const int address, const DeviceConnectionChecker& checker = DeviceConnectionChecker()) 
          : m_cellConfig(cellConfig), m_batteryCapacity(batteryCapacity), m_address(address), m_connChecker(checker)
  {m_cells.resize(cellNumber);}

  bool CheckOverVoltage();
  bool CheckUnderVoltage();
  bool CheckOverTemperature();
  bool CheckUnderTemperature();
  int ReadMeasurements();
  static const std::string ParseStatusCode(const BatteryStatusCode& status);
  static const std::string ParseErrorCode(const BatteryErrorCode& error);
  /* Getters and setter */
  const Cell& GetCell(int index);
  float GetAvailableChargeStorage();    // in kWh
  float GetAvailableDischargeStorage(); // in kWh
  const BatteryPackMetaData GetBatterMetaData();
  CellConfig GetCellConfig() const {return m_cellConfig;}
  const DeviceConnection GetConnState() const {return m_connChecker.GetConnState();}

  void SetCommPort(std::shared_ptr<ModbusPort> commPort){m_commPort = commPort;};
  void SetDataMutex(std::shared_ptr<std::shared_mutex> mutex){m_data.dataMutex = mutex;}

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
  const SystemInfo m_systemInfo;
private:
  void DoCellProtectionLogic();
public:
  Catchpenny(){}
  Catchpenny(CatchpennyConfig config, const SystemInfo& info) : m_config(config), m_systemInfo(info){}
  void AppendCharger(InverterDevice* charger);
  void AppendDischarger(InverterDevice* discharger);
  void AppendBattery(Battery battery){m_battery.push_back(battery);}
  void ReadMeasurements();
  bool UpdateControl();
  int SetPowerSetpoint(const float powerSetpoint);
  void SafetyShutDown();

  /* Getter functions for the datastructures */
  const InverterData GetChargerData(int chargerNumber) {return m_chargers[chargerNumber]->GetInverterData();};
  const InverterData GetDischargerData(int dischargerNumber) {return m_dischargers[dischargerNumber]->GetInverterData();}
  const BatteryPackMetaData GetBatteryData(int batteryNumber) {return m_battery[batteryNumber].GetBatterMetaData();}

  int GetNumberOfChargers(){return m_chargers.size();}
  int GetNumberOfDischargers(){return m_dischargers.size();}
  int GetNumberOfBatteries(){return m_battery.size();}

  const DeviceConnection GetChargerConnState(int chargerNumber) const {return m_chargers[chargerNumber]->GetConnState();}
  const DeviceConnection GetDischargerConnState(int dischargerNumber) const {return m_dischargers[dischargerNumber]->GetConnState();}
  const DeviceConnection GetBatteryConnState(int batteryNumber) const {return m_battery[batteryNumber].GetConnState();}
  CellConfig GetBatteryCellConfig(int batteryNumber){return m_battery[batteryNumber].GetCellConfig();}
  const SystemInfo GetSystemInfo()const {return m_systemInfo;}
  const CatchpennyConfig& GetCpConfig(){return m_config;}
  const CatchpennyState GetState(){return m_state;}
};

/**
 * @brief class containing the register interface of the catchpenny system
 * 
 */
class CatchpennyModbusTcpServer {
public:
  CatchpennyModbusTcpServer(){}
  void UpdateRegisters();
  void Process();
  void Initialize(const std::string& ip, const std::string& port);
  void SetCatchpenny(std::shared_ptr<Catchpenny> catchpenny){m_catchpenny = catchpenny;}
private:
  std::shared_ptr<Catchpenny> m_catchpenny;
  Modbus::Server m_server;
  Modbus::BufferedSlave *m_bufferSlave;
  float m_catchpennyPowerSetpoint = 0;
private:
  static constexpr int INVERTER_PHASE_REG = 1100;
  static constexpr int DISCHARGING_INVERTER_STAT_REG = 1102;
  static constexpr int INVERTER_PAC_REG = 1110;
  static constexpr int BATTERY_STATUS_REG = 1200;
  static constexpr int BATTERY_MAX_CHARGE_VOLTAGE_REG = 1204;
  static constexpr int BATTERY_MODE = 1212;
  static constexpr int BATTERY_STATE_OF_CHARGE = 1214;
  static constexpr int MAX_CHARGING_DC_CURRENT_REG = 1238;
  static constexpr int AC_POWER_REQUEST = 2000;
};

#endif //CATCHPENNY_HPP