#include "mqtt_port.hpp"

using namespace std::chrono;

void PahoMqttPort::Subscribe(std::vector<std::string>& topicsToSubscribe)
{
  for(const auto& topic : topicsToSubscribe)
  {
    /* TODO: do some error handling here */
    m_client->subscribe(topic, m_qosCollection);
    m_subscribedTopics.push_back(topic);
  }
}

void PahoMqttPort::Publish(const MqttMessage& message)
{
  m_client->publish(message.first, message.second.data(), message.second.size()); /* TODO: revisit this one for QOS */
}

void PahoMqttPort::Listen() 
{
  while(true)
  {
    auto msg = m_client->consume_message();

    if(msg)
    {
      /* forward the message to the controller(?)consumer(?) */
      MqttMessage recvMsg = {msg->get_topic(), msg->to_string()};
      std::vector<MqttMessage> pubMessages = m_messageResponder.ActOnMessage(recvMsg);
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
  return true;
}

bool PahoMqttPort::Initailize(const mqttConfig& config)
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
  mqtt::connect_response m_response = m_client->connect(connOptions);
  if(IsConnected())
    return true;
  else
    return false;
}
