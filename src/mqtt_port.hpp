#ifndef MQTT_PORT_HPP
#define MQTT_PORT_HPP
#include <vector>
#include <iostream>
#include <map>

#include "catchpenny.hpp"
#include "mqtt/client.h"
//#include "error_tracker.hpp"

struct MqttMessage {
  std::string topic;
  json message;
};

/**
 * @brief this class responds to the 
 * 
 */
class MqttMessageResponder {
public:
  MqttMessageResponder(){}
  
  /**
   * @brief this function will act on the message reveived via the mqtt interface and will return a map of mqtt messages(topic-message)
   * 
   * @param topic 
   * @param message 
   * @return std::map<std::string,std::string> 
   */
  std::vector<MqttMessage> ActOnMessage(const MqttMessage& message);
  void SetCatchpenny(std::shared_ptr<Catchpenny> catchpenny){m_catchpenny = catchpenny;}
  void SetContainer(std::shared_ptr<DeviceContainer> container){m_container = container;}
private:
  std::shared_ptr<Catchpenny> m_catchpenny; 
  std::shared_ptr<DeviceContainer> m_container; /*container containing all the devices -> get the device data structures from this */
};

class AtParser {
public:
  AtParser(){}
};

/**
 * @brief mqtt interface base class for different mqtt interface implementations, like paho mqtt client or a serial based mqtt, like SIMCOM or Quectel modules
 * 
 */
class MqttInterface {
public:
  MqttInterface() = default;
  virtual ~MqttInterface() = default;
  virtual void Subscribe(const std::vector<std::string>& topicsToSubscribe) = 0;
  virtual void Publish(const MqttMessage& message) = 0;
  virtual void Listen() = 0;
  virtual bool IsConnected() = 0;
protected:
  std::vector<std::string> m_subscribedTopics;
  MqttMessageResponder m_messageResponder;
};

struct MqttConfig {
  std::string serverAddres;
  std::string clientId;
  std::string userName;
  std::string password;
  std::vector<int> qos;
};

class PahoMqttPort : public MqttInterface {
public:
  PahoMqttPort(){};
  virtual void Subscribe(const std::vector<std::string>& subscribeChannels) override;
  virtual void Publish(const MqttMessage& message) override;
  virtual void Listen() override;
  virtual bool IsConnected() override;

  bool Initailize(const MqttConfig& config);
  void SetMqttResponder(MqttMessageResponder responder){m_messageResponder = responder;}
private:
  std::shared_ptr<mqtt::client> m_client;
  std::vector<int> m_qosCollection;
};

/**
 * @brief this class manages the system's mqtt interface, its process function runs in a separate thread and if 
 *        a Get message is recieved -> via its MqttMessageParser it extracts the data structures from Catchpenny class and the Devices contained in the DeviceContainer
 *        and publishes the messages to the specified topics
 * 
 */
class SerialMQTTPort : public MqttInterface {
public:
  SerialMQTTPort() = default;
  virtual void Subscribe(const std::vector<std::string>& subscribeChannels) override;
  virtual void Publish(const MqttMessage& message) override;
  virtual void Listen() override;
};

#endif //MQTT_PORT_HPP