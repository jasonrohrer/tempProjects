


TIME_UNIX_CPP = ../../minorGems/system/unix/TimeUnix.cpp

PATH_LINUX_CPP = ../../minorGems/io/file/linux/PathLinux.cpp



all: magicSquare magicSquare6Test



magicSquare: magicSquare.cpp ${TIME_UNIX_CPP} ${PATH_LINUX_CPP} magicSquare6.o
	g++ -g -o magicSquare -I../.. magicSquare.cpp ${TIME_UNIX_CPP} ${PATH_LINUX_CPP}


magicSquare6.o: magicSquare6.cpp
	g++ -g -c magicSquare6.cpp -I../..


magicSquare6Test: magicSquare6.o
	g++ -g -o magicSquare6Test -I../../ magicSquare6Test.cpp magicSquare6.o