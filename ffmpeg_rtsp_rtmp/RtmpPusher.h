#ifndef rtmp_push_h_h_h____
#define rtmp_push_h_h_h____

#include "../pch.h"

#include "srs_librtmp.h"

class RtmpPusher
{
public:
	RtmpPusher(std::string cmsid, std::string pid, std::string key, std::string rtspuri, std::string rtmpuri);
	~RtmpPusher();

	int  Start();
	void Stop();

	std::string GetPid(){ return pid_; }
	std::string GetToken(){ return token_; }

protected:
	void Init();

	static int __stdcall WorkThread(LPVOID lpParam);
	void WorkThreadFunc();
	void WorkThreadFuncEx();

	int OpenInput(std::string inputUrl);
	int OpenOutput(std::string outUrl);
	int OpenOutputEx(std::string outUrl);

	int CreateIOput(std::string inputUrl, std::string outUrl);
	int DestroyIOput();

	int  WritePacket(std::shared_ptr<AVPacket> packet);
	int  WritePacketEx(std::shared_ptr<AVPacket> packet);
	std::shared_ptr<AVPacket> ReadPacketFromSource();
private:
	std::string pid_;
	std::string rtspuri_;
	std::string cmsid_;
	std::string rtmpuri_;//rtmp://192.168.1.109:1935/
	std::string token_;

	std::string key_;//for check

	std::string pushuri_;

	HANDLE hEventExit_;
	HANDLE hThreadWork_;
	BOOL   bExit_;

	AVFormatContext *inputContext;
	AVFormatContext * outputContext;

	srs_rtmp_t hRtmp_;

};





#endif