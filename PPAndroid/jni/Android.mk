LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)
LOCAL_MODULE := libluajit
LOCAL_SRC_FILES := lua/libluajit.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/lua/$(wildcard *.h)
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libcmain
LOCAL_SRC_FILES := cmain.cpp test.cpp

LOCAL_STATIC_LIBRARIES := libluajit

LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog 

include $(BUILD_SHARED_LIBRARY)
