#include "device.hpp"


template<int (BaseDevice::*functor)()>
int DeviceContainer::ForEachDevice()
{
  int containerSize = m_container.size();
  for(int i = 0; i < containerSize; i++)
  {
    BaseDevice *device = m_container[i];
    (device->*functor)();
  }
}