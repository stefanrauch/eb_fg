//
// eb_tlu_demo: quick hack demo for using the TLU wishbone device via the native Etherbone library
// 

//standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

//Etherbone
#include <etherbone.h>

//Wishbone devices
#include <gsi_tm_latch.h>

#define FG_MAXNUM		6
#define FG_BASE 		0x1d000 						/* begin of startshared */
#define FG_PARAM_1	FG_BASE + FG_MAXNUM
#define MAXLINE 		100


static const char* program;


void itoa(unsigned int n, char s[], int base){
     int i;
 
     i = 0;
     do {       /* generate digits in reverse order */
         s[i++] = n % base + '0';   /* get next digit */
     } while ((n /= base) > 0);     /* delete it */
     s[i] = '\0';
 }


void die(const char* where, eb_status_t status) {
  fprintf(stderr, "%s: %s failed: %s\n",
    program, where, eb_status(status));
  exit(1);
}

int main(int argc, const char** argv) {
  eb_status_t status;
  eb_device_t device;
  eb_socket_t socket;
  eb_cycle_t  cycle;
  eb_address_t wrFG;
  int nDevices;
  eb_data_t   data, data1, data2, data3;

  const char* devName;
  int nFIFO;
  int nIterations;
  int nLatches = 0;
  char time[60];
  char buff[33];
  unsigned long long timestamp = 0;
  unsigned long long timestamp_start = 0;
  unsigned long long timestamp_stop  = 0;
  unsigned long long actu_timestamp = 0;
  unsigned long long prev_timestamp = 0;
  unsigned long long diff_timestamp = 0;
  unsigned long long clockFreq = 125000000;
  unsigned int       elapsed_time = 0;
  unsigned int       timePerEBCycle;
  
  time_t secs;
  const struct tm* tm;
  
  long l_i;
  char line[MAXLINE];
  FILE *fp;
  int linecount = 0;	

  
  program = argv[0];
  if (argc < 4) {
    fprintf(stderr, "reads value of timestamp from time stamp latch unit\n");
    fprintf(stderr, "Syntax: %s <protocol/host/port> <nIterations> <FIFO(0..n)>\n", argv[0]);
    return 1;
  }
  
  devName     = argv[1];
  nFIFO       = strtol(argv[3],0,10);
  nIterations = strtol(argv[2],0,10);
  
  /* Open a socket supporting only 32-bit operations.
   * As we are not exporting any slaves, we don't care what port we get => 0.
   * This function always returns immediately.
   * EB_ABI_CODE helps detect if the application matches the library.
   */
  if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_ADDR32|EB_DATA32, &socket)) != EB_OK)
    die("eb_socket_open", status);
  
  /* Open the remote device with 3 attempts to negotiate bus width.
   * This function is blocking and may stall the thread for up to 3 seconds.
   * If you need asynchronous open, see eb_device_open_nb.
   * Note: the supported widths can never be more than the socket supports.
   */
  if ((status = eb_device_open(socket, devName, EB_ADDR32|EB_DATA32, 3, &device)) != EB_OK)
    die("eb_device_open", status);
  
  /* Record the address of the device */
  wrFG				= FG_BASE;

  fp = fopen("LSA_data.txt","r");

  if (fp) {
    char line[MAXLINE];
    struct pset {
      int a;
      int l_a;
      int b;
      int l_b;
      int c;
      int n;
    } line_pset;

    int count = 0;

    while(fgets(line, MAXLINE, fp) != NULL) {
      if(sscanf(line, "%d %d %d %d %d %d\n", &line_pset.a, &line_pset.l_a, &line_pset.b, &line_pset.l_b, &line_pset.c, &line_pset.n) == 6 ) {
	      printf("%hi %hi %hi %hi %d %hi\n", line_pset.a,  line_pset.l_a,  line_pset.b,  line_pset.l_b,  line_pset.c,  line_pset.n);
//	printf("before open\n");fflush(stdout);       
 
        if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK)
          die("EP eb_cycle_open", status);
//	printf("after open\n");fflush(stdout);       
        
        eb_cycle_write(cycle, wrFG + count, EB_BIG_ENDIAN|EB_DATA32, line_pset.a); count += 4;
        eb_cycle_write(cycle, wrFG + count, EB_BIG_ENDIAN|EB_DATA32, line_pset.l_a); count += 4;
        eb_cycle_write(cycle, wrFG + count, EB_BIG_ENDIAN|EB_DATA32, line_pset.b); count += 4;
        eb_cycle_write(cycle, wrFG + count, EB_BIG_ENDIAN|EB_DATA32, line_pset.l_b); count += 4;
        eb_cycle_write(cycle, wrFG + count, EB_BIG_ENDIAN|EB_DATA32, line_pset.c); count += 4;
        eb_cycle_write(cycle, wrFG + count, EB_BIG_ENDIAN|EB_DATA32, line_pset.n); count += 4;
        
//	printf("before close\n");fflush(stdout);       
        if ((status = eb_cycle_close(cycle)) != EB_OK)
          die("EP eb_cycle_close", status); 
        
//	printf("after close\n");fflush(stdout);       
      }
    }
    //no more data for FG
    if ((status = eb_device_write(device, wrFG + count, EB_BIG_ENDIAN|EB_DATA32, 0xdeadbeef, 0, eb_block)) != EB_OK)
      die("EP eb_device_write", status);
      
    fclose(fp); 	
  }
  
  
  return 0;


  /*
  if ((status = eb_device_read(device, wrTLU + GSI_TM_LATCH_ATSHI, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block)) != EB_OK)
    die("TLU eb_device_read", status);
  timestamp = data;
  timestamp = (timestamp << 32); */
  

  /* Read data from FIFO in one cycle.
   * This demonstrates how to read multiple values in a single round trip.
   * eb_cycle_open always returns immediately.
   */

  for (l_i=0; 0; l_i++) {

    // >>>>>>>>>>>>>>>>>>>>>>>>> this is the important part >>>>>>>>>>>>>>
    if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK)
      die("EP eb_cycle_open", status);


    /* Queueing operations to a cycle never fails (EB_OOM is reported later) */
    /* read status of FIFOs is disabled, since a successful read is detected by the status returned by eb_cycle_close */
  //  eb_cycle_read(cycle, wrTLU + GSI_TM_LATCH_FIFO_READY, EB_BIG_ENDIAN|EB_DATA32, &data1);
    /* read high word of latched timestamp */
  //  eb_cycle_read(cycle, wrTLUFIFO + GSI_TM_LATCH_FIFO_FTSHI, EB_BIG_ENDIAN|EB_DATA32, &data2);
    /* read low word of latched timestamp */
 //   eb_cycle_read(cycle, wrTLUFIFO + GSI_TM_LATCH_FIFO_FTSLO, EB_BIG_ENDIAN|EB_DATA32, &data3);
    /* pop timestamp from FIFO */
 //   eb_cycle_write(cycle, wrTLUFIFO + GSI_TM_LATCH_FIFO_POP, EB_BIG_ENDIAN|EB_DATA32, 0xF);

    /* Because the cycle was opened with eb_block, this is a blocking call.
     * Upon termination, data and data2 will be valid.
     * For higher performance, use multiple asynchronous cycles in a pipeline.
     */

    /* for the TLU, "POP" yields an error in case no TS is available */
    if ((status = eb_cycle_close(cycle)) == EB_OK) {
      // <<<<<<<<<<<<<<<<<<<<<<< this was the important part
      /* do some gymnastics to get a human readible timestamp */
      timestamp = data2;
      //shift high word
      timestamp = (timestamp << 32);
      //add low word
      timestamp = timestamp + data3;

      actu_timestamp = timestamp;
      diff_timestamp = actu_timestamp - prev_timestamp;   
      prev_timestamp = actu_timestamp;
      nLatches++;

      printf("diff time_stamp of FIFO %d  : %llu [ns] \n", nFIFO, diff_timestamp<<3); //fflush (stdout);  
      printf("latched timestamp of FIFO %d: %llu [ns] \n", nFIFO, actu_timestamp<<3); fflush (stdout);
    }
  }


  /* close handler cleanly */
  if ((status = eb_device_close(device)) != EB_OK)
    die("eb_device_close", status);
  if ((status = eb_socket_close(socket)) != EB_OK)
    die("eb_socket_close", status);
  
  return 0;
}
