#ifndef __FFMPEG_H__
#define __FFMPEG_H__

#include "info.h"

extern "C"
{
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/avutil.h"
#include "libavutil/mathematics.h"
#include "libswresample/swresample.h"
#include "libavutil/opt.h"
#include "libavutil/channel_layout.h"
#include "libavutil/samplefmt.h"
#include "libavdevice/avdevice.h"  //����ͷ����
#include "libavfilter/avfilter.h"
#include "libavutil/error.h"
#include "libavutil/mathematics.h"  
#include "libavutil/time.h"  
#include "libavutil/fifo.h"
#include "libavutil/audio_fifo.h"   //����������Ƭʱ���ز���������Ƶ�õ�
#include "inttypes.h"
#include "stdint.h"
};

#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avdevice.lib")
#pragma comment(lib,"avfilter.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"postproc.lib")
#pragma comment(lib,"swresample.lib")
#pragma comment(lib,"swscale.lib")

#define AUDIO_ID            0    //packet �е�ID ������ȼ�����Ƶ pocket ����Ƶ�� 0  ��Ƶ��1�������෴(Ӱ��add_out_stream˳��)
#define VIDEO_ID            1

//#define INPUTURL   "../in_stream/22.ts"
#define INPUTURL   "../in_stream/avier1.mp4"    
//#define INPUTURL   "../in_stream/father.avi"    //��������⣿û����Ƶ
//#define INPUTURL   "../in_stream/ceshi.avi" 

//m3u8 param
#define OUTPUT_PREFIX       "ZWG_TEST"              //�и��ļ���ǰ׺
#define M3U8_FILE_NAME      "ZWG_TEST.m3u8"         //���ɵ�m3u8�ļ���
#define URL_PREFIX          "../out_stream/"        //����Ŀ¼
#define NUM_SEGMENTS        10                      //�ڴ�����һ�����洢���ٸ���Ƭ
#define SEGMENT_DURATION    10                      //ÿһƬ�и������
extern unsigned int m_output_index;                 //���ɵ���Ƭ�ļ�˳����(�ڼ����ļ�)
extern char m_output_file_name[256];                //�����Ҫ��Ƭ���ļ�


extern int nRet;                                                              //״̬��־
extern AVFormatContext* icodec;												  //������context
extern AVFormatContext* ocodec ;                                              //�����context
extern char szError[256];                                                     //�����ַ���
extern AVStream* ovideo_st;
extern AVStream* oaudio_st;              
extern int video_stream_idx;
extern int audio_stream_idx;
extern AVCodec *audio_codec;
extern AVCodec *video_codec;   
extern AVBitStreamFilterContext * vbsf_aac_adtstoasc;                         //aac->adts to asc������
static struct SwsContext * img_convert_ctx_video = NULL;
static int sws_flags = SWS_BICUBIC; //��ֵ�㷨,˫����
extern AVBitStreamFilterContext * vbsf_h264_toannexb;
extern int IsAACCodes;

int init_demux(char * Filename,AVFormatContext ** iframe_c);
int init_mux();
int uinit_demux();
int uinit_mux();
//for mux
AVStream * add_out_stream(AVFormatContext* output_format_context,AVMediaType codec_type_t); 

//�������Ƭ����
void slice_up();
//��дm3u8�ļ�
int write_index_file(const unsigned int first_segment, const unsigned int last_segment, const int end, const unsigned int actual_segment_durations[]);


#endif