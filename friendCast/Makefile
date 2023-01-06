
# Mac OS X settings:

#MPEG_LOCATION = /Volumes/MacOS/cpp/mpeg2dec-0.4.0

#FFMPEG_LOCATION = /Volumes/MacOS/ffmpeg

#ROOT_PATH = /Volumes/MacOS/cpp

#LINK_FLAGS = -framework SDL -framework Cocoa -I/System/Library/Frameworks/SDL.framework/Headers SDLMain.m ${FFMPEG_LOCATION}/libavformat/libavformat.a \
#${FFMPEG_LOCATION}/libavcodec/libavcodec.a ${FFMPEG_LOCATION}/libavutil/libavutil.a -lz ${ROOT_PATH}/minorGems/sound/SoundPlayer.o -framework CoreAudio -framework AudioToolbox ${ROOT_PATH}/minorGems/sound/portaudio/lib/libportaudio.a



# Linux settings:

#MPEG_LOCATION = /home/jcr13/mpeg2dec-0.4.0

FFMPEG_LOCATION = /data5/jcr13/downloads/ffmpeg

ROOT_PATH = /data4/jcr13/cpp

LINK_FLAGS = -lSDL -lpthread ${FFMPEG_LOCATION}/libavformat/libavformat.a \
${FFMPEG_LOCATION}/libavcodec/libavcodec.a ${FFMPEG_LOCATION}/libavutil/libavutil.a -lz ${ROOT_PATH}/minorGems/sound/SoundPlayer.o ${ROOT_PATH}/minorGems/sound/portaudio/lib/libportaudio.a


COMPILE_FLAGS = -g -I${ROOT_PATH}/minorGems/sound/portaudio/pa_common


FFMPEG_INCLUDE = -I${FFMPEG_LOCATION}/libavcodec -I${FFMPEG_LOCATION}/libavformat -I${FFMPEG_LOCATION}/libavutil


LIB_MPEG = #${MPEG_LOCATION}/libmpeg2/.libs/libmpeg2.a

LIB_MPEG_CONVERT = #${MPEG_LOCATION}/libmpeg2/convert/.libs/libmpeg2convert.a



all: playFromFile playFromConnection playFromDataSourceFF.o




FileSeriesDataSource.o:  FileSeriesDataSource.cpp DataSource.h FileSeriesDataSource.h
	g++ ${COMPILE_FLAGS} -c -o FileSeriesDataSource.o FileSeriesDataSource.cpp -I${MPEG_LOCATION}/include/ -I${ROOT_PATH}


NetworkDataSource.o:  NetworkDataSource.cpp DataSource.h NetworkDataSource.h
	g++ ${COMPILE_FLAGS} -c -o NetworkDataSource.o NetworkDataSource.cpp -I${MPEG_LOCATION}/include/ -I${ROOT_PATH}


playFromDataSource.o:  playFromDataSource.cpp DataSource.h playFromDataSource.h
	g++ ${COMPILE_FLAGS} -c -o playFromDataSource.o playFromDataSource.cpp -I${MPEG_LOCATION}/include/ -I${ROOT_PATH}


playFromDataSourceFF.o:  playFromDataSourceFF.cpp DataSource.h playFromDataSource.h
	g++ ${COMPILE_FLAGS} -c -o playFromDataSourceFF.o playFromDataSourceFF.cpp ${FFMPEG_INCLUDE} -I${ROOT_PATH}


playVideoFileFF.o:  playVideoFileFF.cpp playVideoFile.h
	g++ ${COMPILE_FLAGS} -c -o playVideoFileFF.o playVideoFileFF.cpp ${FFMPEG_INCLUDE} -I${ROOT_PATH}


streamNetwork.o:  streamNetwork.cpp streamNetwork.h SentFileTracker.h
	g++ ${COMPILE_FLAGS} -c -o streamNetwork.o streamNetwork.cpp ${FFMPEG_INCLUDE} -I${ROOT_PATH}

streamFileTracker.o:  streamFileTracker.cpp streamFileTracker.h
	g++ ${COMPILE_FLAGS} -c -o streamFileTracker.o streamFileTracker.cpp ${FFMPEG_INCLUDE} -I${ROOT_PATH}


SentFileTracker.o:  SentFileTracker.cpp SentFileTracker.h
	g++ ${COMPILE_FLAGS} -c -o SentFileTracker.o SentFileTracker.cpp ${FFMPEG_INCLUDE} -I${ROOT_PATH}


playFromFile.o:  playFromFile.cpp playVideoFile.h streamFileTracker.h streamNetwork.h
	g++ ${COMPILE_FLAGS} -c -o playFromFile.o playFromFile.cpp -I${MPEG_LOCATION}/include/ -I${ROOT_PATH}


playFromConnection.o:  playFromConnection.cpp playVideoFile.h streamFileTracker.h streamNetwork.h
	g++ ${COMPILE_FLAGS} -c -o playFromConnection.o playFromConnection.cpp -I${MPEG_LOCATION}/include/ -I${ROOT_PATH}



playFromFile:  playFromFile.o  playVideoFileFF.o streamNetwork.o streamFileTracker.o SentFileTracker.o
	g++ ${COMPILE_FLAGS} -o playFromFile playFromFile.o \
playVideoFileFF.o streamNetwork.o streamFileTracker.o SentFileTracker.o \
${ROOT_PATH}/minorGems/util/stringUtils.o \
${ROOT_PATH}/minorGems/util/StringBufferOutputStream.o \
${ROOT_PATH}/minorGems/graphics/linux/ScreenGraphicsLinux.o \
${ROOT_PATH}/minorGems/ui/linux/KeyboardLinux.o \
${ROOT_PATH}/minorGems/network/linux/SocketLinux.o \
${ROOT_PATH}/minorGems/network/linux/SocketServerLinux.o \
${ROOT_PATH}/minorGems/network/NetworkFunctionLocks.o \
${ROOT_PATH}/minorGems/system/linux/MutexLockLinux.o \
${ROOT_PATH}/minorGems/system/linux/ThreadLinux.o \
${ROOT_PATH}/minorGems/system/unix/TimeUnix.o \
${LIB_MPEG} \
${LIB_MPEG_CONVERT} \
${LINK_FLAGS}




playFromConnection:  playFromConnection.o playVideoFileFF.o streamNetwork.o streamFileTracker.o SentFileTracker.o
	g++ ${COMPILE_FLAGS} -o playFromConnection playFromConnection.o \
playVideoFileFF.o streamNetwork.o streamFileTracker.o SentFileTracker.o\
${ROOT_PATH}/minorGems/util/stringUtils.o \
${ROOT_PATH}/minorGems/util/StringBufferOutputStream.o \
${ROOT_PATH}/minorGems/graphics/linux/ScreenGraphicsLinux.o \
${ROOT_PATH}/minorGems/ui/linux/KeyboardLinux.o \
${ROOT_PATH}/minorGems/network/linux/SocketLinux.o \
${ROOT_PATH}/minorGems/network/linux/SocketServerLinux.o \
${ROOT_PATH}/minorGems/network/linux/SocketClientLinux.o \
${ROOT_PATH}/minorGems/network/NetworkFunctionLocks.o \
${ROOT_PATH}/minorGems/system/linux/MutexLockLinux.o \
${ROOT_PATH}/minorGems/system/linux/ThreadLinux.o \
${ROOT_PATH}/minorGems/system/unix/TimeUnix.o \
${LIB_MPEG} \
${LIB_MPEG_CONVERT} \
${LINK_FLAGS}
