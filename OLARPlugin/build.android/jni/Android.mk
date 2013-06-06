LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := OLARPlugin
LOCAL_ARM_MODE  := arm
LOCAL_SRC_FILES := src/OLARPlugin.cpp src/OLARScene.cpp
LOCAL_SRC_FILES += src/Android/OLARActivity.cpp
LOCAL_SRC_FILES += src/ARSub/ARCamera.cpp
LOCAL_SRC_FILES += src/ARSub/ARAndroidCamera.cpp
LOCAL_SRC_FILES += src/ARSub/ARGLUtil.cpp
LOCAL_SRC_FILES += src/ARSub/ARCommon.cpp

COCOS2DX_ROOT=$(LOCAL_PATH)/../../../cocos2dx/cocos2dx
LOCAL_CFLAGS := -DANDROID=1  -Wall -Wno-unused-function
LOCAL_CFLAGS += -I$(LOCAL_PATH)/ARSub
LOCAL_CFLAGS += -I$(COCOS2DX_ROOT)
LOCAL_CFLAGS += -I$(COCOS2DX_ROOT)/include
LOCAL_CFLAGS += -I$(COCOS2DX_ROOT)/platform/android
LOCAL_CFLAGS += -I$(COCOS2DX_ROOT)/kazmath/include

include $(BUILD_STATIC_LIBRARY)


