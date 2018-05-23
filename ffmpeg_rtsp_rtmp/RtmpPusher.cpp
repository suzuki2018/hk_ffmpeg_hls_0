#include "stdafx.h"
#include "RtmpPusher.h"



#include <memory>



const int LEN = 62; // 26 + 26 + 10
char g_arrCharElem[LEN] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', \
'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', \
'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' };


std::string GeneralToken()
{
	std::string strRet;

	char* szStr = new char[7 + 1];
	szStr[7] = '\0';
	srand((unsigned)time(0));
	int iRand = 0;
	for (int i = 0; i < 7; ++i)
	{
		iRand = rand() % LEN;            // iRand = 0 - 61
		szStr[i] = g_arrCharElem[iRand];
	}

	strRet = szStr;

	delete[] szStr;
	return strRet;
}


int RtmpPusher::OpenInput(std::string inputUrl)
{
	inputContext = avformat_alloc_context();
	AVDictionary* options = nullptr;
	av_dict_set(&options, "rtsp_transport", "udp", 0);
	int ret = avformat_open_input(&inputContext, inputUrl.c_str(), nullptr, &options);
	if (ret < 0)
	{
		//av_strerror();
		av_log(NULL, AV_LOG_ERROR, "Input file open input failed\n");
		return  ret;
	}
	ret = avformat_find_stream_info(inputContext, nullptr);
	if (ret < 0)
	{
		av_log(NULL, AV_LOG_ERROR, "Find input file stream inform failed\n");
	}
	else
	{
		av_log(NULL, AV_LOG_FATAL, "Open input file  %s success\n", inputUrl.c_str());
	}

	if (ret <0)
		TRACE("@@@@@@@ Connect Rtsp media fail........\r\n");
	else
		TRACE("@@@@@@@ Connect Rtsp media success........\r\n");

	return ret;
}

int RtmpPusher::CreateIOput(std::string inputUrl, std::string outUrl)
{
	DestroyIOput();//reset

	int ret = 0;
	BOOL bError = FALSE;

	do{

		/* --------------------------------- output 0 ---------------------------------- */
		ret = avformat_alloc_output_context2(&outputContext, nullptr, "flv", outUrl.c_str());
		if (ret < 0)
		{
			bError = TRUE;
			break;
		}

		ret = avio_open2(&outputContext->pb, outUrl.c_str(), AVIO_FLAG_READ_WRITE, nullptr, nullptr);
		if (ret < 0)
		{
			bError = TRUE;
			TRACE("@@@@@@@@@@@@@@ Connect to rtmp server fail..............\r\n");
			break;
		}

		/* --------------------------------- input ---------------------------------- */
		inputContext = avformat_alloc_context();
		AVDictionary* options = nullptr;
		av_dict_set(&options, "rtsp_transport", "udp", 0);
		int ret = avformat_open_input(&inputContext, inputUrl.c_str(), nullptr, &options);
		if (ret < 0)
		{
			bError = TRUE;
			TRACE("@@@@@@@@@@@@@@ Connect to rtsp media fail..............\r\n");
			break;
		}
		ret = avformat_find_stream_info(inputContext, nullptr);
		if (ret < 0)
		{
			bError = TRUE;
			break;
		}

		/* --------------------------------- output 1 ---------------------------------- */
		for (int i = 0; i < inputContext->nb_streams; i++)
		{
			AVStream * stream = avformat_new_stream(outputContext, inputContext->streams[i]->codec->codec);
			ret = avcodec_copy_context(stream->codec, inputContext->streams[i]->codec);
			if (ret < 0)
			{
				bError = TRUE;
				break;
			}
		}

		if (bError)
			break;

		ret = avformat_write_header(outputContext, nullptr);
		if (ret < 0)
		{
			bError = TRUE;
			break;
		}

	} while (0);

	if (bError)
	{
		if (outputContext)
		{
			for (int i = 0; i < outputContext->nb_streams; i++)
			{
				avcodec_close(outputContext->streams[i]->codec);
			}
			avformat_close_input(&outputContext);
			outputContext = nullptr;
		}

		if (inputContext)
		{
			avformat_free_context(inputContext);
			inputContext = nullptr;
		}

	}



	return ret;
}

int RtmpPusher::DestroyIOput()
{
	if (outputContext)
	{
		for (int i = 0; i < outputContext->nb_streams; i++)
		{
			avcodec_close(outputContext->streams[i]->codec);
		}
		avformat_close_input(&outputContext);
		outputContext = nullptr;
	}

	if (inputContext)
	{
		avformat_free_context(inputContext);
		inputContext = nullptr;
	}

	return 0;
}

int RtmpPusher::OpenOutput(std::string outUrl)
{
	int ret = 0;

	BOOL bError = FALSE;
	do
	{
		ret = avformat_alloc_output_context2(&outputContext, nullptr, "flv", outUrl.c_str());
		if (ret < 0)
		{
			av_log(NULL, AV_LOG_ERROR, "open output context failed\n");
			bError = TRUE;
			break;
		}

		ret = avio_open2(&outputContext->pb, outUrl.c_str(), AVIO_FLAG_READ_WRITE, nullptr, nullptr);
		if (ret < 0)
		{
			av_log(NULL, AV_LOG_ERROR, "open avio failed");
			bError = TRUE;
			break;
		}

		for (int i = 0; i < inputContext->nb_streams; i++)
		{
			AVStream * stream = avformat_new_stream(outputContext, inputContext->streams[i]->codec->codec);
			ret = avcodec_copy_context(stream->codec, inputContext->streams[i]->codec);
			if (ret < 0)
			{
				av_log(NULL, AV_LOG_ERROR, "copy coddec context failed");
				bError = TRUE;
				break;
			}
		}

		if (bError)
			break;

		ret = avformat_write_header(outputContext, nullptr);
		if (ret < 0)
		{
			av_log(NULL, AV_LOG_ERROR, "format write header failed");
			bError = TRUE;
			break;
		}

		av_log(NULL, AV_LOG_FATAL, " Open output file success %s\n", outUrl.c_str());
		TRACE("@@@@@@@ Connect Rtmp server success........\r\n");
		return ret;
	} while (0);

	if (bError)
	{
		if (outputContext)
		{
			for (int i = 0; i < outputContext->nb_streams; i++)
			{
				avcodec_close(outputContext->streams[i]->codec);
			}
			avformat_close_input(&outputContext);
		}
	}
	
	if (ret <0)
		TRACE("@@@@@@@ Connect Rtmp server fail........\r\n");
	else
		TRACE("@@@@@@@ Connect Rtmp server success........\r\n");

	return ret;
}

int RtmpPusher::OpenOutputEx(std::string outUrl)
{
	// connect rtmp context
	hRtmp_ = srs_rtmp_create(outUrl.c_str());

	if (srs_rtmp_handshake(hRtmp_) != 0) {
		TRACE("simple handshake failed.\r\n");
		srs_rtmp_destroy(hRtmp_);
		hRtmp_ = NULL;
		return -1;
	}
	TRACE("simple handshake success\r\n");

	if (srs_rtmp_connect_app(hRtmp_) != 0) {
		TRACE("connect vhost/app failed.\r\n");
		srs_rtmp_destroy(hRtmp_);
		hRtmp_ = NULL;
		return -1;
	}
	TRACE("connect vhost/app success\r\n");

	if (srs_rtmp_publish_stream(hRtmp_) != 0) {
		TRACE("publish stream failed.\r\n");
		srs_rtmp_destroy(hRtmp_);
		hRtmp_ = NULL;
		return -1;
	}
	TRACE("publish stream success\r\n");


	return 0;
}


void av_packet_rescale_ts(AVPacket *pkt, AVRational src_tb, AVRational dst_tb)
{
	if (pkt->pts != AV_NOPTS_VALUE)
		pkt->pts = av_rescale_q(pkt->pts, src_tb, dst_tb);
	if (pkt->dts != AV_NOPTS_VALUE)
		pkt->dts = av_rescale_q(pkt->dts, src_tb, dst_tb);
	if (pkt->duration > 0)
		pkt->duration = av_rescale_q(pkt->duration, src_tb, dst_tb);
}



RtmpPusher::RtmpPusher(std::string cmsid, std::string pid, std::string key, std::string rtspuri, std::string rtmpuri)
{
	cmsid_ = cmsid;
	pid_ = pid;
	key_ = key;
	rtspuri_ = rtspuri.insert(7,"admin:1234567890p@");
	rtmpuri_ = rtmpuri;

	hEventExit_ = NULL;
	hThreadWork_ = NULL;
	bExit_ = TRUE;

	inputContext = nullptr;
	outputContext = nullptr;

	hRtmp_ = NULL;

}

RtmpPusher::~RtmpPusher()
{

}

int  RtmpPusher::Start()
{
	if (!bExit_) return 0;

	token_ = GeneralToken();


	char szUrl[1024] = {0};
	sprintf(szUrl, "%s%s?cmsid=%s&devid=%s&key=%s/%s", rtmpuri_.c_str(), cmsid_.c_str(), cmsid_.c_str(), pid_.c_str(), key_.c_str(),token_.c_str());
	//sprintf(szUrl, "%s%s?/%s/key=atb123213213", rtmpuri_.c_str(), cmsid_.c_str(), token_.c_str());


	pushuri_ = szUrl;

	Init();
	

	bExit_ = FALSE;
	hEventExit_ = CreateEvent(NULL, FALSE, FALSE, NULL);
	hThreadWork_ = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)WorkThread, this, NULL, NULL);
	
	TRACE("Start pusing......................\r\n");

	return 0;
}

void RtmpPusher::Stop()
{
	if (bExit_) return;

	bExit_ = TRUE;
	if (WaitForSingleObject(hEventExit_, 6000) != WAIT_OBJECT_0)
	{
		TerminateThread(hThreadWork_, 0);
	}
	CloseHandle(hEventExit_);
	CloseHandle(hThreadWork_);
	hEventExit_ = NULL;
	hThreadWork_ = NULL;

	DestroyIOput();

	TRACE("Stop pusing......................\r\n");
}


void RtmpPusher::Init()
{
	av_register_all();
	avfilter_register_all();
	avformat_network_init();
	av_log_set_level(AV_LOG_ERROR);
}

int  RtmpPusher::WorkThread(LPVOID lpParam)
{
	RtmpPusher *pthis = (RtmpPusher *)lpParam;

	pthis->WorkThreadFuncEx();
	return 0;
}


int RtmpPusher::WritePacket(std::shared_ptr<AVPacket> packet)
{
	if (inputContext == nullptr || outputContext == nullptr) return -1;
	auto inputStream = inputContext->streams[packet->stream_index];
	auto outputStream = outputContext->streams[packet->stream_index];
	av_packet_rescale_ts(packet.get(), inputStream->time_base, outputStream->time_base);
	return av_interleaved_write_frame(outputContext, packet.get());
}

int RtmpPusher::WritePacketEx(std::shared_ptr<AVPacket> packet)
{
	if (hRtmp_ == NULL) return -1;

	int dts = packet->dts;
	int pts = packet->pts;

	// send out the h264 packet over RTMP

	int ret = srs_h264_write_raw_frames(hRtmp_, (char *)packet->data, packet->size, dts, pts);
	if (ret != 0) {
		if (srs_h264_is_dvbsp_error(ret)) {
			TRACE("ignore drop video error, code=%d\r\n", ret);
		}
		else if (srs_h264_is_duplicated_sps_error(ret)) {
			TRACE("ignore duplicated sps, code=%d\r\n", ret);
		}
		else if (srs_h264_is_duplicated_pps_error(ret)) {
			TRACE("ignore duplicated pps, code=%d\r\n", ret);
		}
		else {
			TRACE("send h264 raw data failed. ret=%d\r\n", ret);
			srs_rtmp_destroy(hRtmp_);
			hRtmp_ = NULL;
			return -1;
		}
	}

	//TRACE("h264 raw data completed\r\n");

	return 0;
}

std::shared_ptr<AVPacket> RtmpPusher::ReadPacketFromSource()
{
	std::shared_ptr<AVPacket> packet(static_cast<AVPacket*>(av_malloc(sizeof(AVPacket))), [&](AVPacket *p) { av_packet_free(&p); av_freep(&p); });
	av_init_packet(packet.get());
	int ret = av_read_frame(inputContext, packet.get());
	if (ret >= 0)
	{
		int videoindex = -1;
		for (int i = 0; i<inputContext->nb_streams; i++) {
			if (inputContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){

		//		TRACE(">>>>>>>>>>> VIDEO INDEX : %d\r\n",i);

				videoindex = i;
			}
		}

		if (videoindex != -1 && packet->stream_index == videoindex)
		{
			return packet;
		}
		else
			return nullptr;
	

	}
	else
	{
		return nullptr;
	}
}

void RtmpPusher::WorkThreadFunc()
{
	
	int ret = 0;
	bool bConnected = false;

	while (!bExit_)
	{
		if (!bConnected)
		{
			int ret = OpenInput(rtspuri_.c_str());
			//int ret = OpenOutputEx(pushuri_.c_str());
			if (ret >= 0)
			{
				ret = OpenOutput(pushuri_.c_str());
				//ret = OpenOutputEx(pushuri_.c_str());
				//ret = OpenInput(rtspuri_.c_str());
			}
			if (ret < 0)
			{
				Sleep(3000);
				continue;
			}

			TRACE("############## push pull success.............\r\n");
			bConnected = true;
		}


		auto packet = ReadPacketFromSource();
		if (packet)
		{
			ret = WritePacket(packet);
			//ret = WritePacketEx(packet);
		}
		else
		{
			
		}


	}

	SetEvent(hEventExit_);
}

void RtmpPusher::WorkThreadFuncEx()
{

	int ret = 0;
	bool bConnected = false;

	int flag = 0;


	int iReadDeadCount = 0;
	int iWriteDeadCount = 0;
	while (!bExit_)
	{
		if (!bConnected)
		{
			ret = CreateIOput(rtspuri_, pushuri_);
			if(ret<0)
			{
				TRACE("@@@@@@@@ create input output channel X X X X X X \r\n");
				Sleep(3000);
				continue;
			}

			TRACE("@@@@@@@@ create input output channel O O O O O O O  \r\n");
			bConnected = true;
		}

		auto packet = ReadPacketFromSource();
		if (packet)
		{
			iReadDeadCount = 0;
			ret = WritePacket(packet);
			if (ret!=0)
			{
				//TRACE("----------- w r i t e --------- e r r --- %d \r\n", iWriteDeadCount);
				if (++iWriteDeadCount>600)
				{
					TRACE("----------w r i t e--------e r r -- r e c r e a t e -- \r\n");
					iWriteDeadCount = 0;
					bConnected = false;
				}
				
			}
			else
			{
				iWriteDeadCount = 0;
			}

			//ret = WritePacketEx(packet);
		}
		else
		{
			//TRACE("- - - - - - - - r e a d - - e r r - - %d \r\n", iReadDeadCount);
			if (++iReadDeadCount>600)
			{
				TRACE("- - - - - - - - r e a d - - e r r - - r e c r e a t e - -\r\n");
				iReadDeadCount = 0;
				bConnected = false;
			}
		}


	}

	SetEvent(hEventExit_);
}