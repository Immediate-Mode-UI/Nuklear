/* nuklear - 1.40.8 - public domain
 *
 * Assets manager is based on https://github.com/mambrosi/JNI-Asset-Manager
 *
 * Implemented by leonel hernandez
 *
 * */


#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TAG "Native Asset Manager"

AAssetManager* assetManager;

JNIEXPORT void JNICALL
Java_com_demogles2_nuklear_nuklearActivity_setAssetsNativeManager(JNIEnv *env, jobject thiz, jobject asset_manager)
{
    assetManager = AAssetManager_fromJava(env, asset_manager);
    assert(NULL != assetManager);
}


unsigned char * getAssetFile(const char *filename, int *x, int *y, int *n)
{
    unsigned char *data;

    AAsset* fileAsset = AAssetManager_open(assetManager, filename, AASSET_MODE_UNKNOWN);

    if (fileAsset)
    {
        assert(fileAsset);

        size_t asset_length = AAsset_getLength(fileAsset);

        __android_log_print(ANDROID_LOG_DEBUG, TAG, "Asset file size: %zu\n", asset_length);

        char *buffer = (char*) malloc(asset_length + 1);
        AAsset_read(fileAsset, buffer, asset_length);
        buffer[asset_length] = 0;

        data = stbi_load_from_memory((const unsigned char*)buffer, asset_length, x, y, n, 0);
        if (!data)
            __android_log_print(ANDROID_LOG_ERROR, TAG, "[SDL]: failed to load image: %s", filename);

        __android_log_print(ANDROID_LOG_INFO, TAG, "The value is %s", buffer);

        AAsset_close(fileAsset);
        free(buffer);
    }
    else
    {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Cannot open file");
    }

    return data;
}
