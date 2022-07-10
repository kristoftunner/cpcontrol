#ifndef DEVICE_HPP
#define DEVICE_HPP

#include <iostream>
#include <vector>
#include <map>
#include <shared_mutex>
#include <chrono>

#include "device_comm.hpp"
//#include "error_tracker.hpp"

enum class Devicetype {
  powerMeter,
  inverter,
  heatpump
};

struct PowerMeterData {
public:
  PowerMeterData(){} 

  std::shared_ptr<std::shared_mutex> dataMutex;
  std::string deviceType;
  float powerAcTotal = 0;
  float powerAcPhase1 = 0;
  float powerAcPhase2 = 0;
  float powerAcPhase3 = 0;
  
  float apparentPowerTotal = 0;
  float apparentPowerPhase1 = 0;
  float apparentPowerPhase2 = 0;
  float apparentPowerPhase3 = 0;
  
  float reactivePowerTotal = 0;
  float reactivePowerPhase1 = 0;
  float reactivePowerPhase2 = 0;
  float reactivePowerPhase3 = 0;
  
  float currentAcPhase1 = 0;
  float currentAcPhase2 = 0;
  float currentAcPhase3 = 0;
  float voltageAcPhase1 = 0; 
  float voltageAcPhase2 = 0; 
  float voltageAcPhase3 = 0; 

  float frequency = 0;
};

enum class InverterStatusCode {
  CONNECTED_IDLE,
  CONNECTED_THROTTLED,
  DISCONNECTED
};

enum class InverterErrorCode {
  NOERROR,
  DISCONNECTED_BUS_ERROR,
  OVERHEATED
};
struct InverterData {
public:
  std::string deviceType;
  float powerDc = 0;
  float currentDc = 0;
  float voltageDc = 0;

  float powerAcTotal = 0;
  float powerAcPhase1 = 0;
  float powerAcPhase2 = 0;
  float powerAcPhase3 = 0;
  float powerFactor = 0;

  float currentAcPhase1 = 0;
  float currentAcPhase2 = 0;
  float currentAcPhase3 = 0;
  float voltageAcPhase1 = 0; 
  float voltageAcPhase2 = 0; 
  float voltageAcPhase3 = 0; 

  float frequency;

  float inverterTemperature;
  InverterStatusCode inverterStatus = InverterStatusCode::DISCONNECTED;
  InverterErrorCode inverterError = InverterErrorCode::NOERROR;
  std::shared_ptr<std::shared_mutex> dataMutex;
};

enum class DeviceConnection {
  CONNECTION_PENDING,
  DISCONNECTION_PENDING,
  CONNECTED,
  DISCONNECTED
};

struct ConnectionCheckerConfig {
  /* for checking disconnection */
  int minBadAccessThreshold;
  float minBadSeconds;
  /* for checking connection */
  int minGoodAccessThreshold;
  float minGoodSeconds;

  ConnectionCheckerConfig(int minBadAccess = 4, float minBadSeconds = 2.0, int minGoodAccess = 4, float minGoodSeconds = 2.0)
                          :minBadAccessThreshold(minBadAccess), minBadSeconds(minBadSeconds), minGoodAccessThreshold(minGoodAccess), minGoodSeconds(minGoodSeconds)
                          {}
};
/**
 * @brief checks if the associated device is connected, for example:
 *        - if a modbus access was wrong, register the wrong read, measure the elapsed time between the first bad access and
 *        the occurences of bad accesses, devices are disconnected according to this:
 *          - if both min thresholds are reached
 *        - devices are counted as connected if:
 *          - there was a minium amount of good accesses and the the time from the first good access above the minimum good access threshold
 * 
 */
class DeviceConnectionChecker
{
public:
  DeviceConnectionChecker(const ConnectionCheckerConfig& config = ConnectionCheckerConfig()) : m_config(config){}
  const DeviceConnection GetConnState() const {return m_connection;}
  /**
   * @brief updates the elapsed time
   * 
   */
  void OnDevicesAccess(bool eventType);
private:
  DeviceConnection m_connection = DeviceConnection::DISCONNECTED;
  ConnectionCheckerConfig m_config;
  std::chrono::duration<float> m_goodAccessElapsedTime;
  std::chrono::duration<float> m_badAccessElapsedTime;
  std::chrono::time_point<std::chrono::steady_clock> m_goodAccessStartTime;
  std::chrono::time_point<std::chrono::steady_clock> m_badAccessStartTime;
  int m_goodAccesses = 0;
  int m_badAccesses = 0;
};

class BaseDevice
{
protected:
  std::string m_assetId;
  Devicetype m_type;
  DeviceConnectionChecker m_connChecker;
public:
  BaseDevice(const DeviceConnectionChecker& checker) : m_assetId(""), m_connChecker(checker) {}
  virtual int ReadMeasurements() = 0;
  virtual void SetAssetId(std::string assetId){m_assetId = assetId;}
  const std::string GetAssetId() const {return m_assetId;}
  const DeviceConnection GetConnState() const {return m_connChecker.GetConnState();}
};

class PowerMeterDevice : public BaseDevice {
protected:
  PowerMeterData m_data;
public:
  PowerMeterDevice(const DeviceConnectionChecker& checker) :BaseDevice(checker) {m_type = Devicetype::powerMeter;}
  virtual int Initialize(const json& config) = 0;
  virtual int ReadMeasurements() override {};
  virtual PowerMeterData GetPowerMeterData() const {return m_data;};
  virtual void SetDataMutex(std::shared_ptr<std::shared_mutex> mutex){m_data.dataMutex = mutex;}
};

class SchneiderPM5110Meter : public PowerMeterDevice 
{
private:
/* define the register addresses here: */
  static constexpr int currentPhase1Reg = 3000;
  static constexpr int currentPhase2Reg = 3002;
  static constexpr int currentPhase3Reg = 3004;
  static constexpr int voltagePhase1Reg = 3028;
  static constexpr int voltagePhase2Reg = 3030;
  static constexpr int voltagePhase3Reg = 3032;
  static constexpr int powerAcPhase1Reg = 3054;
  static constexpr int powerAcPhase2Reg = 3056;
  static constexpr int powerAcPhase3Reg = 3058;
  static constexpr int powerAcTotalReg  = 3060;
  static constexpr int powerReactivePhase1Reg = 3062;
  static constexpr int powerReactivePhase2Reg = 3064;
  static constexpr int powerReactivePhase3Reg = 3066;
  static constexpr int powerReactiveTotalReg = 3068;
  static constexpr int powerApparentPhase1Reg = 3070;
  static constexpr int powerApparentPhase2Reg = 3072;
  static constexpr int powerApparentPhase3Reg = 3074;
  static constexpr int powerApparentTotalReg = 3076;
  static constexpr int frequencyReg = 3110;

  std::shared_ptr<ModbusPort> m_commPort;
  int m_address;
public:
  SchneiderPM5110Meter(const DeviceConnectionChecker& checker = DeviceConnectionChecker()) : PowerMeterDevice(checker){m_type = Devicetype::powerMeter;}
  virtual int Initialize(const json& config) override;
  virtual int ReadMeasurements() override;
  virtual PowerMeterData GetPowerMeterData() {return m_data;}

  void SetCommPort(std::shared_ptr<ModbusPort> commPort) {m_commPort = commPort;}
};

class InverterDevice : public BaseDevice {
protected:
  InverterData m_data;
public:
  InverterDevice(const DeviceConnectionChecker& checker) : BaseDevice(checker){m_type = Devicetype::inverter;}

  virtual int Initialize(const json& config) = 0;
  virtual int ReadMeasurements() override {return 0;};
  const InverterData GetInverterData()
  {
    InverterData data;
    m_data.dataMutex->lock_shared();
    data = m_data;
    m_data.dataMutex->unlock_shared();
    return data;
  }
  static const std::string ParseErrorCode(const InverterErrorCode& errorCode);
  static const std::string ParseStatusCode(const InverterStatusCode& statusCode);
  virtual void UpdatePower(float powerSetpoint) = 0;
  virtual void SetDataMutex(std::shared_ptr<std::shared_mutex> mutex){m_data.dataMutex = mutex;}
};

class FroniusIgPlus : public InverterDevice {
private:
  std::shared_ptr<ModbusPort> m_commPort;
  int m_address;
  static constexpr float maxContinuousPower = 5000; // in W
  /* register addresses */
  static constexpr int acCurrentRegBase = 40071;
  static constexpr int acVoltageBase = 40079;
  static constexpr int acPowerBase = 40083;
  static constexpr int powerSetpointRegBase = 40232;
  static constexpr int throttleEnableRegBase = 40236;
  static constexpr int dcValues1Base = 40272;
  static constexpr int dcValues2Base = 40292;
public:
  FroniusIgPlus(const DeviceConnectionChecker& checker = DeviceConnectionChecker()) : InverterDevice(checker){}
  virtual int Initialize(const json& config) override;
  virtual int ReadMeasurements() override;
  virtual void UpdatePower(float powerSetpoint) override;

  void SetCommPort(std::shared_ptr<ModbusPort> commPort) {m_commPort = commPort;}
};

class Tesla : public InverterDevice {
private:
  std::shared_ptr<ModbusPort> m_commPort;
  int m_address;
  static constexpr float maxContinuousPower = 5000; /*TODO specify this in the future*/
  /* register addresses */
  static constexpr int cellVoltagesBase = 0;
  static constexpr int cellTemperaturesBase = 80;
  static constexpr int cellCapacitiesBase = 110;
  static constexpr int chargeCurrentBase = 190;
  static constexpr int statusMeasurementRegBase = 191;
  static constexpr int waterTemperatureBase = 206;
public:
  Tesla(const DeviceConnectionChecker& checker = DeviceConnectionChecker()) : InverterDevice(checker){}
  virtual int Initialize(const json& config) override;
  virtual int ReadMeasurements() override;
  virtual void UpdatePower(float powerSetpoint) override;

  void SetCommPort(std::shared_ptr<ModbusPort> commPort) {m_commPort = commPort;}
};

class DeviceContainer {
private:
  std::vector<PowerMeterDevice*> m_powerMeterContainer;
  std::vector<InverterDevice*> m_inverterContainer;
  
  template<int (PowerMeterDevice::*functor)()>
  int ForEachDevice();
  
  template<int (InverterDevice::*functor)()>
  int ForEachDevice();

public:
  DeviceContainer(){}
  void AppendDevice(PowerMeterDevice *device) {
    m_powerMeterContainer.push_back(device);
  }
  
  void AppendDevice(InverterDevice *device) {
    m_inverterContainer.push_back(device);
  }

  template<class T>
  int ReadMeasurements(){ return ForEachDevice<&T::ReadMeasurements>();}

  bool ContainsPowerMeterDevice(const std::string& assetId);
  bool ContainsInverterDevice(const std::string& assetId);
  const PowerMeterData GetPowerMeterDeviceData(const std::string& assetId) const;
  const InverterData GetInverterDeviceData(const std::string& assetId) const ;
};

template<int (PowerMeterDevice::*functor)()>
int DeviceContainer::ForEachDevice()
{
  int containerSize = m_powerMeterContainer.size();
  int error = 0;
  for(int i = 0; i < containerSize; i++)
  {
    PowerMeterDevice *device = m_powerMeterContainer[i];
    error |= (device->*functor)();
  }

  return error;
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
  
  return 0;
}

#endif //DEVICE_HPP