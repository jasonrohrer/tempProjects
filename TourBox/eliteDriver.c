#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <stdint.h>

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
  for(int i=0;i<x;i++) printf("%02X ", inbuf[i]); puts("");

  // now send 94 config bytes
  uint8_t outConfig[] = { 0xb5, 0x00, 0x5d, 0x4, 0x08,
      0x05, 0x08, 0x06, 0x08, 0x07, 0x08, 0x08, 0x08, 0x09, 0x08, 0x0b, 0x08, 0xc, 0x08, 0x0d, 0x08,
      0x0e, 0x08, 0xf, 0x08, 0x26, 0x08, 0x27, 0x08, 0x28, 0x08, 0x29, 0x08, 0x3b, 0x08, 0x3c, 0x08,
      0x3d, 0x08, 0x3e, 0x08, 0x3f, 0x08, 0x40, 0x08, 0x41, 0x08, 0x42, 0x08, 0x43, 0x08, 0x44, 0x08,
      0x45, 0x08, 0x46, 0x08, 0x47, 0x08, 0x48, 0x08, 0x49, 0x08, 0x4a, 0x08, 0x4b, 0x08, 0x4c, 0x08,
      0x4d, 0x08, 0x4e, 0x08, 0x4f, 0x08, 0x50, 0x08, 0x51, 0x08, 0x52, 0x08, 0x53, 0x08, 0x54, 0x08,
      0xa8, 0x08, 0xa9, 0x08, 0xaa, 0x08, 0xab, 0x08, 0xfe };
  x=0;
  r = libusb_bulk_transfer(h, EP_OUT, out, sizeof(out), &x, TO);
  printf("OUT r=%d xfer=%d\n", r, x);

  
  
  // Try to read a series of responses

  
  for( int z=0; z<100; z++ ) {    
      uint8_t inbuf[512];
      r = libusb_bulk_transfer(h, EP_IN, inbuf, sizeof(inbuf), &x, TO);
      printf("IN r=%d xfer=%d\n", r, x);
      for(int i=0;i<x;i++) printf("%02X ", inbuf[i]); puts("");
      }


  
  libusb_release_interface(h, IFACE);
close:
  libusb_close(h);
out:
  libusb_exit(ctx);
  return 0;
}
