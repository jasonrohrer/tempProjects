


TIME_UNIX_CPP = ../../minorGems/system/unix/TimeUnix.cpp

PATH_LINUX_CPP = ../../minorGems/io/file/linux/PathLinux.cpp

PATH_MIN_MAX_CPP = ../../minorGems/ai/gameTree/minMax.cpp



all: magicSquare magicSquare6Test magicSquareAITest greedyPeekingText



magicSquare: magicSquare.cpp ${TIME_UNIX_CPP} ${PATH_LINUX_CPP} magicSquare6.o
	g++ -g -o magicSquare -I../.. magicSquare.cpp ${TIME_UNIX_CPP} ${PATH_LINUX_CPP}


magicSquare6.o: magicSquare6.cpp
	g++ -g -c magicSquare6.cpp -I../..


magicSquare6Test: magicSquare6.o magicSquare6Test.cpp
	g++ -g -o magicSquare6Test -I../../ magicSquare6Test.cpp magicSquare6.o


MagicSquareGameState.o: MagicSquareGameState.cpp
	g++ -g -c MagicSquareGameState.cpp -I../..

MagicSquareGameStateTwoPick.o: MagicSquareGameStateTwoPick.cpp
	g++ -g -c MagicSquareGameStateTwoPick.cpp -I../..


magicSquareAITest: magicSquare6.o MagicSquareGameState.o magicSquareAITest.cpp ${PATH_MIN_MAX_CPP}
	g++ -Wall -g -o magicSquareAITest -I../../ magicSquareAITest.cpp magicSquare6.o MagicSquareGameState.o ${PATH_MIN_MAX_CPP}



greedyPeekingText: magicSquare6.o MagicSquareGameStateTwoPick.o greedyPeekingTest.cpp ${PATH_MIN_MAX_CPP}
	g++ -Wall -g -o greedyPeekingTest -I../../ greedyPeekingTest.cpp magicSquare6.o MagicSquareGameStateTwoPick.o ${PATH_MIN_MAX_CPP}
