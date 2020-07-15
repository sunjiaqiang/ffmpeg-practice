cmake_minimum_required(VERSION 3.16)
project(ffmpgeDemo C)

set(CMAKE_C_STANDARD 11)
set(FFMPEG_DIR /usr/local/Cellar/ffmpeg/4.3_1) # FFmpeg的安装目录，可以通过命令"brew info ffmpeg"获取
include_directories(${FFMPEG_DIR}/include/) # 头文件搜索路径
link_directories(${FFMPEG_DIR}/lib/)

add_executable(ffmpgeDemo src/getFrame/main.c)

target_link_libraries(
        ffmpgeDemo
        avcodec
        avdevice
        avfilter
        avformat
        avresample
        avutil
        postproc
        swresample
        swscale
)