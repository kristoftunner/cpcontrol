#include <string>
#include "enerman.hpp"
#include "cp_utils.hpp"
#include <thread>

int main()
{
  JsonLoader loader;
  std::cout << std::filesystem::current_path();
  if(loader.LoadJsonFromFile(std::string("../docs/example_config.json")) == false)
    throw std::runtime_error("wrong input json file");
  
  Enerman manager;
  if(manager.BuildDevices(loader.GetJsonConfig()) != EnermanReturnCode::ENERMAN_OK)
    throw std::runtime_error("something wrongk");

  std::thread catchpennyThread([&](){manager.ExecEnergyManagment();});
  std::thread catchpennyModbusThread([&](){manager.ExecCatchpennyModbusTcp();});
  std::thread catchpennyMqttThread([&](){manager.ExecCatchpennyMqtt();});
  catchpennyThread.join();
  catchpennyModbusThread.join();
  catchpennyMqttThread.join();
}