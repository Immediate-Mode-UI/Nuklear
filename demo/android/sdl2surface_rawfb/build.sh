#!/bin/bash
#
# This script is based on SDL sources => androidbuild.sh
#
##

SOURCES=()
MKSOURCES=""
CURDIR=`pwd -P`


if [ -z "$SDLPATH" ];then
    echo "Please set the SDLPATH directory to the path of the SDL2 source code"
    exit 1
fi

if [ -z "$ANDROID_HOME" ];then
    echo "Please set the ANDROID_HOME directory to the path of the Android SDK"
    exit 1
fi

if [ ! -d "$ANDROID_HOME/ndk-bundle" -a -z "$ANDROID_NDK_HOME" ]; then
    echo "Please set the ANDROID_NDK_HOME directory to the path of the Android NDK"
    exit 1
fi

APP="com.sdl2rawfb.nuklear"
APPARR=(${APP//./ })
BUILDPATH="$CURDIR/../build/$APP"

# Start Building

rm -rf $BUILDPATH
mkdir -p $BUILDPATH

cp -r $CURDIR/../project-template/* $BUILDPATH

# Create SDL jni struct
mkdir -p $BUILDPATH/app/jni
mkdir -p $BUILDPATH/app/jni/SDL
mkdir -p $BUILDPATH/app/jni/nuklear
ln -s $SDLPATH/src $BUILDPATH/app/jni/SDL/src
ln -s $SDLPATH/include $BUILDPATH/app/jni/SDL/SDL2
cp -r $CURDIR/../Android.mk $BUILDPATH/app/jni/SDL/
cp -r Android.mk $BUILDPATH/app/jni/nuklear

sed -i -e "s|org\.libsdl\.app|$APP|g" $BUILDPATH/app/build.gradle
sed -i -e "s|org\.libsdl\.app|$APP|g" $BUILDPATH/app/src/main/AndroidManifest.xml

# Copy user sources
cp main.c $BUILDPATH/app/jni/nuklear

ln -s $CURDIR/../../../nuklear.h $BUILDPATH/app/jni/nuklear/nuklear.h
ln -s $CURDIR/../../sdl2surface_rawfb/sdl2surface_rawfb.h $BUILDPATH/app/jni/nuklear/sdl2surface_rawfb.h


# Create an inherited Activity
cd $BUILDPATH/app/src/main/java
for folder in "${APPARR[@]}"
do
    mkdir -p $folder
    cd $folder
done

ACTIVITY="${folder}Activity"
sed -i -e "s|\"SDLActivity\"|\"$ACTIVITY\"|g" $BUILDPATH/app/src/main/AndroidManifest.xml

# Fill in a default Activity
cat >"$ACTIVITY.java" <<__EOF__
package $APP;


import org.libsdl.app.SDLActivity;

public class $ACTIVITY extends SDLActivity
{

}
__EOF__

# Update project and build
cd $BUILDPATH
./gradlew installDebug
