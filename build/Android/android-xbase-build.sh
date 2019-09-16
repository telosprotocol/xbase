#! /bin/sh

#/Users/taylorwei/Downloads/exchange/xtopcom/xbase/build/Android/android-xbase-build.sh release armeabi-v7a
echo " "
echo "***************Start Build Android top xbase libs*******************"
echo "$PWD"

DEBUG="debug"
RELEASE="release"
NDKPATH="/Volumes/Samsung_T1/android/ndk/android-ndk-r18b"
JNIPATH="/Users/taylorwei/Downloads/exchange/xtopcom/xbase/build/Android"
APPABI="armeabi-v7a"
#armeabi x86

if [ -n "$1" ]; then
	echo "Build as : ${1}"
else
   	echo "Please pass Build option: debug or release"
    exit 1
fi

if [ "$1" = "$DEBUG" ]; then
  	echo "start build Debug Libs"
else
	echo "start build Release Libs"
fi

if [ -n "$2" ]; then
 APPABI=$2
 echo "Build for : ${APPABI}"
else 
 echo "Build for : ${APPABI}"
fi


echo "***************Start Build xbase lib*******************"
cd  $JNIPATH
$NDKPATH/ndk-build clean

rm  -r -f $JNIPATH/obj
 
if [ "$1" = "$DEBUG" ]; then
	rm -f $JNIPATH/../../libs/Android/debug/libxbase.a
	$NDKPATH/ndk-build NDK_DEBUG=1
	cp $JNIPATH/obj/local/$APPABI/libxbase.a   $JNIPATH/../../libs/Android/debug/
	ls -l $JNIPATH/../../libs/Android/debug
else
	rm -f $JNIPATH/../../libs/Android/release/libxbase.a
	$NDKPATH/ndk-build NDK_DEBUG=0
	cp $JNIPATH/obj/local/$APPABI/libxbase.a   $JNIPATH/../../libs/Android/release/
	ls -l $JNIPATH/../../libs/Android/release
fi

echo "***************Finish Build xbase lib*******************"