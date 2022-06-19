#ifndef MQTT_PORT_HPP
#define MQTT_PORT_HPP

class MqttPort {
public:
  MqttPort(){}
  void Subscribe(std::vector<std::string> subscribeChannels);
  void Publish()
};
#endif //MQTT_PORT_HPP