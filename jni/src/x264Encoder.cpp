//
//  x264Encoder.cpp
//  voicesdk
//
//  Created by heyunpeng on 15/9/10.
//  Copyright (c) 2015年 heyunpeng. All rights reserved.
//

#include <stdio.h>
#include "x264Encoder.h"
#include <android/log.h>

#define LOG_TAG "encoder"

X264Encoder::X264Encoder()
:in_width(0)
,in_height(0)
,out_width(0)
,out_height(0)
,encoder(NULL)
,sws(NULL)
,num_nals(0)
,callback(NULL)
{
    memset((char*)&pic_raw, 0, sizeof(pic_raw));
}

X264Encoder::~X264Encoder()
{
    if(sws)
    {
        close();
    }
}

bool X264Encoder::open(int width, int height, int outwidth, int outheight)
{
    if(encoder)
        return false;
    
    in_width = width;
    in_height = height;
    out_width = outwidth;
    out_height = outheight;
    
    sws = sws_getContext(in_width, in_height, PIX_FMT_YUV420P,
                         out_width, out_height, PIX_FMT_YUV420P,
                         SWS_FAST_BILINEAR, NULL, NULL, NULL);
    
    if(!sws)
        return false;

    x264_picture_alloc(&pic_in, X264_CSP_I420, out_width, out_height);
    
    setParams();
    
    // create the encoder using our params
    encoder = x264_encoder_open(&params);
    if(!encoder)
        return false;
    
    // write headers
    int nheader = 0;
    int r = x264_encoder_headers(encoder, &nals, &nheader);
    if(r < 0)
        return false;

    return true;
}

void X264Encoder::encode(char* pixels, int size)
{
    // copy the pixels into our "raw input" container.
    if (!avpicture_fill(&pic_raw, (uint8_t*)pixels, PIX_FMT_YUV420P, in_width, in_height))
        return;
    
    // convert to I420 for x264
    if (sws_scale(sws, pic_raw.data, pic_raw.linesize, 0, in_height, pic_in.img.plane, pic_in.img.i_stride) != out_height)
        return;
    
    int frame_size = x264_encoder_encode(encoder, &nals, &num_nals, &pic_in, &pic_out);
    for (int i = 0; i < num_nals; i++)
    {
        static int sps_len = 0;
        static uint8_t* sps_data = NULL;

        static int pps_len = 0;
        static uint8_t* pps_data = NULL;
        if (nals[i].i_type == NAL_SPS)
        {
            sps_len = nals[i].i_payload-4;
            sps_data = new uint8_t[sps_len];
            memcpy(sps_data, nals[i].p_payload+4, sps_len);
        }
        else if (nals[i].i_type == NAL_PPS)
        {
            pps_len = nals[i].i_payload-4;
            pps_data = new uint8_t[pps_len];
            memcpy(pps_data, nals[i].p_payload+4, pps_len);

            if (callback) callback->spsppsCallback(sps_data, sps_len, pps_data, pps_len);
        }
        else
        {
            if (callback) callback->dataCallback(nals[i].p_payload, nals[i].i_payload, pic_out.b_keyframe==1);
        }
    }
}

bool X264Encoder::close()
{
    if(encoder)
    {
        x264_picture_clean(&pic_in);
        memset((char*)&pic_in, 0, sizeof(pic_in));
        memset((char*)&pic_out, 0, sizeof(pic_out));
        
        x264_encoder_close(encoder);
        encoder = NULL;
    }
    
    if(sws)
    {
        sws_freeContext(sws);
        sws = NULL;
    }
    
    memset((char*)&pic_raw, 0, sizeof(pic_raw));
    
    return true;
}

void X264Encoder::setParams()
{
    x264_param_default_preset(&params, "ultrafast", "zerolatency");
    params.i_threads = 1;
    params.i_width = out_width;
    params.i_height = out_height;
    params.i_fps_num = 25;
    params.i_fps_den = 1;
}