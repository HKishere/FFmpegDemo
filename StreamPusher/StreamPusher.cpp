// StreamPusher.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <stdio.h>

#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
//Windows
extern "C"
{
#include "libavformat/avformat.h"
#include "libavutil/mathematics.h"
#include "libavutil/time.h"
};
#else
//Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
#include <libavutil/time.h>
#ifdef __cplusplus
};
#endif
#endif

int main()
{
	AVOutputFormat* oFmt = NULL;
	AVFormatContext* iFmtCtx = NULL, *oFmtCtx = NULL;

	AVPacket* AVpac;
	const char pstrInPutFileName[256] = { 0 };
	const char pstrOutPutFileName[256] = { 0 };
	int ret, i;

	av_register_all();
	if (!avformat_open_input(&iFmtCtx, pstrInPutFileName, NULL, NULL))
	{
		std::cout << "open inputfile faild!" << std::endl;
		return 0;
	}
	if (!avformat_find_stream_info(iFmtCtx, NULL))
	{
		std::cout << "find stream info faild!" << std::endl;
	}
	av_dump_format(iFmtCtx, 0, pstrInPutFileName, false); //打印信息

	avformat_alloc_output_context2(&oFmtCtx, NULL, NULL, pstrOutPutFileName);
	if (!oFmtCtx)
	{
		std::cout << "create output Context faild!" << std::endl;
	}
	oFmt = oFmtCtx->oformat;

	for (i = 0; i < iFmtCtx->nb_streams; i++) 
	{
		//根据输入流创建输出流（Create output AVStream according to input AVStream）
		AVStream *in_stream = iFmtCtx->streams[i];
		AVStream *out_stream = NULL;
		if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			out_stream = avformat_new_stream(oFmtCtx, in_stream->codec->codec);
			if (!out_stream)
			{
				std::cout << "allocate output stream faild!" << std::endl;
			}
			if (!avcodec_copy_context(out_stream->codec, in_stream->codec))
			{
				std::cout << "copy context faild!" << std::endl;
			}
			out_stream->codec->codec_tag = 0;
			if (oFmtCtx->oformat->flags & AVFMT_GLOBALHEADER)
				out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		}
	}
	av_dump_format(oFmtCtx, 0, pstrOutPutFileName, 1);


	if (!(oFmt->flags & AVFMT_NOFILE)) 
	{
		ret = avio_open(&oFmtCtx->pb, pstrOutPutFileName, AVIO_FLAG_WRITE);
		if (ret < 0)
		{
			std::cout << "open URL faild!" << std::endl;
		}
	}
	ret = avformat_write_header(oFmtCtx, NULL);
	if (ret < 0) {
		std::cout << "faild when opening output URL !" << std::endl;
	}
	int64_t start_time = av_gettime();
	while (1)
	{
	}

}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
