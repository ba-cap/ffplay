#!/bin/bash

  g_current_dir=`pwd`
g_build_out_dir=$g_current_dir/android_build_out

#
# remake build out directory
rm    -rf $g_build_out_dir
mkdir -p  $g_build_out_dir

   g_base_toolchains=/tmp/android_ndk-bundle
g_toolchains_armeabi=
  g_toolchains_arm64=

    g_ndk_path=
     g_arm_abi=
g_android_code=
    g_lib_type=shared


function usage() {
    cat <<@EOF
    $0 usage:
       -N android NDK path, read environment 'ANDROID_NDK' for default
       -r remake the toolchains
       -C android sdk platform code, 21 for default
       -A android arm abi, armeabi-v7a arm64-v8a for default
       -h usage
@EOF
    exit 1
}

function check_toolchains() {
    ARM_ABI=$1
    toolchains_path=$g_base_toolchains/$1

    ARM_ARCH=
    if [ "$1" = "armeabi-v7a" ]; then
        ARM_ARCH=arm
        g_toolchains_armeabi=$g_base_toolchains/armeabi-v7a
    fi

    if [ "$1" = "arm64-v8a" ]; then
        ARM_ARCH=arm64
        g_toolchains_arm64=$g_base_toolchains/arm64-v8a
    fi

    if [[ -z "$1" ]]; then
        echo "arch must armeabi-v7a or arm64-v8a"
        exit 1
    fi

    if [ ! -d $toolchains_path/sysroot ]; then
        mkdir -p $toolchains_path

        echo
        echo "    ARCH: $ARM_ARCH"
        echo "PLATFORM: android-$g_android_code"
        echo " INSTALL: $toolchains_path"
        echo
        $g_ndk_path/build/tools/make-standalone-toolchain.sh \
            --arch=$ARM_ARCH \
            --install-dir=$toolchains_path \
            --platform=android-$g_android_code \
            --use-llvm \
            --force
    fi
}

function check_environment() {

    if [[ -z $g_ndk_path ]]; then
        g_ndk_path=$ANDROID_NDK
        if [[ -z $g_ndk_path ]]; then
            echo "ANDROID_NDK or -N not set"
            usage
        fi
    fi

    if [[ -z $g_arm_abi ]]; then
        g_arm_abi="armeabi-v7a arm64-v8a"
    fi

    if [[ -z $g_android_code ]]; then
        g_android_code=21
    fi

    echo "------------------------------------------------------------"
    echo "    ndk path: $g_ndk_path"
    echo "     arm abi: $g_arm_abi"
    echo "android code: $g_android_code"
    echo "------------------------------------------------------------"
    echo

    # check the toolchains
    for abi in $g_arm_abi; do
        check_toolchains $abi
    done
}

#
# get the build library type
# shared  libxxx.so
# static  libxxx.a
function getLibraryType() {
    ff_type=
    if [ "$g_lib_type" = "shared" ]; then
        ff_type="--disable-static --enable-shared "
    else
        ff_type="--disable-shared --enable-static "
    fi

    echo $ff_type
}

#
# set the c build flags
function c_flags() {
    build_c_flags="-fPIC -DANDROID -O3 -Wall -Wextra -ffast-math -pipe -std=c99"

    echo $build_c_flags
}



#
# some arm address: https://blog.csdn.net/heli200482128/article/details/79303286
# https://blog.csdn.net/SoaringLee_fighting/article/details/82800919
#
function ffmpeg_build() {
        build_abi=$1
       build_arch=
        build_cpu=
    build_out_dir=$g_build_out_dir/$build_abi
      build_cross=
         build_cc=
         build_nm=
     build_cflags=
    build_sysroot=

    if [ "$build_abi" = "armeabi-v7a" ]; then
           build_arch=arm
            build_cpu=armv7-a
          build_cross=$g_toolchains_armeabi/bin/arm-linux-androideabi-
             build_cc=$g_toolchains_armeabi/bin/arm-linux-androideabi-clang
             build_nm=$g_toolchains_armeabi/bin/arm-linux-androideabi-nm
         build_cflags="$(c_flags) -march=armv7-a -mcpu=cortex-a8 -mfpu=neon -mfloat-abi=softfp "
        build_sysroot=$g_toolchains_armeabi/sysroot

    elif [ "$build_abi" = "arm64-v8a" ]; then
           build_arch=arm64
            build_cpu=armv8-a
          build_cross=$g_toolchains_arm64/bin/aarch64-linux-android-
             build_cc=$g_toolchains_arm64/bin/aarch64-linux-android-clang
             build_nm=$g_toolchains_arm64/bin/aarch64-linux-android-nm
         build_cflags="$(c_flags) "
        build_sysroot=$g_toolchains_arm64/sysroot

    else
        echo "buid arm abi must one of armeabi-v7a arm64-v8a"
        return
    fi

    echo ""
    echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
    echo "    abi: $build_abi"
    echo "   arch: $build_arch"
    echo "    cpu: $build_cpu"
    echo "  cross: $build_cross"
    echo "     cc: $build_cc"
    echo "     nm: $build_nm"
    echo "sysroot: $build_sysroot"
    echo " cflags: $build_cflags"
    echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
    echo ""

    rm    -rf $build_out_dir
    mkdir -p  $build_out_dir

    $g_current_dir/configure \
        --disable-doc \
        --disable-htmlpages \
        --disable-manpages \
        --disable-podpages \
        --disable-txtpages \
        --disable-programs \
        --disable-ffmpeg \
        --disable-ffplay \
        --disable-ffprobe \
        --target-os=android \
        --enable-cross-compile \
        --prefix=$build_out_dir \
        --arch=$build_arch \
        --cpu=$build_cpu \
        --cross-prefix=$build_cross \
        --extra-cflags="$build_cflags" \
        --sysroot=$build_sysroot \
        --enable-gpl \
        $(getLibraryType) \
        $(custom_ffmpeg_config)

        make clean
        make -j4
        make install

        echo
        echo "finish build ABI of $build_abi"
        echo
}


#
# read the input paramter
while getopts "N:C:A:hr" opt;
do
    case $opt in
        N)
            g_ndk_path=$OPTARG
        ;;
        C)
            g_android_code=$OPTARG
        ;;
        A)
            g_arm_abi=$OPTARG
        ;;
        h)
            usage
        ;;
        r)
            rm -rf $g_base_toolchains
        ;;
    esac
done


#
# check the exec environment first
check_environment

source ./android_ffmepg_config.sh

echo "config: $(custom_ffmpeg_config)"

#
# build the ffmpeg
for abi in $g_arm_abi; do
    ffmpeg_build $abi
done





