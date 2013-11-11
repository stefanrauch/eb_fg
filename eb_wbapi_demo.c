//
// eb_demo: quick hack demo for accessing Wishbone devices using dietrich's wb_api library
// 

//standard includes
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>

//etherbone
#include <etherbone.h>

//wishbone api
#include <wb_api.h>

//omitting routine "die" as shown in the native example: lateron, a FESA class should also not die
int main(int argc, const char** argv) {
  eb_status_t  status;
  eb_device_t  device;
  eb_socket_t  socket;
  
  const char* devName;
  uint64_t    mac;
  char        time[60];
  uint64_t    nsecs64;
  time_t      secs;
  const struct tm* tm;
  char 	      hostname[132];
  char        buff[132];


  if (argc < 2) {
    fprintf(stderr, "Syntax: %s <protocol/host/port\n", argv[0]);
    return 1;
  }
  //init stuff
  devName = argv[1];
  status = EB_OK;

  //open Etherbone device and socket
  status = wb_open(devName, &device, &socket);

  //get and print time
  if ((status = wb_wr_get_time(device, &nsecs64) == EB_OK)) {
    secs = (unsigned long)((double)nsecs64 / 1000000000.0);
    tm = localtime(&secs);
    strftime(time, sizeof(time), "%Y-%m-%d %H:%M:%S%Z", tm);
    printf("Current TAI      : %s\n", time);
  } 
  else printf("main: status %d, getting actual time from GSI_TM_LATCH\n", status);

  //get and print mac
  if ((status = wb_wr_get_mac(device, &mac) == EB_OK)) {
    printf("WR interface MAC : %12llx\n", mac);
  }
  else printf("main: status %d, error getting MAC from WR_ENDPOINT\n", status);
  
  //as a goody: print some info to display
  gethostname(hostname, 128);
  sprintf(buff, "   %8s%11llu%s", hostname, mac, time);
  //printf("buff %s\n", buff);
  wb_display_clear(device);
  wb_display_print(device, buff);
  
  wb_close(device, socket);
  return 0;
}
