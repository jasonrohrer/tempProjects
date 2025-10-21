

gcc -std=c89 -ffreestanding -fno-builtin -Wall -Wextra -Werror \
    -Wstrict-prototypes -Wmissing-prototypes -Wconversion -Wshadow \
    -c -o program.o program.c

# audit link to make sure we don't have stdlib dependencies by accident
rm -f auditOut
echo
echo
echo "***  Building audit build to make sure we don't use stdlib"
gcc --entry main -o auditOut program.o -nostdlib

if [ -e auditOut ]; then
      echo "***  Success"
	  rm auditOut
  else
      echo "xxx  Audit build failed"
	  exit 1
  fi


echo "***  Linking program"
gcc -o program program.o