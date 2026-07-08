#!/bin/bash
#
# This script is based on SDL sources => androidbuild.sh
#
##

SOURCES=()
MKSOURCES=""
CURDIR="$( pwd -P )"


if [ -z "$SDLPATH" ];then
    echo "Please set the SDLPATH directory to the path of the SDL2 source code"
    exit 1
fi

if [ -z "$ANDROID_HOME" ];then
    echo "Please set the ANDROID_HOME directory to the path of the Android SDK"
    exit 1
fi

if [ ! -d "$ANDROID_HOME/ndk-bundle" ] && [ -z "$ANDROID_NDK_HOME" ];  then
    echo "Please set the ANDROID_NDK_HOME directory to the path of the Android NDK"
    exit 1
fi

APP="com.demogles2.nuklear"
APPARR=(${APP//./ })
BUILDPATH="$CURDIR/../build/$APP"

# Start Building
# Whe are erasing previos building
rm -rf $BUILDPATH
mkdir -p $BUILDPATH

cp -r $SDLPATH/android-project/* $BUILDPATH
rm -r $BUILDPATH/app/jni/src/*

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
cp AssetManager.h $BUILDPATH/app/jni/nuklear

ln -s $CURDIR/../../../nuklear.h $BUILDPATH/app/jni/nuklear/nuklear.h
ln -s $CURDIR/../../sdl_opengles2/nuklear_sdl_gles2.h $BUILDPATH/app/jni/nuklear/nuklear_sdl_gles2.h
ln -s $CURDIR/../../../example/stb_image.h $BUILDPATH/app/jni/nuklear/stb_image.h

#wee need to add some images assets
mkdir $BUILDPATH/app/src/main/assets
cp -r ${CURDIR}/../../../example/images/* $BUILDPATH/app/src/main/assets/
cp -r ${CURDIR}/../../../example/icon/* $BUILDPATH/app/src/main/assets/

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

import android.content.res.AssetManager;
import android.os.Bundle;

import org.libsdl.app.SDLActivity;

public class $ACTIVITY extends SDLActivity
{
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setAssetsNativeManager(getAssets());
    }

    public native void setAssetsNativeManager(AssetManager assetManager);
}
__EOF__

# Updating and building project
cd $BUILDPATH
./gradlew installDebug
