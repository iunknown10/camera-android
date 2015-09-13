//
//  xhRtmpclient.cpp
//  voicesdk
//
//  Created by heyunpeng on 15/9/10.
//  Copyright (c) 2015年 heyunpeng. All rights reserved.
//

#include <stdio.h>
#include "xhRtmpclient.h"
#include <string>
#include <android/log.h>

#define LOG_TAG "rtmp"

enum
{
    FLV_CODECID_H264 = 7,
};

char * put_byte( char *output, uint8_t nVal )
{
    output[0] = nVal;
    return output+1;
}
char * put_be16(char *output, uint16_t nVal )
{
    output[1] = nVal & 0xff;
    output[0] = nVal >> 8;
    return output+2;
}
char * put_be24(char *output,uint32_t nVal )
{
    output[2] = nVal & 0xff;
    output[1] = nVal >> 8;
    output[0] = nVal >> 16;
    return output+3;
}
char * put_be32(char *output, uint32_t nVal )
{
    output[3] = nVal & 0xff;
    output[2] = nVal >> 8;
    output[1] = nVal >> 16;
    output[0] = nVal >> 24;
    return output+4;
}
char *  put_be64( char *output, uint64_t nVal )
{
    output=put_be32( output, nVal >> 32 );
    output=put_be32( output, nVal );
    return output;
}
char * put_amf_string( char *c, const char *str )
{
    uint16_t len = strlen( str );
    c=put_be16( c, len );
    memcpy(c,str,len);
    return c+len;
}
char * put_amf_double( char *c, double d )
{
    *c++ = AMF_NUMBER;  /* type: Number */
    {
        unsigned char *ci, *co;
        ci = (unsigned char *)&d;
        co = (unsigned char *)c;
        co[0] = ci[7];
        co[1] = ci[6];
        co[2] = ci[5];
        co[3] = ci[4];
        co[4] = ci[3];
        co[5] = ci[2];
        co[6] = ci[1];
        co[7] = ci[0];
    }
    return c+8;
}

xhRTMPStream::xhRTMPStream(void):
_pRtmp(NULL),
_nFileBufSize(0),
_nCurPos(0)
{
    _pFileBuf = new unsigned char[FILEBUFSIZE];
    memset(_pFileBuf,0,FILEBUFSIZE);
    _pRtmp = RTMP_Alloc();
    RTMP_Init(_pRtmp);
}

xhRTMPStream::~xhRTMPStream(void)
{
    Close();
    delete[] _pFileBuf;
}

bool xhRTMPStream::Connect(const char* url)
{
    if(RTMP_SetupURL(_pRtmp, (char*)url)<0)
        return false;

    //public stream
    RTMP_EnableWrite(_pRtmp);
    if(RTMP_Connect(_pRtmp, NULL) < 0 || RTMP_ConnectStream(_pRtmp, 0) < 0)
        return false;

    return true;
}

void xhRTMPStream::Close()
{
    if(_pRtmp)
    {
        RTMP_Close(_pRtmp);
        RTMP_Free(_pRtmp);
        _pRtmp = NULL;
    }
}

int xhRTMPStream::SendPacket(unsigned int nPacketType,unsigned char *data,unsigned int size,unsigned int nTimestamp)
{
    if(_pRtmp == NULL)
        return false;
    
    RTMPPacket packet;
    RTMPPacket_Reset(&packet);
    RTMPPacket_Alloc(&packet,size);
    
    packet.m_packetType = nPacketType;
    packet.m_nChannel = 0x04;
    packet.m_headerType = RTMP_PACKET_SIZE_LARGE;
    packet.m_nTimeStamp = nTimestamp;
    packet.m_nInfoField2 = _pRtmp->m_stream_id;
    packet.m_nBodySize = size;
    packet.m_hasAbsTimestamp = 0;
    memcpy(packet.m_body,data,size);
    
    int nRet = RTMP_SendPacket(_pRtmp,&packet,false);
    
    RTMPPacket_Free(&packet);
    
    return nRet;
}

bool xhRTMPStream::SendMetadata(LPRTMPMetadata lpMetaData)
{
    if(lpMetaData == NULL)
        return false;

    char body[1024] = {0};;
    
    char * p = (char *)body;
    p = put_byte(p, AMF_STRING );
    p = put_amf_string(p , "@setDataFrame" );
    
    p = put_byte( p, AMF_STRING );
    p = put_amf_string( p, "onMetaData" );
    
    p = put_byte(p, AMF_OBJECT );
    p = put_amf_string( p, "copyright" );
    p = put_byte(p, AMF_STRING );
    p = put_amf_string( p, "firehood" );
    
    p =put_amf_string( p, "width");
    p =put_amf_double( p, lpMetaData->nWidth);
    
    p =put_amf_string( p, "height");
    p =put_amf_double( p, lpMetaData->nHeight);
    
    p =put_amf_string( p, "framerate" );
    p =put_amf_double( p, lpMetaData->nFrameRate);
    
    p =put_amf_string( p, "videocodecid" );
    p =put_amf_double( p, FLV_CODECID_H264 );
    
    p =put_amf_string( p, "" );
    p =put_byte( p, AMF_OBJECT_END  );
        
    SendPacket(RTMP_PACKET_TYPE_INFO,(unsigned char*)body,p-body,0);
    
    int i = 0;
    body[i++] = 0x17; // 1:keyframe  7:AVC
    body[i++] = 0x00; // AVC sequence header
    
    body[i++] = 0x00;
    body[i++] = 0x00;
    body[i++] = 0x00; // fill in 0;
    
    // AVCDecoderConfigurationRecord.
    body[i++] = 0x01; // configurationVersion
    body[i++] = lpMetaData->Sps[1]; // AVCProfileIndication
    body[i++] = lpMetaData->Sps[2]; // profile_compatibility
    body[i++] = lpMetaData->Sps[3]; // AVCLevelIndication
    body[i++] = 0xff; // lengthSizeMinusOne
    
    // sps nums
    body[i++] = 0xE1; //&0x1f
    // sps data length
    body[i++] = lpMetaData->nSpsLen>>8;
    body[i++] = lpMetaData->nSpsLen&0xff;
    // sps data
    memcpy(&body[i],lpMetaData->Sps,lpMetaData->nSpsLen);
    i= i+lpMetaData->nSpsLen;
    
    // pps nums
    body[i++] = 0x01; //&0x1f
    // pps data length
    body[i++] = lpMetaData->nPpsLen>>8;
    body[i++] = lpMetaData->nPpsLen&0xff;
    // sps data
    memcpy(&body[i],lpMetaData->Pps,lpMetaData->nPpsLen);
    i= i+lpMetaData->nPpsLen;
    
    return SendPacket(RTMP_PACKET_TYPE_VIDEO,(unsigned char*)body,i,0);
    
}

int xhRTMPStream::SendAACPacket(unsigned char *data, unsigned int size)
{
    RTMPPacket packet;
    RTMPPacket_Reset(&packet);
    RTMPPacket_Alloc(&packet, size+2);
    
    unsigned char *body = new unsigned char[size+2];

    /*AF 00 + AAC RAW data*/
    body[0] = 0xAF;
    body[1] = 0x00;
    memcpy(&body[2],data,size); /*spec_buf是AAC sequence header数据*/
    
    packet.m_packetType = RTMP_PACKET_TYPE_AUDIO;
    packet.m_nBodySize = size+2;
    packet.m_nChannel = 0x04;
    packet.m_nTimeStamp = 0;
    packet.m_hasAbsTimestamp = 0;
    packet.m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    packet.m_nInfoField2 = _pRtmp->m_stream_id;
    memcpy(packet.m_body,body,size+2);
    
    /*调用发送接口*/
    int nRet = RTMP_SendPacket(_pRtmp, &packet, false);
    
    RTMPPacket_Free(&packet);
    delete []body;
    
    return nRet;
}

int xhRTMPStream::SendH264spspps(unsigned char *sps, unsigned int spsSize, unsigned char *pps, unsigned int ppsSize)
{
    if (sps == NULL || pps == NULL)
        return 0;

    unsigned char* body = new unsigned char[spsSize+ppsSize+24];
    int i = 0;
    body[i++] = 0x17;
    body[i++] = 0x00;
 
    body[i++] = 0x00;
    body[i++] = 0x00;
    body[i++] = 0x00;
 
    /*AVCDecoderConfigurationRecord*/
    body[i++] = 0x01;
    body[i++] = sps[1];
    body[i++] = sps[2];
    body[i++] = sps[3];
    body[i++] = 0xff;
 
    /*sps*/
    body[i++]   = 0xe1;
    body[i++] = (spsSize >> 8) & 0xff;
    body[i++] = spsSize & 0xff;
    memcpy(&body[i],sps,spsSize);
    i +=  spsSize;
 
    /*pps*/
    body[i++]   = 0x01;
    body[i++] = (ppsSize >> 8) & 0xff;
    body[i++] = (ppsSize) & 0xff;
    memcpy(&body[i],pps,ppsSize);
    i +=  ppsSize;

    RTMPPacket packet;
    RTMPPacket_Reset(&packet);
    RTMPPacket_Alloc(&packet, i);

    packet.m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet.m_nBodySize = i;
    packet.m_nChannel = 0x04;
    packet.m_nTimeStamp = 0;
    packet.m_hasAbsTimestamp = 0;
    packet.m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    packet.m_nInfoField2 = _pRtmp->m_stream_id;
    memcpy(packet.m_body,body,i);

    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "send sps|ppsadfadfad");
 
    /*调用发送接口*/
    int nRet = RTMP_SendPacket(_pRtmp,&packet,false);

    RTMPPacket_Free(&packet);
    delete []body;
 
    return nRet;
}

int xhRTMPStream::SendH264Packet(unsigned char *data,unsigned int size,unsigned int nTimeStamp)
{
    if(data == NULL && size < 11)
        return false;
    
    unsigned char *body = new unsigned char[size+9];
    
    int i = 0;
#define NAL_SLICE_IDR 5
    if(data[0]&0x1f == NAL_SLICE_IDR)
    {
        body[i++] = 0x17;// 1:Iframe  7:AVC
    }
    else
    {
        body[i++] = 0x27;// 2:Pframe  7:AVC
    }

    body[i++] = 0x01;// AVC NALU
    body[i++] = 0x00;
    body[i++] = 0x00;
    body[i++] = 0x00;
    
    // NALU size
    body[i++] = size>>24;
    body[i++] = size>>16;
    body[i++] = size>>8;
    body[i++] = size&0xff;;
    
    // NALU data
    memcpy(&body[i],data,size);
    
    int nRet = SendPacket(RTMP_PACKET_TYPE_VIDEO,body,i+size,nTimeStamp);
    
    delete[] body;
    
    return nRet;
}