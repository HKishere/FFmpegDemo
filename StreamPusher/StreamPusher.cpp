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
#include <libswscale/swscale.h>

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
	AVCodecContext * iCodecCtx = NULL;

	AVPacket AVpac;
	const char pstrInPutFileName[256] = "C:\\Users\\HZK\\Desktop\\无人机\\视频\\VID_20210908_133050.mp4";
	//const char pstrInPutFileName[256] = "rtsp://admin:abcd1234@192.168.2.146:554/h264/ch1/main/av_stream";
	const char pstrOutPutFileName[256] = "rtmp://visione.kishere.cn:3519/live/79uJfl07R?sign=nru1BlA7Rz";
	int ret, i;
	int videoindex;

	//av_register_all();
	avformat_network_init();
	if (avformat_open_input(&iFmtCtx, pstrInPutFileName, NULL, NULL))
	{
		std::cout << "open inputfile faild!" << std::endl;
		system("pasue");
		return 0;
	}
	if (avformat_find_stream_info(iFmtCtx, NULL) < 0)
	{
		std::cout << "find stream info faild!" << std::endl;
		system("pasue");
		return 0;
	}
	av_dump_format(iFmtCtx, 0, pstrInPutFileName, false); //打印信息

	avformat_alloc_output_context2(&oFmtCtx, NULL, "flv", NULL);
	if (!oFmtCtx)
	{
		std::cout << "create output Context faild!" << std::endl;
		system("pasue");
		return 0;
	}
	oFmt = oFmtCtx->oformat;

	for (i = 0; i < iFmtCtx->nb_streams; i++) 
	{
		//根据输入流创建输出流（Create output AVStream according to input AVStream）
		AVStream *in_stream = iFmtCtx->streams[i];
		AVStream *out_stream = NULL;
		//if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		videoindex = i;
		out_stream = avformat_new_stream(oFmtCtx, avcodec_find_decoder(in_stream->codecpar->codec_id));
		if (!out_stream)
		{
			std::cout << "allocate output stream faild!" << std::endl;
			system("pasue");
			return 0;
		}
		iCodecCtx = avcodec_alloc_context3(avcodec_find_decoder(in_stream->codecpar->codec_id));
		avcodec_parameters_to_context(iCodecCtx, in_stream->codecpar);
		if (avcodec_parameters_from_context(out_stream->codecpar, iCodecCtx) < 0)
		{
			std::cout << "copy context faild!" << std::endl;
			system("pasue");
			return 0;
		}
		//if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		//{
		//	out_stream->codecpar->bit_rate = 256;
		//}
		out_stream->codecpar->codec_tag = 0;
		if (oFmtCtx->oformat->flags & AVFMT_GLOBALHEADER)
		{
			avcodec_parameters_to_context(iCodecCtx, out_stream->codecpar);
			iCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
			avcodec_parameters_from_context(out_stream->codecpar, iCodecCtx);
		}
	}
	av_dump_format(oFmtCtx, 0, pstrOutPutFileName, 1);
	oFmtCtx->bit_rate = 256;

	if (!(oFmt->flags & AVFMT_NOFILE)) 
	{
		ret = avio_open(&oFmtCtx->pb, pstrOutPutFileName, AVIO_FLAG_WRITE);
		if (ret < 0)
		{
			std::cout << "open URL faild!" << std::endl;
			system("pasue");
			return 0;
		}
	}
	ret = avformat_write_header(oFmtCtx, NULL);
	if (ret < 0) {
		std::cout << "faild when opening output URL !" << std::endl;
		system("pasue");
		return 0;
	}

	SwsContext *pSwsCtx = sws_getContext(iCodecCtx->width, iCodecCtx->height, iCodecCtx->pix_fmt, 1280, 720,
		AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);


	AVCodec* pCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
	AVCodecContext * pOutCodecCtx = avcodec_alloc_context3(pCodec);

	int64_t start_time = av_gettime();
	int nFreamIndex = 0;
	while (1)
	{
		if (av_read_frame(iFmtCtx, &AVpac))
		{
			break;
		}
		if (AVpac.pts == AV_NOPTS_VALUE)
		{
			AVRational time_base1 = iFmtCtx->streams[videoindex]->time_base;
			int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(iFmtCtx->streams[videoindex]->r_frame_rate);
			AVpac.pts = (double)(nFreamIndex * calc_duration) / (double)(av_q2d(time_base1)*AV_TIME_BASE);
			AVpac.dts = AVpac.pts;
			AVpac.duration = (double)calc_duration / (double)(av_q2d(time_base1)*AV_TIME_BASE);
		}

		//Important:Delay
		if (AVpac.stream_index == videoindex) {
			AVRational time_base = iFmtCtx->streams[videoindex]->time_base;
			AVRational time_base_q = { 1, AV_TIME_BASE };
			int64_t pts_time = av_rescale_q(AVpac.dts, time_base, time_base_q);
			int64_t now_time = av_gettime() - start_time;
			if (pts_time > now_time)
				av_usleep(pts_time - now_time);
		}
		AVStream *iStream = iFmtCtx->streams[AVpac.stream_index];
		AVStream *oStream = oFmtCtx->streams[AVpac.stream_index];

		AVpac.pts = av_rescale_q_rnd(AVpac.pts, iStream->time_base, oStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		AVpac.dts = av_rescale_q_rnd(AVpac.dts, iStream->time_base, oStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		AVpac.duration = av_rescale_q(AVpac.duration, iStream->time_base, oStream->time_base);
		AVpac.pos = -1;

		if (AVpac.stream_index == videoindex) {
			printf("Send %8d video frames to output URL\n", nFreamIndex);
			nFreamIndex++;
		}
		AVFrame* pFrame = NULL;
		AVFrame* pFrameOut = NULL;
		pFrameOut = av_frame_alloc();
		pFrame = av_frame_alloc();

		//av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer,
		//	p_ffmpeg_param->pCodecCtx->pix_fmt,
		//	p_ffmpeg_param->pCodecCtx->width, p_ffmpeg_param->pCodecCtx->height, 1);

		int temp_ret = avcodec_receive_frame(avcodec_alloc_context3(avcodec_find_decoder(iStream->codecpar->codec_id)), pFrame);
		sws_scale(pSwsCtx, (const unsigned char* const*)&pFrame->data,
			pFrame->linesize, 0, iCodecCtx->height, pFrameOut->data,
			pFrameOut->linesize);
		avcodec_send_frame(pOutCodecCtx, pFrame);
		avcodec_receive_packet(pOutCodecCtx, &AVpac);

		ret = av_interleaved_write_frame(oFmtCtx, &AVpac);

		if (ret < 0)
		{
			std::cout << "Error muxing packet!" << std::endl;
			system("pasue");
			return 0;
		}
		av_packet_unref(&AVpac);
	}
	av_write_trailer(oFmtCtx);

	avformat_close_input(&iFmtCtx);
	/* close output */
	if (oFmtCtx && !(oFmt->flags & AVFMT_NOFILE))
		avio_close(oFmtCtx->pb);
	avformat_free_context(oFmtCtx);
	if (ret < 0 && ret != AVERROR_EOF) {
		printf("Error occurred.\n");
		system("pasue");
		return -1;
	}
	system("pasue");
	return 0;
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
