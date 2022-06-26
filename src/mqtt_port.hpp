#ifndef MQTT_PORT_HPP
#define MQTT_PORT_HPP
#include <vector>
#include <iostream>
#include <map>

#include "catchpenny.hpp"

class MqttResponder {
public:
  MqttResponder(){}
  /**
   * @brief 
   * 
   * @param topic 
   * @param message 
   */
  void ParseMessage(std::string& topic, std::string& message);
};

class AtParser {
public:
  AtParser(){}
};
/**
 * @brief this class manages the system's mqtt interface, its process function runs in a separate thread and if 
 *        a Get message is recieved -> via its MqttMessageParser it extracts the data structures from Catchpenny class and the Devices contained in the DeviceContainer
 *        and publishes the messages to the specified topics
 * 
 */
class MqttInterface {
public:
  MqttInterface(){}
  void Subscribe(std::vector<std::string>& subscribeChannels);
  void Publish(std::string& topic, std::string& message);
  void Process();
private:
  AtParser m_atParser;
  MqttResponder m_responder;
  std::shared_ptr<Catchpenny> m_catchpenny; 
  std::shared_ptr<DeviceContainer> m_container; /*container containing all the devices -> get the device data structures from this */
};

void MqttInterface::Process()
{
  /* 1) read the incoming messages from the device port
  *  2) 
  */
}
#endif //MQTT_PORT_HPP