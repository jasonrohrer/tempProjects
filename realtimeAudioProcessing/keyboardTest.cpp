#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdbool.h>
#include <stdio.h>

int main()
{
    Display* dpy = XOpenDisplay(NULL);
    char keys_return[32];
    XQueryKeymap( dpy, keys_return );
    KeyCode kc2 = XKeysymToKeycode( dpy, XK_Shift_L );
    bool bShiftPressed = !!( keys_return[ kc2>>3 ] & ( 1<<(kc2&7) ) );
    printf("Shift is %spressed\n", bShiftPressed ? "" : "not ");
    XCloseDisplay(dpy);
}
