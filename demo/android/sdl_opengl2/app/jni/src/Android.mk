#APP_ALLOW_MISSING_DEPS=true

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH := ../SDL

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include

LOCAL_SRC_FILES := main-gles2.c

LOCAL_SHARED_LIBRARIES := SDL2 hidapi

LOCAL_LDLIBS := -landroid -lGLESv1_CM -lGLESv2 -llog

include $(BUILD_SHARED_LIBRARY)


