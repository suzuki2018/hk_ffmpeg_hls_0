#include "ffmpeg.h"

int main(int argc ,char ** argv)
{
	av_register_all();
	avformat_network_init();

	init_demux(INPUTURL,&icodec);
	printf("--------�������п�ʼ----------\n");
	//////////////////////////////////////////////////////////////////////////
	slice_up();
	//////////////////////////////////////////////////////////////////////////
	uinit_demux();
	printf("--------�������н���----------\n");
	printf("-------�밴������˳�---------\n");
	return getchar();
}