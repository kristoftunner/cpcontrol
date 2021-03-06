/**
 * @brief libmodbuspp clock-server example
 *
 * Shows how to use libmodbuspp to build a MODBUS time server.
 *
 * @code
 * clock-server [-m tcp|rtu] [-c host_ip or serial_port] [-s ip_port or serial_settings]
 * @endcode
 *
 * Once the server has started you can test it with mbpoll :
 *
 * @code
    $ mbpoll -mtcp -p1502 -a10 -t3 -c8 localhost
    mbpoll 1.4-12 - FieldTalk(tm) Modbus(R) Master Simulator
    Copyright © 2015-2019 Pascal JEAN, https://github.com/epsilonrt/mbpoll
    This program comes with ABSOLUTELY NO WARRANTY.
    This is free software, and you are welcome to redistribute it
    under certain conditions; type 'mbpoll -w' for details.

    Protocol configuration: Modbus TCP
    Slave configuration...: address = [10]
                            start reference = 1, count = 8
    Communication.........: localhost, port 1502, t/o 1.00 s, poll rate 1000 ms
    Data type.............: 16-bit register, input register table

    -- Polling slave 10... Ctrl-C to stop)
    [1]:  37
    [2]:  40
    [3]:  15
    [4]:  28
    [5]:  11
    [6]:  2019
    [7]:  4
    [8]:  332
 * @endcode
 *
 * These data correspond to 15:40:37 on Thursday 28/11/2019, 332nd day of the
 * year. To read the time difference from GMT (in seconds) :
 *
 * @code
    $ mbpoll -mtcp -p1502 -a10 -t4:int -B -1 localhost
      ....
      -- Polling slave 10...
      [1]:  3600
 * @endcode
 *
 * To set the offset to GMT-2 hours :
 *
 * @code
    $ mbpoll -mtcp -p1502 -a10 -t4:int -B localhost -- -7200
      ....
      Written 1 references.
 * @endcode
 */
#include <ctime>
#include <iostream>
#include <modbuspp.h>
#include <modbuspp/popl.h>

using namespace std;
using namespace Modbus;

const int SlaveAddress = 10;

Server srv; // instantiates new MODBUS Server

// -----------------------------------------------------------------------------
// Signal trap, triggers during a CTRL+C or if kill is called
void
sighandler (int sig) {

  srv.close();
  cout << "everything was closed." << endl << "Have a nice day !" << endl;
  exit (EXIT_SUCCESS);
}

// -----------------------------------------------------------------------------
int main (int argc, char **argv) {
  Net net;
  string connection;
  string settings;
  string mode;

  time_t now, before;
  tm * t;

  // ModBus Data
  bool daylight;// daylight saving time, true = summer time
  Data<int32_t> gmtoff; // GMT offset, +/- minutes, Big endian order : ABCD
  uint16_t mb_time[8]; // date and time
  Data<uint32_t> myData[4] = {0,1,2,3};
  // parsing options from the command line
  // watch https://github.com/badaix/popl to understand how we use popl ...

  try {

    net = Tcp;

    cout << "Modbus Time Server" << endl;
    // CTRL+C and kill call triggers the trap sighandler()
    signal (SIGINT, sighandler);
    signal (SIGTERM, sighandler);
    cout << "Press CTRL+C to stop... " << endl << endl;

    srv.setBackend (net, string("127.0.0.1"), string("502"));
    srv.setDebug();
    srv.setRecoveryLink();

    BufferedSlave * slv = &(srv.addSlave (SlaveAddress)); // Adding a new slave to the server
    cout << "Slave id: " << slv->number() << endl << endl;
    if (srv.debug()) {
      cout << "Mapping of registers:" << endl
           << "--- Input Registers" << endl
           << "@ Reg.  Size    Description" << endl
           << "1       16-bit  Seconds (0-60), unsigned" << endl
           << "2       16-bit  Minutes (0-59), unsigned" << endl
           << "3       16-bit  Hours (0-23), unsigned" << endl
           << "4       16-bit  Day of the month (1-31), unsigned" << endl
           << "5       16-bit  Month (1-12), unsigned" << endl
           << "6       16-bit  Year e.g. 2019, unsigned" << endl
           << "7       16-bit  Day of the week (0-6, Sunday = 0), unsigned" << endl
           << "8       16-bit  Day in the year (1-366, 1 Jan = 1), unsigned" << endl
           << "--- Holding Registers" << endl
           << "@ Reg.  Size    Description" << endl
           << "1       32-bit  number of seconds to add to UTC to get local time, signed" << endl
           << "--- Coils" << endl
           << "@ Reg.  Size    Description" << endl
           << "1       1-bit   Daylight saving time" << endl << endl;
    }


    /* --- Holding Registers
     @ Reg.  Size    Description
     1       32-bit  number of seconds to add to UTC to get local time, signed */
    slv->setBlock (InputRegister, 16, 1000);

    std::string systemSerialNumber("1023");
    //slv->writeInputRegisters(1000,myData, 4);
    size_t size = systemSerialNumber.size();
    int ret = slv->writeInputRegisters(1005, reinterpret_cast<uint16_t*>(systemSerialNumber.data()), systemSerialNumber.size()/2);

    if (srv.open ()) { // open a connection
      cout << "Listening server on " <<
           srv.connection() << ":" << srv.settings() << "..." << endl << endl;

      do {

        /*now = time (nullptr);

        if (now > before) {

          before = now;

          // update daylight saving time from holding register
          slv->readCoil (1, daylight);

          // update GMT offset from holding register
          slv->readRegister (1, gmtoff);

          // calculate the epoch time
          now += (daylight ? 3600 : 0) + gmtoff;

          t = gmtime (&now);
          mb_time[0] = t->tm_sec;
          mb_time[1] = t->tm_min;
          mb_time[2] = t->tm_hour;
          mb_time[3] = t->tm_mday;
          mb_time[4] = t->tm_mon + 1;
          mb_time[5] = t->tm_year + 1900;
          mb_time[6] = t->tm_wday;
          mb_time[7] = t->tm_yday + 1;

          // update the input registers
          slv->writeInputRegisters (1, mb_time, 8);
        }*/

        srv.poll (100);
      }
      while (srv.isOpen());
    }
  }
  catch (std::exception & e) {

    cerr << "Error: " << e.what() << endl;
  }
  catch (...) {

    cerr << "Unattended exception !" << endl;
  }

/* ========================================================================== */


  return 0;
}