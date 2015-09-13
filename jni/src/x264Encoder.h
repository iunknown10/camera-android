//
//  x264Encoder.h
//  voicesdk
//
//  Created by heyunpeng on 15/9/10.
//  Copyright (c) 2015年 heyunpeng. All rights reserved.
//

#ifndef voicesdk_x264Encoder_h
#define voicesdk_x264Encoder_h
#include <inttypes.h>
#include <stdio.h>
#include <string>

extern "C" {
#include <x264.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavfilter/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavutil/error.h>
#include <libavutil/pixfmt.h>
}

class X264Handler
{
public:
    virtual void spsppsCallback(uint8_t* sps, int spsSize, uint8_t* pps, int ppsSize)=0;
    virtual void dataCallback(uint8_t* data, int size, bool keyFrame)=0;
};

class X264Encoder
{
public:
    X264Encoder();
    virtual ~X264Encoder();

    void regeditCallback(X264Handler* handler){callback=handler;};

    bool open(int width, int height, int outwidth, int outheight); /* open for encoding */
    void encode(char* pixels, int size);
    bool close();                                                 /* close the encoder and file, frees all memory */
private:
    void setParams();                                             /* sets the x264 params */
    
public:
    /* params the user should set */
    int in_width;
    int in_height;
    int out_width;
    int out_height;
    
    /* x264 */
    AVPicture pic_raw;                                            /* used for our "raw" input container */
    x264_picture_t pic_in;
    x264_picture_t pic_out;
    x264_param_t params;
    x264_nal_t* nals;
    x264_t* encoder;
    int num_nals;
    
    /* input / output */
    struct SwsContext* sws;

    X264Handler *callback;
};

#endif
