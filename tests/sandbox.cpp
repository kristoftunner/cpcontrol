#include <string>
#include <thread>
#include <memory>

#include "enerman.hpp"
#include "cp_utils.hpp"
#include "error_tracker.hpp"

int main()
{
  JsonLoader loader;
  std::cout << std::filesystem::current_path();
  if(loader.LoadJsonFromFile(std::string("../docs/example_config.json")) == false)
    throw std::runtime_error("wrong input json file");

  std::shared_ptr<ErrorTracker> tracker = std::make_shared<ErrorTracker>();
  Enerman manager(tracker);
  if(manager.BuildDevices(loader.GetJsonConfig()) != EnermanReturnCode::ENERMAN_OK)
    throw std::runtime_error("something wrongk");

  std::thread catchpennyThread([&](){manager.ExecEnergyManagment();});
  std::thread catchpennyModbusThread([&](){manager.ExecCatchpennyModbusTcp();});
  std::thread catchpennyMqttThread([&](){manager.ExecCatchpennyMqtt();});
  catchpennyThread.join();
  catchpennyModbusThread.join();
  catchpennyMqttThread.join();
}