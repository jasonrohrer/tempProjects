cat /usr/include/linux/input.h | grep "#define KEY_" | sed "s/\t.*//g" | sed "s/#define //"
