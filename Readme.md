## Json configuration input example:
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

