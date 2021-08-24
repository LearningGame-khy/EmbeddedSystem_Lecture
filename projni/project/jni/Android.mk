LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := projni
LOCAL_SRC_FILES := projni.c

LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog
LOCAL_CFLAGS := -DCONFIG_EMBEDDED -DUSE_IND_THREAD

include $(BUILD_SHARED_LIBRARY)
