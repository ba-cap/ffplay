
# How to build ffmepg for android

## get ffmepg data source

```
mkdir -p ffmpeg
cd ffmpeg
wget -c http://ffmpeg.org/releases/ffmpeg-3.4.6.tar.xz
tar Jxvf ffmpeg-3.4.6.tar.xz
cd ffmpeg-3.4.6

# wget -c https://ffmpeg.org/releases/ffmpeg-4.2.1.tar.bz2
# tar jxvf ffmpeg-4.2.1.tar.bz2
# cd ffmpeg-4.2.1

//
// 创建相关的软链接过去
// ln -s YourScriptDir/android_ffmepg_build.sh
// ln -s YourScriptDir/android_ffmepg_config.sh
```

## config your build env

+ modify the BuildFfmpegForAndroid.sh set you NDK path
+ modify the function in ff_config_custom config your ffmpeg to build
+ copy the build shared file and c head file to project directory


## how to run

1. copy the directory 'sources/ffmpeg-test' to your phone disk.
2. modify the code of 'dai_android_media_ffplay_MainActivity.cpp' which source to use.

## problem

+ not support --enable-hwaccel=h264_mediacodec

  we can use follow command show all --enable-hwaccel
  ```shell
  ffmpeg-4.2.1$ ./configure --list-hwaccels

  // command result:
    h263_vaapi              hevc_d3d11va            mpeg1_nvdec             mpeg2_vdpau             vc1_dxva2               vp9_nvdec
    h263_videotoolbox       hevc_d3d11va2           mpeg1_vdpau             mpeg2_videotoolbox      vc1_nvdec               vp9_vaapi
    h264_d3d11va            hevc_dxva2              mpeg1_videotoolbox      mpeg2_xvmc              vc1_vaapi               wmv3_d3d11va
    h264_d3d11va2           hevc_nvdec              mpeg1_xvmc              mpeg4_nvdec             vc1_vdpau               wmv3_d3d11va2
    h264_dxva2              hevc_vaapi              mpeg2_d3d11va           mpeg4_vaapi             vp8_nvdec               wmv3_dxva2
    h264_nvdec              hevc_vdpau              mpeg2_d3d11va2          mpeg4_vdpau             vp8_vaapi               wmv3_nvdec
    h264_vaapi              hevc_videotoolbox       mpeg2_dxva2             mpeg4_videotoolbox      vp9_d3d11va             wmv3_vaapi
    h264_vdpau              mjpeg_nvdec             mpeg2_nvdec             vc1_d3d11va             vp9_d3d11va2            wmv3_vdpau
    h264_videotoolbox       mjpeg_vaapi             mpeg2_vaapi             vc1_d3d11va2            vp9_dxva2
  ```

  some address see: https://trac.ffmpeg.org/wiki/HWAccelIntro

  but this support --enable-hwaccel=h264_mediacodec in ffmpeg release version ffmpeg-3.4.6
  ```
  patrick@opensuse:ffmpeg-3.4.6$ ./configure --list-hwaccels | grep media
  h264_mediacodec		mpeg2_d3d11va		vc1_vaapi
  h264_vaapi		mpeg2_mediacodec	vp8_mediacodec
  hevc_d3d11va		mpeg2_xvmc		vp9_mediacodec
  hevc_dxva2		mpeg4_mediacodec	wmv3_d3d11va
  hevc_mediacodec		mpeg4_mmal		wmv3_d3d11va2
  ```

+ support --enable-decoder=h264_mediacodec

  use follow command
  ```shell
  patrick@opensuse:ffmpeg-4.2.1$ ./configure --list-decoders | grep media
  // command result
    ac3                     h264_mediacodec         pcx
    adpcm_dtk               hevc_mediacodec         prores
    cljr                    mpeg2_mediacodec        vc1
    dds                     mpeg4_mediacodec        vmdaudio
    dst                     mss2                    vp8_mediacodec
    dxtory                  mwsc                    vp9_mediacodec
  ```

+ how to fix `__wrap_swr_convert` dlopen problem

when build ffmpeg configure set the 
```
  --enable-xmm-clobber-test 
  --enable-neon-clobber-test
```
  do not enable that test, that only support Mircosoft windows


+ NDK version VS ffmpeg version

 | ffmpeg version |      NDK version       |           备注                  |
 |:---------------|:-----------------------|:--------------------------------|
 | 4.2.1          |    含 r14 以及以后      | 不支持 android MediaCodec 硬解码 |
 | 3.4.6          | r14 和 r16 r20 编译报错 | 支持 android MediaCodec 硬解码   |
 
所以如果需要支持 android 硬解码需要使用 ffmpeg 的 3.4.6 版本进行编译。
