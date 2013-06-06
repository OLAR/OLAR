#!/bin/sh

print_usage(){
	BINNAME=`basename $0`
	echo "usage: $BINNAME <SDK_DIR> <NDK_DIR>"
}

DSTDIR=../plugins/Android

BINDIR=`dirname $0`
cd $BINDIR

if [ -f "./android-sdk.conf" ]; then
    echo "[Load saved SDK/NDK paths from android-sdk.conf]"
    source "./android-sdk.conf"
fi

if [ "$1" != "" ]; then
	ANDROID_SDK_ROOT="$1"
fi

if [ "$2" != "" ]; then
	ANDROID_NDK_ROOT="$2"
fi

SAVECONFFLAG=""

if [ "$ANDROID_SDK_ROOT" == "" ]; then
    echo "Input Android SDK path"
    read -p [AndroidSDK]: -e TMPDIR
    ANDROID_SDK_ROOT=`dirname "$TMPDIR/xxx"`
    SAVECONFFLAG=1
fi

if [ "$ANDROID_NDK_ROOT" == "" ]; then
    echo "Input Android NDK path"
    read -p [AndroidNDK]: -e TMPDIR
    ANDROID_NDK_ROOT=`dirname "$TMPDIR/xxx"`
    SAVECONFFLAG=1
fi

if [ ! -f "$ANDROID_SDK_ROOT/tools/android" ]; then
    echo "[$ANDROID_SDK_ROOT] is not Android SDK directory!"
    print_usage
    exit
fi

if [ ! -f "$ANDROID_NDK_ROOT/ndk-build" ]; then
    echo "[$ANDROID_NDK_ROOT] is not Android NDK directory!"
    print_usage
    exit
fi

echo "SDK: $ANDROID_SDK_ROOT"
echo "NDK: $ANDROID_NDK_ROOT"

if [ "$SAVECONFFLAG" == "1" ]; then
    echo "# Auto saved Android SDK/NDK paths" > android-sdk.conf
    echo "ANDROID_SDK_ROOT=\"$ANDROID_SDK_ROOT\"" >> android-sdk.conf
    echo "ANDROID_NDK_ROOT=\"$ANDROID_NDK_ROOT\"" >> android-sdk.conf
fi

echo "Building native library..."

"$ANDROID_NDK_ROOT/ndk-build"

if [ -f ./obj/local/armeabi-v7a/libOLARPlugin.a ]; then
    cp ./obj/local/armeabi-v7a/libOLARPlugin.a "$DSTDIR/OLARPlugin.a"
    echo "Copy native library]"
fi

if [ -f ./jni/src/OLARPlugin.h ]; then
    cp ./jni/src/OLARPlugin.h "$DSTDIR/../"
    echo "Copy plugin header"
fi

ANDROID_JAR=$ANDROID_SDK_ROOT/platforms/android-10/android.jar

if [ ! -d bin ]; then
    mkdir bin
fi

if [ ! -d bin/classes ]; then
    mkdir bin/classes
fi

echo "Building java plugins..."

javac -J-Dfile.encoding=UTF8 -classpath $ANDROID_JAR:src -d bin/classes \
    src/com/olar/OLARPlugin/OLARActivity.java \
    src/com/olar/OLARPlugin/CameraConfig.java \
    src/com/olar/OLARPlugin/CameraSurfaceView.java \
    src/com/olar/OLARPlugin/L.java

jar -cf "$DSTDIR/OLARPlugin.jar" -C bin/classes com

echo "Copy plugin jar file"
