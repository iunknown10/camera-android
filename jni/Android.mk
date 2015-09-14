LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

FFMPEG_LIB_PATH := ${LOCAL_PATH}/../third_party/libs
X264_LIB_PATH := ${LOCAL_PATH}/../third_party/libs
OPENSSL_LIB_PATH := ${LOCAL_PATH}/../third_party/libs

LOCAL_MODULE    := core


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

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
                    $(LOCAL_PATH)/src \
                    ${LOCAL_PATH}/../third_party/include\
                    ${LOCAL_PATH}/../third_party/include/ffmpeg\
                    ${LOCAL_PATH}/../third_party/include/librtmp\


#					/Users/heyunpeng/workstation/src/AndroidFFmpeg/library-jni/jni/ffmpeg \
#					/Users/heyunpeng/workstation/src/librtmp-android/jni/include \
#					/Users/heyunpeng/workstation/src/x264-android/build/android/include \
#					/Users/heyunpeng/workstation/src/librtmp-android/jni/include \
#					/Users/heyunpeng/workstation/src/librtmp-android/jni/include/librtmp \
#					/Users/heyunpeng/workstation/src/OpenSSL-for-iPhone/include \
					

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
