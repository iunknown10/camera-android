#include <jni.h>
#include <string>
#include <android/log.h>
#include "x264Encoder.h"
#include "xhRtmpclient.h"

#define LOG_TAG "JNI_Interface"

static JavaVM *s_javavm = NULL;
#define JNI_FUNCTION(function) Java_com_harrison_camera_##function

static X264Encoder encoder;
static xhRTMPStream stream;
static std::string server = "rtmp://101.251.103.82:1935/hls/demo?user=demo&pass=34bf539b505450d2f44b7cbb6e591a89";

static AVCodecContext *avContext = NULL;
static AVFrame *frame = NULL;
static uint16_t *samples = NULL;

static int videoWidth = 0;
static int VideoHeight = 0;

class encoderHandler : public X264Handler
{
public:
    virtual void spsppsCallback(uint8_t* sps, int spsSize, uint8_t* pps, int ppsSize);
    virtual void dataCallback(uint8_t* data, int size, bool keyFrame);
};

void encoderHandler::spsppsCallback(uint8_t* sps, int spsSize, uint8_t* pps, int ppsSize)
{
	RTMPMetadata metaData;  
    memset(&metaData,0,sizeof(RTMPMetadata));
    metaData.nSpsLen = spsSize;
    memcpy(metaData.Sps,sps,spsSize);

    metaData.nPpsLen = ppsSize;  
    memcpy(metaData.Pps,pps,ppsSize);

    metaData.nWidth = videoWidth;  
    metaData.nHeight = VideoHeight;  
    metaData.nFrameRate = 25;
    //stream.SendMetadata(&metaData);
}

long long GetTickCount()
{
	struct  timeval    tv;
	struct  timezone   tz;
	gettimeofday(&tv,&tz);
    return (((long long)(tv.tv_sec) * 1000)+ tv.tv_usec/1000);
}

void encoderHandler::dataCallback(uint8_t* data, int size, bool keyFrame)
{
	static long long startTime = GetTickCount();
	long pts = GetTickCount() - startTime;

	int decollator = 0;
	if (data[2] == 0x00)
	{
		decollator = 4;
	} 
	else if (data[2] == 0x01)
	{
		decollator = 3;
	}

	//stream.SendH264Packet(data+decollator, size-decollator, keyFrame, pts);
}

jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
	s_javavm = vm;

	JNIEnv* env;
    vm->GetEnv((void**)&env, JNI_VERSION_1_2);
    vm->AttachCurrentThread(&env, NULL);

    return JNI_VERSION_1_2;
}

extern "C" {
    JNIEXPORT void JNICALL JNI_FUNCTION(CameraPreview_stopVideo) (JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL JNI_FUNCTION(CameraPreview_initVideo) (JNIEnv * env, jobject obj, jint width, jint height);
    JNIEXPORT void JNICALL JNI_FUNCTION(CameraPreview_processVideo) (JNIEnv * env, jobject obj, jbyteArray array, jint length);
    JNIEXPORT jint JNICALL JNI_FUNCTION(AudioRecorde_initAudio) (JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL JNI_FUNCTION(AudioRecorde_stopAudio) (JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL JNI_FUNCTION(AudioRecorde_processAudio) (JNIEnv * env, jobject obj, jbyteArray array, jint length);
};

void ff_log_callback(void*avcl, int level, const char*fmt, va_list vl)  
{  
    char log[1024] = {0};
    vsnprintf(log,sizeof(log),fmt,vl);  
    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "%s", log);  
} 

int initaac()
{
	av_register_all();

	av_log_set_callback(ff_log_callback);
    
    AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (!codec) {
    	__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "find encoder is failed");
    	return 0;
    }
    
    avContext = avcodec_alloc_context3(codec);

    /* put sample parameters */
    avContext->bit_rate = 64000;
    avContext->sample_rate = 44100;
    avContext->channels = 2;
    avContext->channel_layout = av_get_default_channel_layout(2);
    avContext->sample_fmt = AV_SAMPLE_FMT_FLT;
    
    codec->capabilities &= ~CODEC_CAP_EXPERIMENTAL;

    if (avcodec_open2(avContext, codec, NULL) < 0) {
        __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "open codec is failed");
       	avcodec_close(avContext);
       	return 0;
    }

    frame = avcodec_alloc_frame();
    frame->nb_samples = avContext->frame_size;
    frame->format = avContext->sample_fmt;
    frame->channel_layout = avContext->channel_layout;
    
    int buffer_size = av_samples_get_buffer_size(NULL, avContext->channels, avContext->frame_size, avContext->sample_fmt, 1);
    samples = (uint16_t*)av_malloc(buffer_size);
    avcodec_fill_audio_frame(frame, avContext->channels, avContext->sample_fmt, (const uint8_t*)samples, buffer_size, 1);

    return buffer_size;
}

void cloneaac()
{
	if (samples)
	{
		av_freep(&samples);
	}

	if (frame)
	{
		avcodec_free_frame(&frame);
	}

	if (avContext)
	{
		avcodec_close(avContext);
	}
}

JNIEXPORT void JNICALL JNI_FUNCTION(CameraPreview_stopVideo) (JNIEnv * env, jobject obj)
{
	encoder.close();
	stream.Close();
}

JNIEXPORT int JNICALL JNI_FUNCTION(AudioRecorde_initAudio) (JNIEnv * env, jobject obj)
{
	int bufferSize = initaac();
	stream.SendAACSpec(avContext->extradata, avContext->extradata_size);

	return bufferSize;
}

JNIEXPORT void JNICALL JNI_FUNCTION(AudioRecorde_stopAudio) (JNIEnv * env, jobject obj)
{
	cloneaac();
}

JNIEXPORT void JNICALL JNI_FUNCTION(CameraPreview_initVideo) (JNIEnv * env, jobject obj, jint width, jint height)
{
	if (!encoder.open(width, height, width, height))
	{
		videoWidth = width;
		VideoHeight = height;
		__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "encoder open failed");
	}
	else
	{
    	static encoderHandler handler;
		encoder.regeditCallback(&handler);

	    if (!stream.Connect(server.c_str()))
	    {
	    	__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "stream connect failed");
	    }
	}
}

JNIEXPORT void JNICALL JNI_FUNCTION(CameraPreview_processVideo) (JNIEnv * env, jobject obj, jbyteArray array, jint length)
{
	jbyte* bufferPtr = env->GetByteArrayElements(array, NULL);

	char* buffer = new char[length];
	memmove(buffer, bufferPtr, length);

	encoder.encode(buffer, length);

	delete []buffer;

	env->ReleaseByteArrayElements(array, bufferPtr, 0);
}

JNIEXPORT void JNICALL JNI_FUNCTION(AudioRecorde_processAudio) (JNIEnv * env, jobject obj, jbyteArray array, jint length)
{
	jbyte* bufferPtr = env->GetByteArrayElements(array, NULL);

	static long long startTime = GetTickCount();
	long pts = GetTickCount() - startTime;

	char* buffer = new char[length];
	memmove(buffer, bufferPtr, length);

	AVPacket pkt;
	av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    int got_output = 0;
    frame->data[0] = (uint8_t*)buffer;
    frame->pts = pts;
    if (avcodec_encode_audio2(avContext, &pkt, frame, &got_output) < 0)
    {
        __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "encode audio is failed");
    }
    else if (got_output)
    {
	    //ignore 7 bit decollator
    	stream.SendAACPacket(pkt.data+7, pkt.size-7, pts);
    }
    
    av_free_packet(&pkt);

	delete []buffer;

	env->ReleaseByteArrayElements(array, bufferPtr, 0);
}