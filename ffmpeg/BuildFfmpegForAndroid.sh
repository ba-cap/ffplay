#!/bin/bash

   CURRENT_DIR=`pwd`
BUILD_DEST_DIR=$CURRENT_DIR/android_out

export SRC_PATH=$CURRENT_DIR

# 重建目标编译路径
rm    -rf $BUILD_DEST_DIR
mkdir -p  $BUILD_DEST_DIR

# 使用 ndk r20 版本
NDK_DIR=/home/patrick/tools/google/sdk/ndk-bundle

# NDK toolchain 目标目录
      ndk_bundle_dir=/tmp/ndk-bundle/r20
  ndk_bundle_dir_arm=$ndk_bundle_dir/arm
ndk_bundle_dir_arm64=$ndk_bundle_dir/arm64

for DIR in arm arm64; do

    target_dir=
          arch=

    case $DIR in
        arm)
            target_dir=$ndk_bundle_dir_arm
                  arch=arm
        ;;
        arm64)
            target_dir=$ndk_bundle_dir_arm64
                  arch=arm64
        ;;
    esac

    if [ ! -d "$target_dir/sysroot" ]; then
        echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++++"
        echo " TARGET DIR: $target_dir"
        echo "       ARCH: $arch"
        echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++++"

        rm   -rf $target_dir
        mkdir -p $target_dir

        $NDK_DIR/build/tools/make-standalone-toolchain.sh \
            --arch=$arch \
            --install-dir=$target_dir \
            --platform=android-23 \
            --use-llvm \
            --force
         echo
    fi
done


#########################################################################
LIBTYPE=shared

#
# -Wall      代开所有警告
# -Werror    将所有的警告当成错误进行处理
#
#CFLAGS="-O2 -fPIC -DANDROID -Wall -Werror -Wno-unused -Wfatal-errors -Wno-deprecated -Wunused-command-line-argument"
#CFLAGS="-DANDROID -O2 -fPIC -Wall -Wextra -Werror -Wfatal-errors"
 CFLAGS="-DANDROID -O2 -fPIC -Wall -Wextra"


# get the build library type
# shared  libxxx.so
# static  libxxx.a
function getLibraryType() {
    ff_type=
    if [ "$LIBTYPE" = "shared" ]; then
        ff_type="--disable-static --enable-shared"
    else
        ff_type="--disable-shared --enable-static"
    fi

    echo $ff_type
}

function ff_config_custom2() {
  ffconfig="--enable-network \
            --enable-jni \
            --enable-mediacodec \
            --enable-decoder=h264_mediacodec \
            --enable-hwaccel=h264_mediacodec \
            --enable-asm \
            --enable-runtime-cpudetect \
            --enable-protocols  \
            --enable-parsers \
            --enable-bsfs \
            --enable-swscale  \
            --enable-demuxer=aac \
            --enable-demuxer=ac3 \
            --enable-demuxer=amr \
            --enable-demuxer=ape \
            --enable-demuxer=asf \
            --enable-demuxer=avs \
            --enable-demuxer=cavsvideo \
            --enable-demuxer=dts \
            --enable-demuxer=dvbsub \
            --enable-demuxer=dvbtxt \
            --enable-demuxer=eac3 \
            --enable-demuxer=flac \
            --enable-demuxer=flv \
            --enable-demuxer=g722 \
            --enable-demuxer=g729 \
            --enable-demuxer=h261 \
            --enable-demuxer=h263 \
            --enable-demuxer=h264 \
            --enable-demuxer=hevc \
            --enable-demuxer=hls \
            --enable-demuxer=m4v \
            --enable-demuxer=matroska \
            --enable-demuxer=mjpeg \
            --enable-demuxer=mjpeg_2000 \
            --enable-demuxer=mov \
            --enable-demuxer=mp3 \
            --enable-demuxer=mpegps \
            --enable-demuxer=mpegts \
            --enable-demuxer=mpegtsraw \
            --enable-demuxer=mpegvideo \
            --enable-demuxer=mpjpeg \
            --enable-demuxer=pcm_alaw \
            --enable-demuxer=pcm_f32be \
            --enable-demuxer=pcm_f32le \
            --enable-demuxer=pcm_f64be \
            --enable-demuxer=pcm_f64le \
            --enable-demuxer=pcm_mulaw \
            --enable-demuxer=pcm_s16be \
            --enable-demuxer=pcm_s16le \
            --enable-demuxer=pcm_s24be \
            --enable-demuxer=pcm_s24le \
            --enable-demuxer=pcm_s32be \
            --enable-demuxer=pcm_s32le \
            --enable-demuxer=pcm_s8 \
            --enable-demuxer=pcm_u16be \
            --enable-demuxer=pcm_u16le \
            --enable-demuxer=pcm_u24be \
            --enable-demuxer=pcm_u24le \
            --enable-demuxer=pcm_u32be \
            --enable-demuxer=pcm_u32le \
            --enable-demuxer=pcm_u8 \
            --enable-demuxer=rawvideo \
            --enable-demuxer=rm \
            --enable-demuxer=rtp \
            --enable-demuxer=rtsp \
            --enable-demuxer=vc1 \
            --enable-demuxer=vobsub \
            --enable-demuxer=wav \
            --enable-demuxer=webvtt \
            --enable-decoder=aac \
            --enable-decoder=aac_latm \
            --enable-decoder=ac3 \
            --enable-decoder=alac \
            --enable-decoder=amrnb \
            --enable-decoder=amrwb \
            --enable-decoder=ass \
            --enable-decoder=truehd \
            --enable-decoder=cavs \
            --enable-decoder=dnx \
            --enable-decoder=dnxhd \
            --enable-decoder=cook \
            --enable-decoder=dvbsub \
            --enable-decoder=dvdsub \
            --enable-decoder=eac3 \
            --enable-decoder=vc1 \
            --enable-decoder=flac \
            --enable-decoder=flv \
            --enable-decoder=g729 \
            --enable-decoder=h261 \
            --enable-decoder=h263 \
            --enable-decoder=h264 \
            --enable-decoder=hevc \
            --enable-decoder=vp8 \
            --enable-decoder=vp8_mediacodec \
            --enable-decoder=hevc_mediacodec \
            --enable-decoder=mjpeg \
            --enable-decoder=mp2 \
            --enable-decoder=mp3 \
            --enable-decoder=mpeg2video \
            --enable-decoder=mpeg4 \
            --enable-decoder=mpegvideo \
            --enable-decoder=vp9 \
            --enable-decoder=vp9_mediacodec \
            --enable-decoder=webvtt \
            --enable-decoder=wmalossless \
            --enable-decoder=wmapro \
            --enable-decoder=pgssub \
            --enable-decoder=rv10 \
            --enable-decoder=rv20 \
            --enable-decoder=rv30 \
            --enable-decoder=rv40 \
            --enable-decoder=yuv4"

    echo $ffconfig
}

function ff_config_custom3() {
  ffconfig="--enable-network \
            --enable-jni \
            --enable-mediacodec \
            --enable-decoder=h264_mediacodec \
            --enable-hwaccel=h264_mediacodec \
            --enable-runtime-cpudetect \
            --enable-asm \
            --enable-neon \
            --enable-small"

    echo $ffconfig
}

function ff_config_custom() {
  ffconfig="--enable-jni \
            --enable-mediacodec \
            --enable-decoder=h264_mediacodec \
            --enable-hwaccel=h264_mediacodec \
            --enable-runtime-cpudetect \
            --enable-asm \
            --enable-neon \
            --enable-small \
            --disable-postproc \
            --disable-avdevice \
            --disable-symver \
            --disable-stripping"

    echo $ffconfig
}


function ffmpeg_build() {
          ABI=$1
         TYPE=$2
      MY_ARCH=
          CPU=
    CROSS_PRE=
    BUILD_OUT=$BUILD_DEST_DIR/$ABI
        MY_CC=
        MY_NM=
     MY_CFLAG=
      SYSROOT=

    if [ "$ABI" = "armeabi-v7a" ]; then
         MY_ARCH=arm
             CPU=armv7-a
        CROSS_PRE=$ndk_bundle_dir_arm/bin/arm-linux-androideabi-
            MY_CC=$ndk_bundle_dir_arm/bin/arm-linux-androideabi-clang
            MY_NM=$ndk_bundle_dir_arm/bin/arm-linux-androideabi-nm
          SYSROOT=$ndk_bundle_dir_arm/sysroot
        #MY_CFLAG="-I$SYSROOT/usr/include $CFLAGS -mfpu=neon -mfloat-abi=softfp "
         MY_CFLAG="$CFLAGS -mfpu=neon -mfloat-abi=softfp "
    else
          MY_ARCH=arm64
              CPU=armv8-a
        CROSS_PRE=$ndk_bundle_dir_arm64/bin/aarch64-linux-android-
            MY_CC=$ndk_bundle_dir_arm64/bin/aarch64-linux-android-clang
            MY_NM=$ndk_bundle_dir_arm64/bin/aarch64-linux-android-nm
          SYSROOT=$ndk_bundle_dir_arm64/sysroot
        #MY_CFLAG="-I$SYSROOT/usr/include $CFLAGS "
         MY_CFLAG="$CFLAGS "
    fi

    # make the out directory
    rm    -rf $BUILD_OUT
    mkdir -p  $BUILD_OUT

    echo "-----------------------------------------------------------"
    echo "build android with follow info:"
    echo "     TYPE: $TYPE"
    echo "      ABI: $ABI"
    echo "  MY_ARCH: $MY_ARCH"
    echo "      CPU: $CPU"
    echo "CROSS_PRE: $CROSS_PRE"
    echo "    MY_CC: $MY_CC"
    echo "    MY_NM: $MY_NM"
    echo " MY_CFLAG: $MY_CFLAG"
    echo "BUILD_OUT: $BUILD_OUT"
    echo "  SYSROOT: $SYSROOT"
    echo "-----------------------------------------------------------"
    echo

    ## --disable-all replace --enable-small

    $CURRENT_DIR/configure \
        --target-os=android \
        --prefix=$BUILD_OUT \
        --arch=$MY_ARCH \
        --cpu=$CPU \
        --cross-prefix=$CROSS_PRE \
        --extra-cflags="$MY_CFLAG" \
        --sysroot=$SYSROOT \
        --enable-cross-compile \
        --disable-doc \
        --disable-htmlpages \
        --disable-manpages \
        --disable-podpages \
        --disable-txtpages \
        --disable-programs \
        --disable-ffmpeg \
        --disable-ffplay \
        --disable-ffprobe \
        $(getLibraryType) \
        $(ff_config_custom) \
        --enable-gpl

    make clean
    make -j4
    make install
}


#ABI_LIST="arm64-v8a arm64-v8a" 
ABI_LIST="armeabi-v7a"

for abi in $ABI_LIST; do

    ffmpeg_build $abi

    sleep 6

done
