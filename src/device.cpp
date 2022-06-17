#include "device.hpp"

template<int (PowerMeterDevice::*functor)()>
int DeviceContainer::ForEachDevice()
{
  int containerSize = m_powerMeterContainer.size();
  for(int i = 0; i < containerSize; i++)
  {
    PowerMeterDevice *device = m_powerMeterContainer[i];
    (device->*functor)();
  }
}

template<int (InverterDevice::*functor)()>
int DeviceContainer::ForEachDevice()
{
  int containerSize = m_inverterContainer.size();
  for(int i = 0; i < containerSize; i++)
  {
    InverterDevice *device = m_inverterContainer[i];
    (device->*functor)();
  }
}


int SchneiderPM5110Meter::Initialize(json config) 
{
  
}

int SchneiderPM5110Meter::ReadMeasurements()
{

}

