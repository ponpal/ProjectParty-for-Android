LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libluajit
LOCAL_SRC_FILES := lua/libluajit.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/lua/$(wildcard *.h)
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := mystery
LOCAL_SRC_FILES := main.cpp core/font_loading.cpp core/image_loader.cpp core/renderer.cpp core/shader.cpp core/lua_core.cpp

LOCAL_STATIC_LIBRARIES := libluajit android_native_app_glue ndk_helper
LOCAL_C_INCLUDES += $(LOCAL_PATH)/glm

LOCAL_LDLIBS :=  -llog -landroid -lEGL -lGLESv2

include $(BUILD_SHARED_LIBRARY)

$(call import-module, android/native_app_glue)
$(call import-module, android/ndk_helper)