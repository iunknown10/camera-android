LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

FFMPEG_LIB_PATH := /Users/heyunpeng/workstation/src/AndroidFFmpeg/library-jni/jni/ffmpeg-build/armeabi-v7a-neon/lib
X264_LIB_PATH := /Users/heyunpeng/workstation/src/x264-android/build/android/lib
OPENSSL_LIB_PATH := /Users/heyunpeng/workstation/src/OpenSSL1.0.1cForAndroid/obj/local/armeabi-v7a

LOCAL_MODULE    := core

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
                        $(MY_DIR)/src \

LOCAL_SRC_FILES := src/jni_interface.cpp \
					src/x264Encoder.cpp \
					src/xhRtmpclient.cpp \
					src/rtmp/amf.c \
					src/rtmp/hashswf.c \
					src/rtmp/log.c \
					src/rtmp/parseurl.c \
					src/rtmp/rtmp.c \

LOCAL_CFLAGS += -DBUILD_OGLES2
LOCAL_CFLAGS += -W -Wall

LOCAL_CPPFLAGS += -frtti

LOCAL_C_INCLUDES := /Users/heyunpeng/workstation/src/AndroidFFmpeg/library-jni/jni/ffmpeg \
					/Users/heyunpeng/workstation/src/librtmp-android/jni/include \
					/Users/heyunpeng/workstation/src/x264-android/build/android/include \
					/Users/heyunpeng/workstation/src/librtmp-android/jni/include \
					/Users/heyunpeng/workstation/src/librtmp-android/jni/include/librtmp \
					/Users/heyunpeng/workstation/src/OpenSSL-for-iPhone/include \

LOCAL_LDLIBS += -L$(FFMPEG_LIB_PATH) -lswscale
LOCAL_LDLIBS += -L$(FFMPEG_LIB_PATH) -lfribidi
LOCAL_LDLIBS += -L$(FFMPEG_LIB_PATH) -lavformat
LOCAL_LDLIBS += -L$(FFMPEG_LIB_PATH) -lavcodec
LOCAL_LDLIBS += -L$(FFMPEG_LIB_PATH) -lavresample
LOCAL_LDLIBS += -L$(FFMPEG_LIB_PATH) -lavutil
LOCAL_LDLIBS += -L$(FFMPEG_LIB_PATH) -lswresample
LOCAL_LDLIBS += -L$(FFMPEG_LIB_PATH) -lass
LOCAL_LDLIBS += -L$(FFMPEG_LIB_PATH) -lfreetype
LOCAL_LDLIBS += -L$(FFMPEG_LIB_PATH) -lvo-aacenc
LOCAL_LDLIBS += -L$(FFMPEG_LIB_PATH) -lvo-amrwbenc

LOCAL_LDLIBS += -L$(OPENSSL_LIB_PATH) -lssl
LOCAL_LDLIBS += -L$(OPENSSL_LIB_PATH) -lcrypto

LOCAL_LDLIBS += -L$(X264_LIB_PATH) -lx264
LOCAL_LDLIBS += -llog
LOCAL_LDLIBS += -lz
LOCAL_LDLIBS += -lm

include $(BUILD_SHARED_LIBRARY)
