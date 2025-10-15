
#include <linux/uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>




void emit( int fd, int type, int code, int val ) {
    struct input_event ie;

    ie.type = type;
    ie.code = code;
    ie.value = val;
    /* timestamp values below are ignored */
    ie.time.tv_sec = 0;
    ie.time.tv_usec = 0;

    write( fd, &ie, sizeof(ie) );
    }


 
int main() {
    struct uinput_user_dev uud;
    int version, rc, fd;
    
    fd = open( "/dev/uinput", O_WRONLY | O_NONBLOCK );

    if( fd == -1 ) {
        printf( "Failed to open /dev/uinput\n" );
        return 1;
        }
    
    ioctl( fd, UI_SET_EVBIT, EV_KEY );
    ioctl( fd, UI_SET_KEYBIT, KEY_SPACE );

    memset( &uud, 0, sizeof(uud) );
    snprintf( uud.name, UINPUT_MAX_NAME_SIZE, "Tourbox Elite" );
    write( fd, &uud, sizeof(uud) );

    ioctl( fd, UI_DEV_CREATE );

    sleep(1);

    int i;
    for( i=0; i<100; i++ ) {
        
        /* Key press, report the event, send key release, and report again */
        emit( fd, EV_KEY, KEY_SPACE, 1 );
        emit( fd, EV_SYN, SYN_REPORT, 0 );
        emit( fd, EV_KEY, KEY_SPACE, 0 );
        emit( fd, EV_SYN, SYN_REPORT, 0 );
        sleep( 1 );
        }
    
        
    
    close( fd );
    
    return 0;
    }
