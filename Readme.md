## Json configuration input example:
``` json
{
  "commPorts" : [
    {
    "name" : "modbusRtu1",
    "mode" : "rtu",
    "connection": "/dev/ttyS1",
    "settings": "38400E1",
    "debug": true,
    "response-timeout": 500,
    "byte-timeout": 500,
    "rtu": {
      "mode": "rs485",
      "rts": "down"
      }
    },
    {
      "name" : "modbusTcp1",
      "mode" : "tcp",
      "connection" : "127.0.0.1",
      "settings" : "5002"
    }
  ],
  "powerMeters" : [{
    "address" : 2,
    "interfacePort" : "modbusRTU1",
    "manufacturer" : "schneider",
    "model" : "a9mem3255"
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

