#include "mqtt_port.hpp"
#include "json.hpp"

using namespace std::chrono;
using json = nlohmann::json;

void PahoMqttPort::Subscribe(const std::vector<std::string>& topicsToSubscribe)
{
  for(const auto& topic : topicsToSubscribe)
  {
    /* TODO: do some error handling here */
    m_client->subscribe(topic, 1);
    m_subscribedTopics.push_back(topic);
  }
}

void PahoMqttPort::Publish(const MqttMessage& message)
{
  std::string data = message.message.dump();
  m_client->publish(mqtt::message(message.topic, message.message.dump(), 1, false));
}

void PahoMqttPort::Listen() 
{
  while(true)
  {
    auto msg = m_client->consume_message();

    if(msg)
    {
      /* forward the message to the controller(?)consumer(?) */
      std::string myString = msg->to_string();
      json parsedMessage = json::parse(msg->to_string(), nullptr, false, false);
      if(!parsedMessage.is_discarded())
      {
        MqttMessage recvMsg = {msg->get_topic(), parsedMessage};
        std::vector<MqttMessage> pubMessages = m_messageResponder.ActOnMessage(recvMsg);
        if(pubMessages.size() > 0)
        {
          for(const auto& message : pubMessages)
          {
            Publish(message);
          }
        }
      }
    }
    if(!IsConnected())
    {
      /* TODO: do here a reconnect */
      break;
    }
  }
}

bool PahoMqttPort::IsConnected()
{
  /* TODO: implement the error handling here */
  int counter = 0;
	if (!m_client->is_connected()) {
		std::cout << "not connected" << std::endl;
		while (!m_client->is_connected()) {
      counter++;
      if(++counter >= 10)
      {
        std::cout << "lost connection" << std::endl;
        return false;
      }
			std::this_thread::sleep_for(milliseconds(250));
		}
		std::cout << "Re-established connection" << std::endl;
	}
  std::cout << "connected to the mqtt server" << std::endl;
  return true;
}

bool PahoMqttPort::Initailize(const MqttConfig& config)
{
  std::shared_ptr<mqtt::client> cli = std::make_shared<mqtt::client>(config.serverAddres, config.clientId);
  auto connOptions = mqtt::connect_options_builder()
		.user_name(config.userName)
		.password(config.password)
		.keep_alive_interval(seconds(30))
		.automatic_reconnect(seconds(2), seconds(30))
		.clean_session(false)
		.finalize();

  m_qosCollection = config.qos;
  m_client = cli;
  std::cout << "connecting to thes erver...\n";
  m_client->connect(connOptions);
  if(IsConnected())
    return true;
  else
    return false;
}

std::vector<MqttMessage> MqttMessageResponder::ActOnMessage(const MqttMessage& message)
{
  /* here we should:
  *   -parse the topic 
  *   -act on the message
  */
  std::vector<MqttMessage> ret;
  /* first lets do the catchpenny battery related messages */
  if(message.topic == "GetSystemInfo")
  {
    const SystemInfo info = m_catchpenny->GetSystemInfo();
    json responseJson = {
      {"id", 0},
      {"firmwareVersion", info.firmwareVersion},
      {"systemModel", info.systemModel},
      {"serialNumber", info.serialNumber},
      {"systemVersion", info.systemVersion}
    };
    MqttMessage response = {message.topic, responseJson};
    ret.push_back({std::string("SystemInfo"), responseJson});
  }
  else if(message.topic == "GetConfigInfo")
  {
    const CatchpennyConfig configInfo = m_catchpenny->GetCpConfig();
    json responseMessage = {
      {"id", 0},
      {"numberOfChargers", configInfo.numberOfChargers},
      {"numberOfDischargers", configInfo.numberOfDischargers},
      {"maxChargeVoltage", configInfo.maxChargeVoltage},
      {"minChargeVoltage", configInfo.minChargeVoltage},
      {"maxChargeCurrent", configInfo.maxChargeCurrent},
      {"minChargeCurrent", configInfo.minChargeCurrent},
      {"maxDischargeVoltage", configInfo.maxDischargeVoltage},
      {"minDischargeVoltage", configInfo.minDischargeVoltage},
      {"maxChargingPower", configInfo.maxChargingPower},
      {"maxDischargingPower", configInfo.maxDischargingPower},
      {"maxStoredEnergy", configInfo.maxStoredEnergy}
    };
    ret.push_back({std::string("ConfigInfo"), responseMessage});
  }
  else if(message.topic == "GetStatusUpdate")
  {
    /* TODO: implement this message with the error tracker */
  }
  else if(message.topic == "GetCatchpennyDevices")
  {
    /* get the data, parse the error and status code into the json message */
    const int numberOfChargers = m_catchpenny->GetNumberOfChargers();
    const int numberOfDischargers = m_catchpenny->GetNumberOfDischargers();
    const int numberOfBatteries = m_catchpenny->GetNumberOfBatteries();
    json responseMessage = {{"id",0}};
    json chargerArray = json::array();
    json dischargerArray = json::array();
    json batteryArray = json::array();
    for(int i = 0; i < numberOfChargers; i++)
    {
      const InverterData data = m_catchpenny->GetChargerData(i);
      const std::string status = InverterDevice::ParseStatusCode(data.inverterStatus);
      json singleCharger = {{"id", i}, {"model", data.deviceModel}, {"status", status}};
      chargerArray.emplace_back(singleCharger);
    }
    for(int i = 0; i < numberOfDischargers; i++)
    {
      const InverterData data = m_catchpenny->GetDischargerData(i);
      const std::string status = InverterDevice::ParseStatusCode(data.inverterStatus);
      json singleDischarger = {{"id", i}, {"model", data.deviceModel}, {"status", status}};
      dischargerArray.emplace_back(singleDischarger);
    }
    for(int i = 0; i < numberOfBatteries; i++)
    {
      const BatteryPackMetaData data = m_catchpenny->GetBatteryData(i);
      const std::string status = Battery::ParseStatusCode(data.batteryStatus);
      json singleBattery = {{"id", i}, {"model", data.deviceModel}, {"status", "CONNECTED"}};
      batteryArray.emplace_back(singleBattery);
    }
    
    responseMessage["chargers"] = chargerArray;
    responseMessage["dischargers"] = dischargerArray;
    responseMessage["batteries"] = batteryArray;
    ret.push_back({std::string("CatchpennyDevices"), responseMessage});
  }
  else if(message.topic == "PowerRequest")
  {
    const float power = message.message.at("power").get<float>();
    m_catchpenny->SetPowerSetpoint(power);
  }
  else if(message.topic == "GetChargerData")
  {
    int chargerNumber = message.message.at("chargerId").get<int>();
    if(chargerNumber < m_catchpenny->GetNumberOfChargers() && chargerNumber >= 0)
    {
      InverterData invData = m_catchpenny->GetChargerData(chargerNumber);
      json responseMessage = {
        {"id",0},
        {"chargerId", chargerNumber},
        {"powerAcL1", invData.powerAcPhase1},
        {"powerAcL2", invData.powerAcPhase2},
        {"powerAcL3", invData.powerAcPhase3},
        {"voltageAcL1", invData.voltageAcPhase1},
        {"voltageAcL2", invData.voltageAcPhase2},
        {"voltageAcL3", invData.voltageAcPhase3},
        {"currentAcL1", invData.currentAcPhase1},
        {"currentAcL2", invData.currentAcPhase2},
        {"currentAcL3", invData.currentAcPhase3},
        {"temperature", invData.inverterTemperature},
        {"acFrequency", invData.frequency},
        {"dcVoltage",   invData.voltageDc},
        {"dcCurrent",   invData.currentDc},
        {"status",      invData.inverterStatus},
        {"errorCode",   invData.inverterError}
      };
      ret.push_back({std::string("ChargerData"), responseMessage});
    }
  }
  else if(message.topic == "GetDischargerData")
  {
    int dischargerNumber = message.message.at("dischargerId").get<int>();
    if(dischargerNumber < m_catchpenny->GetNumberOfDischargers() && dischargerNumber >= 0)
    {
      InverterData invData = m_catchpenny->GetDischargerData(dischargerNumber);
      json responseMessage = {
        {"id",0},
        {"chargerId", dischargerNumber},
        {"powerAcL1", invData.powerAcPhase1},
        {"powerAcL2", invData.powerAcPhase2},
        {"powerAcL3", invData.powerAcPhase3},
        {"voltageAcL1", invData.voltageAcPhase1},
        {"voltageAcL2", invData.voltageAcPhase2},
        {"voltageAcL3", invData.voltageAcPhase3},
        {"currentAcL1", invData.currentAcPhase1},
        {"currentAcL2", invData.currentAcPhase2},
        {"currentAcL3", invData.currentAcPhase3},
        {"temperature", invData.inverterTemperature},
        {"acFrequency", invData.frequency},
        {"dcVoltage",   invData.voltageDc},
        {"dcCurrent",   invData.currentDc},
        {"status",      invData.inverterStatus},
        {"errorCode",   invData.inverterError}
      };
      ret.push_back({std::string("DischargerData"), responseMessage});
    }
  }
  else if(message.topic == "GetBatteryData")
  {
    /* TODO: fix this single/multiple battery issue */
    BatteryPackMetaData batteryData = m_catchpenny->GetBatteryData(0);
    json responseMessage = {
      {"id", 0},
      {"minCellVoltage",      batteryData.minCellVoltage},
      {"maxCellVoltage",      batteryData.maxCellVoltage},
      {"averageCellVoltage",  batteryData.averageCellVoltage},
      {"minCellTemperature",  batteryData.minCellTemperature},
      {"maxCellTemperature",  batteryData.maxCellTemperature},
      {"averageCellTemperature",batteryData.averageCellTemperature},
      {"minCellCapacity",     batteryData.minCellCapacity},
      {"maxCellCapacity",     batteryData.maxCellCapacity},
      {"averageCellCapacity", batteryData.averageCellCapacity},
      {"stateOfCharge",       batteryData.stateOfCharge},
      {"stateOfHealth",       batteryData.stateOfHealth},
      {"batteryMode","none"},
      {"batteryStatus",       batteryData.batteryStatus},
      {"batteryError",        batteryData.batteryError}
    };
    ret.push_back({std::string("BatteryData"), responseMessage});
  }
  else if(message.topic == "GetManagedDevices")
  {
    const int numberOfPowermeters = m_container->GetNumberOfPowerMeters(); 
    const int numberOfInverters = m_container->GetNumberOfInverters();
    json inverterArray = json::array();
    json powerMeterArray = json::array();

    for(int i = 0; i < numberOfPowermeters; i++)
    {
      const PowerMeterData data = m_container->GetPowerMeterDeviceData(i);
      const std::string status = PowerMeterDevice::ParseStatusCode(data.powerMeterStatus);
      json singlePowerMeter = {
        {"assetId", data.assetId},
        {"model", data.deviceModel},
        {"status", status}
      };
      powerMeterArray.emplace_back(singlePowerMeter);
    }
    for(int i = 0; i < numberOfInverters; i++)
    {
      const InverterData data = m_container->GetInverterDeviceData(i);
      const std::string status = InverterDevice::ParseStatusCode(data.inverterStatus);
      json singleInverter = {
        {"assetId", data.assetId},
        {"model", data.deviceModel},
        {"status", status}
      };
      inverterArray.emplace_back(singleInverter);
    }

    json responseMessage = {{"id",0}};
    responseMessage["powerMeters"] = powerMeterArray;
    responseMessage["inverters"] = inverterArray;
    ret.push_back({std::string("ManagedDevices"), responseMessage});
  }
  else if(message.topic == "GetPowerMeterData")
  {
    const std::string assetId = message.message.at("assetId").get<std::string>();
    if(m_container->ContainsPowerMeterDevice(assetId))
    {
      PowerMeterData data = m_container->GetPowerMeterDeviceData(assetId);
      json responseMessage = {
        {"assetId", assetId},
        {"deviceModel", data.deviceModel},
        {"powerAcL1", data.powerAcPhase1},
        {"powerAcL2", data.powerAcPhase2},
        {"powerAcL3", data.powerAcPhase3},
        {"apparentPowerL1", data.apparentPowerPhase1},
        {"apparentPowerL2", data.apparentPowerPhase2},
        {"apparentPowerL3", data.apparentPowerPhase3},
        {"reactivePowerL1", data.reactivePowerPhase1},
        {"reactivePowerL2", data.reactivePowerPhase2},
        {"reactivePowerL3", data.reactivePowerPhase3},
        {"currentAcL1", data.currentAcPhase1},
        {"currentAcL2", data.currentAcPhase2},
        {"currentAcL3", data.currentAcPhase3},
        {"voltageAcL1", data.voltageAcPhase1},
        {"voltageAcL2", data.voltageAcPhase2},
        {"voltageAcL3", data.voltageAcPhase3},
        {"acFrequency", data.frequency}
      };
      ret.push_back({std::string("PowerMeterData"), responseMessage});
    }
  }
  else if(message.topic == "GetInverterData")
  {
    const std::string assetId = message.message.at("assetId").get<std::string>();
    if(m_container->ContainsInverterDevice(assetId))
    {
      InverterData data = m_container->GetInverterDeviceData(assetId);
      json responseMessage = {
        {"assetId", assetId},
        {"deviceModel", data.deviceModel},
        {"powerAcL1", data.powerAcPhase1},
        {"powerAcL2", data.powerAcPhase2},
        {"powerAcL3", data.powerAcPhase3},
        {"powerFactor", data.powerFactor},
        {"currentAcL1", data.currentAcPhase1},
        {"currentAcL2", data.currentAcPhase2},
        {"currentAcL3", data.currentAcPhase3},
        {"voltageAcL1", data.voltageAcPhase1},
        {"voltageAcL2", data.voltageAcPhase2},
        {"voltageAcL3", data.voltageAcPhase3},
        {"acFrequency", data.frequency},
        {"powerDc", data.powerDc},
        {"currentDc", data.currentDc},
        {"voltageDc", data.voltageDc}
      };

      ret.push_back({std::string("InverterData"), responseMessage});
    }
  }
  else
  {
    //const Error error = {ErrorType::ERROR_MQTT_BAD_TOPIC, ErrorSeverityLevel::ERROR_WARNING};
    //m_et->PushBackError(error);
  }

  return ret;
}
