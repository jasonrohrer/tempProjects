#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>



int main(void){
    const char *path = "/dev/ttyACM0";

    int fileDescriptor = open( path, O_RDWR | O_SYNC );

    if( fileDescriptor == -1 ) {
        printf( "Error opening TourBox Elite device path %s\n", path );
        return 1;
        }

    // we were dropping bytes and seeing incomplete responses
    // make tty raw to get raw bytes.
    struct termios tty;

    if( tcgetattr( fileDescriptor, &tty ) != 0 ) {
        printf( "Error getting terminal atributes from "
                "TourBox Elite device path %s\n", path );
        close( fileDescriptor );
        return 1;
        }

    cfmakeraw( &tty );

    if( tcsetattr( fileDescriptor, TCSANOW, &tty ) != 0 ) {
        printf( "Error setting terminal atributes for "
                "TourBox Elite device path %s\n", path );
        close( fileDescriptor );
        return 1;
        }
    


    // send 8-byte init sequence
    uint8_t out[] = { 0x55, 0x00, 0x07, 0x88, 0x94, 0x00, 0x1a, 0xfe };

    ssize_t bytesWritten = write( fileDescriptor, out, sizeof( out ) );

    if( bytesWritten == -1 ) {
        printf( "Error writing to TourBox Elite device file %s\n", path );
        close( fileDescriptor );
        return 1;
        }

    printf( "Wrote %d bytes (out of %d) to TourBox device\n", bytesWritten,
            sizeof( out ) );

    // read one response, should be 26 bytes
    uint8_t inputBuffer[512];

    ssize_t bytesRead = read( fileDescriptor, inputBuffer,
                              sizeof( inputBuffer ) - 1 );

    if( bytesRead == -1 ) {
        printf( "Error reading from TourBox Elite device file %s\n", path );
        close( fileDescriptor );
        return 1;
        }

    printf( "Read %d bytes from TourBox device (expecting 26)\n", bytesRead );
    
    for( int i=0; i<bytesRead; i++ ) {
        printf( "%02X ", inputBuffer[i] );
        }
    printf( "\n" );
    

  // now send 94 config bytes
  uint8_t outConfig[] = { 0xb5, 0x00, 0x5d, 0x4, 0x08,
      0x05, 0x08, 0x06, 0x08, 0x07, 0x08, 0x08, 0x08, 0x09, 0x08, 0x0b, 0x08, 0xc, 0x08, 0x0d, 0x08,
      0x0e, 0x08, 0xf, 0x08, 0x26, 0x08, 0x27, 0x08, 0x28, 0x08, 0x29, 0x08, 0x3b, 0x08, 0x3c, 0x08,
      0x3d, 0x08, 0x3e, 0x08, 0x3f, 0x08, 0x40, 0x08, 0x41, 0x08, 0x42, 0x08, 0x43, 0x08, 0x44, 0x08,
      0x45, 0x08, 0x46, 0x08, 0x47, 0x08, 0x48, 0x08, 0x49, 0x08, 0x4a, 0x08, 0x4b, 0x08, 0x4c, 0x08,
      0x4d, 0x08, 0x4e, 0x08, 0x4f, 0x08, 0x50, 0x08, 0x51, 0x08, 0x52, 0x08, 0x53, 0x08, 0x54, 0x08,
      0xa8, 0x08, 0xa9, 0x08, 0xaa, 0x08, 0xab, 0x08, 0xfe };


  bytesWritten = write( fileDescriptor, outConfig,
                        sizeof( outConfig ) );
  
  if( bytesWritten == -1 ) {
      printf( "Error writing to TourBox Elite device file %s\n", path );
      close( fileDescriptor );
      return 1;
      }
  
  printf( "Wrote %d bytes (out of %d) to TourBox device\n", bytesWritten,
          sizeof( outConfig ) );
  

  
  // Try to read a series of responses

  
  for( int z=0; z<100; z++ ) {    
      bytesRead = read( fileDescriptor, inputBuffer,
                        sizeof( inputBuffer ) - 1 );
      
      if( bytesRead == -1 ) {
          printf( "Error reading from TourBox Elite device file %s\n", path );
          close( fileDescriptor );
          return 1;
          }
      
      printf( "Read %d bytes from TourBox device\n", bytesRead );
      
      for( int i=0; i<bytesRead; i++ ) {
        printf( "%02X ", inputBuffer[i] );
          }
      printf( "\n" );
  
      FILE *commandOutput = popen( "xprop -id $(xprop -root -f _NET_ACTIVE_WINDOW 0x \" \\$0\\n\" _NET_ACTIVE_WINDOW | awk '{print $2}') WM_NAME | awk -F'=' '{print $2}' | sed 's/\"//g'", "r" );
      if( commandOutput != NULL ) {
          
          char buffer[1024];
          if( fgets( buffer, sizeof( buffer ), commandOutput ) != NULL ) {
              printf( "active window = %s\n", buffer );
              }
          pclose( commandOutput );
          }
      }
  
  if( close( fileDescriptor ) == -1 ) {
      printf( "Error closing TourBox Elite device file\n" );
      return 1;
      }
  
  return 0;
}
