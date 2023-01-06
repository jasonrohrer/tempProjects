#
# Modification History
#
# 2002-May-2    Jason Rohrer
# Created.
#
# 2002-May-5    Jason Rohrer
# Added support for WebClient compilation.
#


GXX=g++ 
# OSX uses c++
#GXX=c++ 

ROOT_PATH = ../../..

DEBUG_ON_FLAG = -g
DEBUG_OFF_FLAG = 

DEBUG_FLAG = ${DEBUG_ON_FLAG}


PROFILE_ON_FLAG = -pg -a -DUSE_GPROF_THREADS
PROFILE_OFF_FLAG = 

PROFILE_FLAG = ${PROFILE_OFF_FLAG}



ELECTRIC_FENCE_ON_FLAG = -L../util/electricFence -lefence 
ELECTRIC_FENCE_OFF_FLAG =

ELECTRIC_FENCE_FLAG = ${ELECTRIC_FENCE_OFF_FLAG}


# Controls whether Fortify malloc debugger is used
FORTIFY_COMPILE_ON_FLAG = -DFORTIFY
FORTIFY_COMPILE_OFF_FLAG =

FORTIFY_COMPILE_FLAG = ${FORTIFY_COMPILE_OFF_FLAG}

FORTIFY_LINK_ON_FLAG = ${ROOT_PATH}/minorGems/util/development/fortify/fortify.o
FORTIFY_LINK_OFF_FLAG =

FORTIFY_LINK_FLAG = ${FORTIFY_LINK_OFF_FLAG}





# pthread library needed for linux
PLATFORM_FLAGS = -DLINUX -lpthread
# pthread library linked differently for BSD
# PLATFORM_FLAGS = -DBSD -pthread
# pthread flag not needed for OSX
#PLATFORM_FLAGS = -DBSD
# all sorts of other libraries needed for Solaris
#PLATFORM_FLAGS = -DSOLARIS -lsocket -lnsl -lresolv


COMPILE_FLAGS = ${DEBUG_FLAG} ${PLATFORM_FLAGS} ${PROFILE_FLAG} ${FORTIFY_COMPILE_FLAG}




PLATFORM = Linux
PLATFORM_PATH = linux

TIME_PLATFORM = Unix
TIME_PLATFORM_PATH = unix


SIMPLE_VECTOR_H = ${ROOT_PATH}/minorGems/util/SimpleVector.h

OUTPUT_STREAM_H = ${ROOT_PATH}/minorGems/io/OutputStream.h
INPUT_STREAM_H = ${ROOT_PATH}/minorGems/io/InputStream.h


HOST_ADDRESS_H = ${ROOT_PATH}/minorGems/network/HostAddress.h
HOST_ADDRESS_CPP = ${PLATFORM_HOST_ADDRESS}.cpp
HOST_ADDRESS_O = ${PLATFORM_HOST_ADDRESS}.o

SOCKET_H = ${ROOT_PATH}/minorGems/network/Socket.h
SOCKET_CPP = ${PLATFORM_SOCKET}.cpp
SOCKET_O = ${PLATFORM_SOCKET}.o

SOCKET_CLIENT_H = ${ROOT_PATH}/minorGems/network/SocketClient.h
SOCKET_CLIENT_CPP = ${PLATFORM_SOCKET_CLIENT}.cpp
SOCKET_CLIENT_O = ${PLATFORM_SOCKET_CLIENT}.o

SOCKET_SERVER_H = ${ROOT_PATH}/minorGems/network/SocketServer.h
SOCKET_SERVER_CPP = ${PLATFORM_SOCKET_SERVER}.cpp
SOCKET_SERVER_O = ${PLATFORM_SOCKET_SERVER}.o

SOCKET_STREAM_H = ${ROOT_PATH}/minorGems/network/SocketStream.h

PATH_H = ${ROOT_PATH}/minorGems/io/file/Path.h
PATH_CPP = ${ROOT_PATH}/minorGems/io/file/${PLATFORM_PATH}/Path${PLATFORM}.cpp
PATH_O = ${ROOT_PATH}/minorGems/io/file/${PLATFORM_PATH}/Path${PLATFORM}.o


FILE_H = ${ROOT_PATH}/minorGems/io/file/File.h
FILE_OUTPUT_STREAM_H = ${ROOT_PATH}/minorGems/io/file/FileOutputStream.h
FILE_INPUT_STREAM_H = ${ROOT_PATH}/minorGems/io/file/FileInputStream.h


TYPE_IO_H = ${ROOT_PATH}/minorGems/io/TypeIO.h
TYPE_IO_CPP = ${PLATFORM_TYPE_IO}.cpp
TYPE_IO_O = ${PLATFORM_TYPE_IO}.o


TIME_H = ${ROOT_PATH}/minorGems/system/Time.h
TIME_CPP = ${PLATFORM_TIME}.cpp
TIME_O = ${PLATFORM_TIME}.o

THREAD_H = ${ROOT_PATH}/minorGems/system/Thread.h
THREAD_CPP = ${PLATFORM_THREAD}.cpp
THREAD_O = ${PLATFORM_THREAD}.o

MUTEX_LOCK_H = ${ROOT_PATH}/minorGems/system/MutexLock.h
MUTEX_LOCK_CPP = ${PLATFORM_MUTEX_LOCK}.cpp
MUTEX_LOCK_O = ${PLATFORM_MUTEX_LOCK}.o


BINARY_SEMAPHORE_H = ${ROOT_PATH}/minorGems/system/BinarySemaphore.h
BINARY_SEMAPHORE_CPP = ${PLATFORM_BINARY_SEMAPHORE}.cpp
BINARY_SEMAPHORE_O = ${PLATFORM_BINARY_SEMAPHORE}.o


SEMAPHORE_H = ${ROOT_PATH}/minorGems/system/Semaphore.h

APP_LOG_H = ${ROOT_PATH}/minorGems/util/log/AppLog.h
APP_LOG_CPP = ${ROOT_PATH}/minorGems/util/log/AppLog.cpp
APP_LOG_O = ${ROOT_PATH}/minorGems/util/log/AppLog.o

PRINT_LOG_H = ${ROOT_PATH}/minorGems/util/log/PrintLog.h
PRINT_LOG_CPP = ${ROOT_PATH}/minorGems/util/log/PrintLog.cpp
PRINT_LOG_O = ${ROOT_PATH}/minorGems/util/log/PrintLog.o


FILE_LOG_H = ${ROOT_PATH}/minorGems/util/log/FileLog.h
FILE_LOG_CPP = ${ROOT_PATH}/minorGems/util/log/FileLog.cpp
FILE_LOG_O = ${ROOT_PATH}/minorGems/util/log/FileLog.o


LOG_H = ${ROOT_PATH}/minorGems/util/log/Log.h
LOG_CPP = ${ROOT_PATH}/minorGems/util/log/Log.cpp
LOG_O = ${ROOT_PATH}/minorGems/util/log/Log.o


PRINT_UTILS_H = ${ROOT_PATH}/minorGems/util/printUtils.h
PRINT_UTILS_CPP = ${ROOT_PATH}/minorGems/util/printUtils.cpp
PRINT_UTILS_O = ${ROOT_PATH}/minorGems/util/printUtils.o




##
# Platform-specific file path prefixes
##

PLATFORM_TIME = ${ROOT_PATH}/minorGems/system/${TIME_PLATFORM_PATH}/Time${TIME_PLATFORM}

PLATFORM_HOST_ADDRESS = ${ROOT_PATH}/minorGems/network/${PLATFORM_PATH}/HostAddress${PLATFORM}

PLATFORM_SOCKET = ${ROOT_PATH}/minorGems/network/${PLATFORM_PATH}/Socket${PLATFORM}

PLATFORM_SOCKET_CLIENT = ${ROOT_PATH}/minorGems/network/${PLATFORM_PATH}/SocketClient${PLATFORM}

PLATFORM_SOCKET_SERVER = ${ROOT_PATH}/minorGems/network/${PLATFORM_PATH}/SocketServer${PLATFORM}


PLATFORM_TYPE_IO = ${ROOT_PATH}/minorGems/io/${PLATFORM_PATH}/TypeIO${PLATFORM}

PLATFORM_THREAD = ${ROOT_PATH}/minorGems/system/${PLATFORM_PATH}/Thread${PLATFORM}

PLATFORM_MUTEX_LOCK = ${ROOT_PATH}/minorGems/system/${PLATFORM_PATH}/MutexLock${PLATFORM}

PLATFORM_BINARY_SEMAPHORE = ${ROOT_PATH}/minorGems/system/${PLATFORM_PATH}/BinarySemaphore${PLATFORM}



##
# Targets
##

all:  WebServer.o RequestHandlingThread.o ThreadHandlingThread.o ThoughtLinksPageGenerator.o thoughtLinks.o ${HOST_ADDRESS_O} ${SOCKET_O} ${SOCKET_SERVER_O} ${SOCKET_CLIENT_O} ${PATH_O} ${TYPE_IO_O} ${TIME_O} ${THREAD_O} ${MUTEX_LOCK_O} ${BINARY_SEMAPHORE_O} ${APP_LOG_O} ${PRINT_LOG_O} ${FILE_LOG_O} ${LOG_O} ${PRINT_UTILS_O} thoughtLinks testWebClient


${FORTIFY_LINK_ON_FLAG}: ${ROOT_PATH}/minorGems/util/development/fortify/fortify.h ${ROOT_PATH}/minorGems/util/development/fortify/ufortify.h ${ROOT_PATH}/minorGems/util/development/fortify/fortify.cpp
	cd ${ROOT_PATH}/minorGems/util/development/fortify/; make fortify.o


thoughtLinks: WebClient.o WebServer.o RequestHandlingThread.o ThreadHandlingThread.o StopSignalThread.o ThoughtLinksPageGenerator.o thoughtLinks.o ${HOST_ADDRESS_O} ${SOCKET_O} ${SOCKET_SERVER_O} ${SOCKET_CLIENT_O} ${PATH_O} ${TYPE_IO_O} ${TIME_O} ${THREAD_O} ${MUTEX_LOCK_O} ${BINARY_SEMAPHORE_O} ${APP_LOG_O} ${PRINT_LOG_O} ${FILE_LOG_O} ${LOG_O} ${PRINT_UTILS_O} ${FORTIFY_LINK_FLAG}
	${GXX} ${COMPILE_FLAGS} -o thoughtLinks WebClient.o WebServer.o RequestHandlingThread.o ThreadHandlingThread.o StopSignalThread.o ThoughtLinksPageGenerator.o thoughtLinks.o ${HOST_ADDRESS_O} ${SOCKET_O} ${SOCKET_SERVER_O} ${SOCKET_CLIENT_O} ${PATH_O} ${TYPE_IO_O} ${TIME_O} ${THREAD_O} ${MUTEX_LOCK_O} ${BINARY_SEMAPHORE_O} ${APP_LOG_O} ${PRINT_LOG_O} ${FILE_LOG_O} ${LOG_O} ${PRINT_UTILS_O} ${ELECTRIC_FENCE_FLAG} ${FORTIFY_LINK_FLAG}


testWebClient: testWebClient.o WebClient.o ${HOST_ADDRESS_O} ${SOCKET_O} ${SOCKET_CLIENT_O} ${TYPE_IO_O} ${APP_LOG_O} ${PRINT_LOG_O} ${FILE_LOG_O} ${LOG_O} ${MUTEX_LOCK_O} ${TIME_O} ${PRINT_UTILS_O} ${FORTIFY_LINK_FLAG}
	${GXX} ${COMPILE_FLAGS} -o testWebClient testWebClient.o WebClient.o ${HOST_ADDRESS_O} ${SOCKET_O} ${SOCKET_CLIENT_O} ${TYPE_IO_O} ${APP_LOG_O} ${PRINT_LOG_O} ${FILE_LOG_O} ${LOG_O} ${MUTEX_LOCK_O} ${TIME_O} ${PRINT_UTILS_O} ${FORTIFY_LINK_FLAG}


clean:
	rm -rf *.o thoughtLinks



# Counts the number of lines, then exits
linecount:
	cat *.cpp *.h | wc --lines




##
# Objects
##


WebClient.o: WebClient.h WebClient.cpp
	 ${GXX} ${COMPILE_FLAGS} -I${ROOT_PATH} -o WebClient.o -c WebClient.cpp


WebServer.o: ThreadHandlingThread.h RequestHandlingThread.h WebServer.h WebServer.cpp
	 ${GXX} ${COMPILE_FLAGS} -I${ROOT_PATH} -o WebServer.o -c WebServer.cpp


RequestHandlingThread.o: RequestHandlingThread.h RequestHandlingThread.cpp
	 ${GXX} ${COMPILE_FLAGS} -I${ROOT_PATH} -o RequestHandlingThread.o -c RequestHandlingThread.cpp


ThreadHandlingThread.o: ThreadHandlingThread.h ThreadHandlingThread.cpp
	 ${GXX} ${COMPILE_FLAGS} -I${ROOT_PATH} -o ThreadHandlingThread.o -c ThreadHandlingThread.cpp


StopSignalThread.o: StopSignalThread.h StopSignalThread.cpp
	 ${GXX} ${COMPILE_FLAGS} -I${ROOT_PATH} -o StopSignalThread.o -c StopSignalThread.cpp


ThoughtLinksPageGenerator.o: ThoughtLinksPageGenerator.h ThoughtLinksPageGenerator.cpp ${PATH_H} ${FILE_H} ${FILE_INPUT_STREAM_H}
	 ${GXX} ${COMPILE_FLAGS} -I${ROOT_PATH} -o ThoughtLinksPageGenerator.o -c ThoughtLinksPageGenerator.cpp



thoughtLinks.o: ThreadHandlingThread.h RequestHandlingThread.h WebServer.h ThoughtLinksPageGenerator.h thoughtLinks.cpp
	 ${GXX} ${COMPILE_FLAGS} -I${ROOT_PATH} -o thoughtLinks.o -c thoughtLinks.cpp


testWebClient.o: WebClient.h testWebClient.cpp
	 ${GXX} ${COMPILE_FLAGS} -I${ROOT_PATH} -o testWebClient.o -c testWebClient.cpp







${HOST_ADDRESS_O}: ${HOST_ADDRESS_H} ${HOST_ADDRESS_CPP}
	${GXX} ${COMPILE_FLAGS} -I${ROOT_PATH} -o ${HOST_ADDRESS_O} -c ${HOST_ADDRESS_CPP}


${SOCKET_O}: ${SOCKET_H} ${SOCKET_CPP}
	${GXX} ${COMPILE_FLAGS} -I${ROOT_PATH} -o ${SOCKET_O} -c ${SOCKET_CPP}


${SOCKET_SERVER_O}: ${SOCKET_SERVER_H} ${SOCKET_SERVER_CPP}
	${GXX} ${COMPILE_FLAGS} -I${ROOT_PATH} -o ${SOCKET_SERVER_O} -c ${SOCKET_SERVER_CPP}


${SOCKET_CLIENT_O}: ${SOCKET_CLIENT_H} ${SOCKET_CLIENT_CPP}
	${GXX} ${COMPILE_FLAGS} -I${ROOT_PATH} -o ${SOCKET_CLIENT_O} -c ${SOCKET_CLIENT_CPP}


${PATH_O}: ${PATH_H} ${PATH_CPP}
	${GXX} ${COMPILE_FLAGS} -I${ROOT_PATH} -o ${PATH_O} -c ${PATH_CPP}


${TYPE_IO_O}: ${TYPE_IO_H} ${TYPE_IO_CPP}
	${GXX} ${COMPILE_FLAGS} -I${ROOT_PATH} -o ${TYPE_IO_O} -c ${TYPE_IO_CPP}


${TIME_O}: ${TIME_H} ${TIME_CPP}
	${GXX} ${COMPILE_FLAGS} -I${ROOT_PATH} -o ${TIME_O} -c ${TIME_CPP}


${THREAD_O}: ${THREAD_H} ${THREAD_CPP}
	${GXX} ${COMPILE_FLAGS} -I${ROOT_PATH} -o ${THREAD_O} -c ${THREAD_CPP}


${MUTEX_LOCK_O}: ${MUTEX_LOCK_H} ${MUTEX_LOCK_CPP}
	${GXX} ${COMPILE_FLAGS} -I${ROOT_PATH} -o ${MUTEX_LOCK_O} -c ${MUTEX_LOCK_CPP}


${BINARY_SEMAPHORE_O}: ${BINARY_SEMAPHORE_H} ${BINARY_SEMAPHORE_CPP}
	${GXX} ${COMPILE_FLAGS} -I${ROOT_PATH} -o ${BINARY_SEMAPHORE_O} -c ${BINARY_SEMAPHORE_CPP}


${APP_LOG_O}: ${APP_LOG_H} ${APP_LOG_CPP}
	${GXX} ${COMPILE_FLAGS} -I${ROOT_PATH} -o ${APP_LOG_O} -c ${APP_LOG_CPP}


${PRINT_LOG_O}: ${PRINT_LOG_H} ${PRINT_LOG_CPP}
	${GXX} ${COMPILE_FLAGS} -I${ROOT_PATH} -o ${PRINT_LOG_O} -c ${PRINT_LOG_CPP}

${FILE_LOG_O}: ${FILE_LOG_H} ${FILE_LOG_CPP}
	${GXX} ${COMPILE_FLAGS} -I${ROOT_PATH} -o ${FILE_LOG_O} -c ${FILE_LOG_CPP}


${LOG_O}: ${LOG_H} ${LOG_CPP}
	${GXX} ${COMPILE_FLAGS} -I${ROOT_PATH} -o ${LOG_O} -c ${LOG_CPP}


${PRINT_UTILS_O}: ${PRINT_UTILS_H} ${PRINT_UTILS_CPP}
	${GXX} ${COMPILE_FLAGS} -I${ROOT_PATH} -o ${PRINT_UTILS_O} -c ${PRINT_UTILS_CPP}


