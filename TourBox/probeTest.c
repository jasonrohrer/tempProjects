#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define VID 0xC251
#define PID 0x2005
#define IFACE 1     // <-- set to your Vendor-Specific interface number
#define EP_OUT 0x02 // <-- set from lsusb -v
#define EP_IN  0x82 // <-- set from lsusb -v
#define TO 500

int main(void){
  libusb_context* ctx=NULL; libusb_device_handle* h=NULL;
  int r = libusb_init(&ctx); if(r<0) return r;

  h = libusb_open_device_with_vid_pid(ctx, VID, PID);
  if(!h){ fprintf(stderr,"open fail\n"); goto out; }

  libusb_set_auto_detach_kernel_driver(h, 1);
  if((r = libusb_claim_interface(h, IFACE))){ fprintf(stderr,"claim: %s\n", libusb_strerror(r)); goto close; }

  // Send a small test OUT (replace with your captured init bytes)
  // If you have a Windows sniff, paste the first OUT packet here:
  uint8_t out[] = { 0x55, 0x00, 0x07, 0x88, 0x94, 0x00, 0x1a, 0xfe };
  int x=0;
  r = libusb_bulk_transfer(h, EP_OUT, out, sizeof(out), &x, TO);
  printf("OUT r=%d xfer=%d\n", r, x);

  // read one response, should be 26 bytes
  uint8_t inbuf[512];
  r = libusb_bulk_transfer(h, EP_IN, inbuf, sizeof(inbuf), &x, TO);
  printf("IN r=%d xfer=%d\n", r, x);
  int i;
  
  for(i=0;i<x;i++) printf("%02X ", inbuf[i]); puts("");

  // now send 94 config bytes
  uint8_t outConfig[] = { 0xb5, 0x00, 0x5d, 0x4, 0x00,
      0x05, 0x00, 0x06, 0x00, 0x07, 0x00, 0x08, 0x00, 0x09, 0x00, 0x0b, 0x00, 0xc, 0x00, 0x0d, 0x00,
      0x0e, 0x00, 0xf, 0x00, 0x26, 0x00, 0x27, 0x00, 0x28, 0x00, 0x29, 0x00, 0x3b, 0x00, 0x3c, 0x00,
      0x3d, 0x00, 0x3e, 0x00, 0x3f, 0x00, 0x40, 0x00, 0x41, 0x00, 0x42, 0x00, 0x43, 0x00, 0x44, 0x00,
      0x45, 0x00, 0x46, 0x00, 0x47, 0x00, 0x48, 0x00, 0x49, 0x00, 0x4a, 0x00, 0x4b, 0x00, 0x4c, 0x00,
      0x4d, 0x00, 0x4e, 0x00, 0x4f, 0x00, 0x50, 0x00, 0x51, 0x00, 0x52, 0x00, 0x53, 0x00, 0x54, 0x00,
      0xa8, 0x00, 0xa9, 0x00, 0xaa, 0x00, 0xab, 0x00, 0xfe };
  x=0;
  /*
  int try;
  for( try = 4; try < 94; try += 2 ) {
      printf( "Setting byte %d\n", try );

      uint8_t tempConfig[94];
      memcpy( tempConfig, outConfig, 94 );
      tempConfig[ try ] = 0x08;

      r = libusb_bulk_transfer(h, EP_OUT, tempConfig, sizeof(tempConfig), &x, TO);
      printf("OUT r=%d xfer=%d\n", r, x);

      printf( "Enter for next: " );

      getchar();
      }
  */
  outConfig[4] = 0x08;
  int try;
  for( try=0; try < 16; try++ ) {
      printf( "Setting upper four bits of byte 4 to %d\n", try );
      outConfig[4] = 0x08 | try << 4;
      
      r = libusb_bulk_transfer(h, EP_OUT, outConfig, sizeof(outConfig), &x, TO);
      printf("OUT r=%d xfer=%d\n", r, x);

      printf( "Turn knob to test\n" );
      int z;
      for( z=0; z<10; z++ ) {    
          uint8_t inbuf[512];
          r = libusb_bulk_transfer(h, EP_IN, inbuf, sizeof(inbuf), &x, TO);
          printf("IN r=%d xfer=%d\n", r, x);
          for(i=0;i<x;i++) printf("%02X ", inbuf[i]); puts("");
          }
      
      printf( "Enter for next: " );

      getchar();
      
      }
   
  
  
  // Try to read a series of responses

  int z;
  for( z=0; z<100; z++ ) {    
      uint8_t inbuf[512];
      r = libusb_bulk_transfer(h, EP_IN, inbuf, sizeof(inbuf), &x, TO);
      printf("IN r=%d xfer=%d\n", r, x);
      for(i=0;i<x;i++) printf("%02X ", inbuf[i]); puts("");

      FILE *commandOutput = popen( "xprop -id $(xprop -root -f _NET_ACTIVE_WINDOW 0x \" \\$0\\n\" _NET_ACTIVE_WINDOW | awk '{print $2}') WM_NAME | awk -F'=' '{print $2}' | sed 's/\"//g'", "r" );
      if( commandOutput != NULL ) {

          char buffer[1024];
          if( fgets( buffer, sizeof( buffer ), commandOutput ) != NULL ) {
              printf( "active window = %s\n", buffer );
              }
          pclose( commandOutput );
          }
      
      }


  
  libusb_release_interface(h, IFACE);
close:
  libusb_close(h);
out:
  libusb_exit(ctx);
  return 0;
}
