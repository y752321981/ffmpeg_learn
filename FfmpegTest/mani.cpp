#include <iostream>
using namespace std;
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include <SDL.h>
}
int main(int argc, char* argv[]) {

	avformat_network_init();
	AVFormatContext* pFormatCtx = NULL;
	const char* filename = "C:\\Users\\75232\\Desktop\\test.mp4";
	if (avformat_open_input(&pFormatCtx, filename, NULL, NULL) != 0)
	{
		cout << "打开视频文件失败" << endl;
		exit(1);
	}
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
	{
		cout << "获取流失败" << endl;
		exit(2);
	}
	av_dump_format(pFormatCtx, 0, NULL, 0);
	AVCodecParameters* pCodecParam = NULL;
	AVCodecContext* pCodecCtx = NULL;
	int i, videoStream = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codecpar->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO)
		{
			videoStream = i;
			break;
		}
	}
	if (videoStream == -1)
	{
		cout << "获取视频流失败" << endl;
		exit(3);
	}
	pCodecParam = pFormatCtx->streams[videoStream]->codecpar;
	const AVCodec* pCodec = avcodec_find_decoder(pCodecParam->codec_id);
	
	if (pCodec == NULL)
	{
		cout << "找不到解码器" << endl;
		exit(4);
	}
	pCodecCtx = avcodec_alloc_context3(pCodec);
	avcodec_parameters_to_context(pCodecCtx, pCodecParam);
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
	{
		cout << "打开流失败" << endl;
		exit(5);
	}
	SwsContext *img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
	int ret;
	int rateNum = 0;
	AVFrame *pFrame = av_frame_alloc();
	AVFrame *pFrameYUV = av_frame_alloc();
	av_image_alloc(pFrameYUV->data,
		pFrameYUV->linesize,
		pCodecCtx->coded_width,
		pCodecCtx->coded_height,
		AV_PIX_FMT_YUV420P,
		1);
	AVPacket* packet = (AVPacket*)av_malloc(sizeof(AVPacket));
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window = SDL_CreateWindow("SDL Test", SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED, pCodecCtx->coded_width, pCodecCtx->coded_height,
		SDL_WINDOW_SHOWN);
	SDL_Renderer * render = SDL_CreateRenderer(window, -1, 0);
	SDL_Texture *texture = SDL_CreateTexture(render, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, pCodecCtx->width, pCodecCtx->height);

	while (av_read_frame(pFormatCtx, packet) >= 0) {
		if (packet->stream_index == videoStream) {
			ret = avcodec_send_packet(pCodecCtx, packet);
			if (ret < 0) 
			{ 
				cout << "播放错误" << endl;
			}
			while (ret >= 0) {
				ret = avcodec_receive_frame(pCodecCtx, pFrame);
				if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
					break;
				}
				else if (ret < 0) {
					break;
				}
				if (ret >= 0) {
					//调用雷霄骅示例程序中的SDL绘制处理代码即可
					sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
					SDL_UpdateTexture(texture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0]);
					SDL_RenderClear(render);
					SDL_RenderCopy(render, texture, NULL, NULL);
					SDL_RenderPresent(render);
					SDL_Delay(80);
				}
				
			} 
		}
	}
	av_packet_free(&packet);
	SDL_Delay(3000); // 等待 3 秒
	SDL_DestroyWindow(window);
	SDL_Quit();


	if (pFormatCtx) {
		avformat_close_input(&pFormatCtx);
	}
	
	system("pause");
	return 0;
}