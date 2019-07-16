cmake_minimum_required(VERSION 3.14)
project(IIW C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lMagickWand-6.Q16 -lMagickCore-6.Q16 -DMAGICKCORE_HDRI_ENABLE=0 -DMAGICKCORE_QUANTUM_DEPTH=16")
set(ImageMagick_FOUND ${IMAGEMAGICK_FOUND})
include_directories("/usr/include/ImageMagick-6/")
include_directories("/usr/include/x86_64-linux-gnu/ImageMagick-6/")
add_executable(IIW
        main.c
        utils.c
        byteFlowHanlder.c
        errorHandler.c
        httpHandler.c
        const.h

        cache/cache.c
        cache/cache.h
        cache/image_mdf.c
        cache/image_mdf.h
        log/log.c
        log/log.h
        log/get_time.h
        log/get_time.c
        deviceDetection/51DegreesWorker.c
        deviceDetection/51DegreesWorker.h
        Device-Detection-3.2.20.4/src/pattern/51Degrees.h
        Device-Detection-3.2.20.4/src/pattern/51Degrees.c
        Device-Detection-3.2.20.4/src/threading.h
        Device-Detection-3.2.20.4/src/threading.c
        Device-Detection-3.2.20.4/src/cityhash/city.c
        Device-Detection-3.2.20.4/src/cityhash/city.h deviceDetection/mbInfoStruct.h)




FIND_PACKAGE(ImageMagick
        REQUIRED
        COMPONENTS MagickWand
        )
find_package (Threads)


INCLUDE_DIRECTORIES(
        ${Boost_INCLUDE_DIR}
        ${ImageMagick_INCLUDE_DIRS}
        ${ImageMagick_MagickWand_INCLUDE_DIRS}
)

TARGET_LINK_LIBRARIES(IIW
        ${Boost_LIBRARIES}
        ${CURL_LIBRARIES}
        ${ImageMagick_LIBRARIES}
        ${ImageMagick_MagickWand_LIBRARY}
        ${CMAKE_THREAD_LIBS_INIT}
        m)