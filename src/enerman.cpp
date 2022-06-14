#include "enerman.hpp"

EnermanReturnCode Enerman::BuildDevices(std::vector<std::map<std::string,std::string>> deviceDict)
{
  for(auto deviceConfigMap : deviceDict)
  {
    /* build the device -> append to the container */
    std::string type = deviceConfigMap.at("type");
    if(type == "meter")
    {
      MeterDevice *device = new MeterDevice();
      if(device->Initialize(deviceConfigMap) == 0)
        m_deviceContainer.AppendDevice(device);
      else
        return EnermanReturnCode::ENERMAN_CONFIG_ERR;
    }
    else if(type == "inverter")
    {
      ;
    }
  }

  return EnermanReturnCode::ENERMAN_OK;
}