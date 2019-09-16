APP_MODULES:=xbase
APP_OPTIM:=release
APP_CPPFLAGS += -std=c++11
APP_CPPFLAGS += -frtti
APP_CPPFLAGS += -fexceptions
#c99 option -std=c99
APP_STL := c++_static
#android-16 -> android os 4.x
#android-21 -> android os 5.x
#android-24 -> android os 7.x
APP_PLATFORM := android-16
APP_ABI := armeabi-v7a

#armeabi nolonger support by latest NDK
#APP_ABI := armeabi armeabi-v7a
#APP_ABI := armeabi armeabi-v7a x86

#must using NDSK >= 4.7 to support C++11
#NDSK 4.8 ask use GCC 4.8,NDSK4.7 ask GCC4.7
NDK_TOOLCHAIN_VERSION=4.8