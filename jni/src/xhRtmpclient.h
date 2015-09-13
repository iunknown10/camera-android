//
//  xhRtmpclient.h
//  voicesdk
//
//  Created by heyunpeng on 15/9/10.
//  Copyright (c) 2015年 heyunpeng. All rights reserved.
//

#ifndef voicesdk_xhRtmpclient_h
#define voicesdk_xhRtmpclient_h
#include "librtmp/rtmp.h"
#include "librtmp/amf.h"
#include <stdio.h>

#define FILEBUFSIZE (1024 * 1024 * 10)       //  10M

// NALU单元
typedef struct _NaluUnit
{
    int type;
    int size;
    unsigned char *data;
}NaluUnit;

typedef struct _RTMPMetadata
{
    // video, must be h264 type
    unsigned int    nWidth;
    unsigned int    nHeight;
    unsigned int    nFrameRate;     // fps
    unsigned int    nVideoDataRate; // bps
    unsigned int    nSpsLen;
    unsigned char   Sps[1024];
    unsigned int    nPpsLen;
    unsigned char   Pps[1024];
    
    // audio, must be aac type
    bool            bHasAudio;
    unsigned int    nAudioSampleRate;
    unsigned int    nAudioSampleSize;
    unsigned int    nAudioChannels;
    char            pAudioSpecCfg;
    unsigned int    nAudioSpecCfgLen;
    
} RTMPMetadata,*LPRTMPMetadata;


class xhRTMPStream
{
public:
    xhRTMPStream(void);
    ~xhRTMPStream(void);
public:
    // 连接到RTMP Server
    bool Connect(const char* url);
    // 断开连接
    void Close();
    // 发送MetaData
    bool SendMetadata(LPRTMPMetadata lpMetaData);
    // 发送H264数据帧
    int SendH264spspps(unsigned char *sps, unsigned int spsSize, unsigned char *pps, unsigned int ppsSize);
    int SendH264Packet(unsigned char *data,unsigned int size,unsigned int nTimeStamp);
    int SendAACPacket(unsigned char *data, unsigned int size);
private:
    // 送缓存中读取一个NALU包
    bool ReadOneNaluFromBuf(NaluUnit &nalu);
    // 发送数据
    int SendPacket(unsigned int nPacketType,unsigned char *data,unsigned int size,unsigned int nTimestamp);
private:
    RTMP* _pRtmp;
    unsigned char* _pFileBuf;
    unsigned int  _nFileBufSize;
    unsigned int  _nCurPos;
};

#endif
