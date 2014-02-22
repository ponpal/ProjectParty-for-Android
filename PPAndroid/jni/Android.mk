LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libluajit
LOCAL_SRC_FILES := lua/libluajit.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/lua/$(wildcard *.h)
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := mystery
LOCAL_SRC_FILES := main.cpp
FILE_LIST := $(wildcard $(LOCAL_PATH)/core/*.cpp)
LOCAL_SRC_FILES += $(FILE_LIST:$(LOCAL_PATH)/%=%)

LOCAL_STATIC_LIBRARIES := libluajit android_native_app_glue ndk_helper
LOCAL_C_INCLUDES += $(LOCAL_PATH)/glm

LOCAL_LDLIBS :=  -llog -landroid -lEGL -lGLESv2

include $(BUILD_SHARED_LIBRARY)

$(call import-module, android/native_app_glue)
$(call import-module, android/ndk_helper)