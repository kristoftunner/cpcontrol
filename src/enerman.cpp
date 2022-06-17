#include "enerman.hpp"

EnermanReturnCode Enerman::BuildDevices(json devicesConfig)
{
  for(auto deviceConfigMap : deviceDict)
  {
    /* build the device -> append to the container */
    std::string type = deviceConfigMap.at("type");
    if(type == "meter")
    {
      PowerMeterDevice *device = new PowerMeterDevice();
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