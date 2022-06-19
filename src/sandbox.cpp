#include <string>
#include "enerman.hpp"
#include "cp_utils.hpp"

int main()
{
  JsonLoader loader;
  std::cout << std::filesystem::current_path();
  if(loader.LoadJsonFromFile(std::string("../example_config.json")) == false)
    throw std::runtime_error("wrong input json file");
  
  Enerman manager;
  if(manager.BuildDevices(loader.GetJsonConfig()) != EnermanReturnCode::ENERMAN_OK)
    throw std::runtime_error("something wrongk");

  return 0;
}