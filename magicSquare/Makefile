


TIME_UNIX_CPP = ../../minorGems/system/unix/TimeUnix.cpp

PATH_LINUX_CPP = ../../minorGems/io/file/linux/PathLinux.cpp




magicSquare: magicSquare.cpp ${TIME_UNIX_CPP} ${PATH_LINUX_CPP} magicSquare6.o
	g++ -g -o magicSquare -I../.. magicSquare.cpp ${TIME_UNIX_CPP} ${PATH_LINUX_CPP}


magicSquare6.o: magicSquare6.cpp
	g++ -g -c magicSquare6.cpp -I../..
