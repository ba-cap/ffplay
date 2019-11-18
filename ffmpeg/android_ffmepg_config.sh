#!/bin/bash

function custom_ff_v3_4_6_small() {
  ffconfig="--disable-postproc \
            --disable-avdevice \
            --disable-symver \
            --disable-stripping \
            --disable-sdl2 \
            --enable-small \
            --enable-asm \
            --enable-neon \
            --enable-thumb \
            --enable-runtime-cpudetect \
            --enable-jni \
            --enable-mediacodec \
            --enable-decoder=h264_mediacodec \
            --enable-hwaccel=h264_mediacodec \
            --enable-decoder=hevc_mediacodec \
            --enable-hwaccel=hevc_mediacodec \
            --enable-decoder=mpeg2_mediacodec \
            --enable-hwaccel=mpeg2_mediacodec \
            --enable-decoder=mpeg4_mediacodec \
            --enable-hwaccel=mpeg4_mediacodec \
            --enable-decoder=vp8_mediacodec \
            --enable-hwaccel=vp8_mediacodec \
            --enable-decoder=vp9_mediacodec \
            --enable-hwaccel=vp9_mediacodec"

    echo $ffconfig
}

function custom_ff_v4_2_1_small() {
  ffconfig="--disable-postproc \
            --disable-avdevice \
            --disable-symver \
            --disable-stripping \
            --disable-sdl2 \
            --enable-small \
            --enable-asm \
            --enable-neon \
            --enable-thumb \
            --enable-runtime-cpudetect \
            --enable-jni \
            --enable-mediacodec \
            --enable-decoder=h264_mediacodec \
            --enable-decoder=hevc_mediacodec \
            --enable-decoder=mpeg2_mediacodec \
            --enable-decoder=mpeg4_mediacodec \
            --enable-decoder=vp8_mediacodec \
            --enable-decoder=vp9_mediacodec"

    echo $ffconfig
}




function custom_ffmpeg_config() {
    #echo "$(custom_ff_v4_2_1_small)"
    echo "$(custom_ff_v3_4_6_small)"
}