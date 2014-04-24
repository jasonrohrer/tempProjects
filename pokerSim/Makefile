

all: pokerSim


INCLUDE_PATH = ../..

CRYPTO_RANDOM_CPP = ${INCLUDE_PATH}/minorGems/crypto/cryptoRandom.cpp

pokerSim: pokerSim.cpp ${CRYPTO_RANDOM_CPP} Makefile
	g++ -O0 -I${INCLUDE_PATH} -g -Wall -o pokerSim pokerSim.cpp ${CRYPTO_RANDOM_CPP}