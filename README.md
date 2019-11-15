
# How to build ffmepg for android

## get ffmepg data source

```
mkdir -p ffmpeg
cd ffmpeg
# wget -c http://ffmpeg.org/releases/ffmpeg-3.4.6.tar.xz
wget -c https://ffmpeg.org/releases/ffmpeg-4.2.1.tar.bz2
tar jxvf ffmpeg-4.2.1.tar.bz2
# tar Jxvf ffmpeg-3.4.6.tar.xz
cd ffmpeg-3.4.6

// copy the BuildFfmpegForAndroid.sh into ffmpeg-4.2.1
```

## config your build env

+ modify the BuildFfmpegForAndroid.sh set you NDK path
+ modify the function in ff_config_custom config your ffmpeg to build
+ copy the build shared file and c head file to project directory


