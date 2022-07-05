#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "mqtt_port.hpp"
#include "catchpenny.hpp"

namespace catchpenny_config{
  static const MqttConfig mqttConfig = {"127.0.0.1", "myClientId", "user", "passwd",std::vector<int>{0,1}};
  static const SystemInfo systemInfo = {"1001", "1.0.0", "CP1200", "1.0.0"};
  static const CellConfig cellCfg = {45,38,60,5};
  static const std::vector<std::string> subrscribeTopics = {
    "GetSystemInfo",
    "GetConfigInfo",
    "StatusUpdate",
    "GetChargerData",
    "GetDischargerData",
    "GetBatteryData",
    "GetConfigInfo",
    "PowerRequest",
    "GetPowerMeterData",
    "GetInverterData",
    "GetStatusUpdate"
  };
  
  static constexpr int circularBuffersize = 256;
};
#endif //CONFIG_HPP
