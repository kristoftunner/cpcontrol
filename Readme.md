# Dependecies, prerequisites:
- build paho.mqtt.cpp from source: https://github.com/eclipse/paho.mqtt.cpp
- build and install libmodbus: https://github.com/stephane/libmodbus
- build and install libmodbuspp: https://github.com/epsilonrt/libmodbuspp

# Json configuration input example:
``` json
{
  "commPorts" : [
    {
    "name" : "modbusRtu1",
    "mode" : "rtu",
    "port": "/dev/ttyS1",
    "settings": "38400E1",
    "debug": true,
    "responseTimeout": 500,
    "byteTimeout": 500,
    "serialMode": "rs485",
    "rts": "down"
    },
    {
    "name" : "modbustcp1",
    "mode": "tcp",
    "connection": "localhost",
    "port": "1502",
    "recovery-link": true,
    "debug": true,
    "response-timeout": 500,
    "byte-timeout": 500
  }
  ],
  "powerMeters" : [{
    "address" : 2,
    "interfacePort" : "modbusRTU1",
    "manufacturer" : "schneider",
    "model" : "schneidrPM5110"
  },
  {
    "address" : 1,
    "interfacePort" : "modbusTcp1",
    "manufacturer" : "janitza",
    "model" : "umg96rm-m"
  }
  ],
  "inverters" : [{
    "manufacturer" : "SMA",
    "model" : "SunnyTripowerCore1",
    "interface" : "modbusRS485"
  },
  {
    "manufacturer" : "huawei",
    "model" : "sun2000-3ktl-m1",
    "interface" : "modbustcp"
  }
  ]
}
```

# Status and error codes:
## Inverter status and error codes:
Inverter status events are a combination of the connection status checked by ```DeviceConnectionChecker``` class and the actual state of the specific inverter and this rule is applied to the error as well
- status:
  - CONNECTED_IDLE
  - CONNECTED_THROTTLED
  - DISCONNECTED
- error:
  - DISCONNECTED_BUS_ERROR
## Powermeter status and error codes:
- status:
  - CONNECTED
  - DISCONNECTED
error:
  - DISCONNECTED_BUS_ERROR
## Battery status and error codes:
- status:
  - CONNECTED_OK
  - CONNECTED_ERROR
  - DISCONNECTED
- error:
  - OVERVOLTAGE_ERROR
  - OVERTEMPERATURE_ERROR
  - UNDERVOLTAGE_ERROR
  - UNDERTEMPERATURE_ERROR
## Catchpenny status and error codes:
- status:
  - IDLE
  - CHARGIN
  - DISCHARGING
  - HALTED_ON_ERROR
- error:
  - various battery errors

## Powermeter device status and error codes:
- status:
  - CONNECTED
  - DISCONNECTED
- error:
  - NOERROR
  - DISCONNECTED_BUS_ERROR
## Feature backlog:
- Parse the system information from a config file(ini?) and do not hardcode those informations
- this ```assetId``` thing should be resolved: how to fill all the devices assetId, how to use it in the code 

### TODO/bugfixes
- check every ```json.at("")``` that the json actually contains the value -> otherwise it runs on a exception 
