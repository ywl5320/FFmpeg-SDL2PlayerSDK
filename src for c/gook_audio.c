#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "SDL.h"
#include "SDL_thread.h"

#include <android/log.h>
#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"ywl5320",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"ywl5320",FORMAT,##__VA_ARGS__);

#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/mathematics.h"
#include "libavutil/samplefmt.h"

#define SDL_AUDIO_BUFFER_SIZE 1024
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000

typedef struct PacketQueue
{
	AVPacketList *first_pkt, *last_pkt;
	int nb_packets;
	int size;
	SDL_mutex *mutex;
	SDL_cond *cond;
}PacketQueue;

PacketQueue audioq;
int quit = 0;

void packet_queue_init(PacketQueue *q) {
	memset(q, 0, sizeof(PacketQueue));
	q->mutex = SDL_CreateMutex();
	q->cond = SDL_CreateCond();
}


int packet_queue_put(PacketQueue *q, AVPacket *pkt) {

	AVPacketList *pkt1;
	if (av_dup_packet(pkt) < 0) {
		return -1;
	}
	pkt1 = av_malloc(sizeof(AVPacketList));
	if (!pkt1)
		return -1;
	pkt1->pkt = *pkt;
	pkt1->next = NULL;

	SDL_LockMutex(q->mutex);

	if (!q->last_pkt)
		q->first_pkt = pkt1;
	else
		q->last_pkt->next = pkt1;
	q->last_pkt = pkt1;
	q->nb_packets++;
	q->size += pkt1->pkt.size;
	SDL_CondSignal(q->cond);

	SDL_UnlockMutex(q->mutex);
	return 0;
}

static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block) {
	AVPacketList *pkt1;
	int ret;

	SDL_LockMutex(q->mutex);

	for (;;) {

		if (quit) {
			ret = -1;
			break;
		}

		pkt1 = q->first_pkt;
		if (pkt1) {
			q->first_pkt = pkt1->next;
			if (!q->first_pkt)
				q->last_pkt = NULL;
			q->nb_packets--;
			q->size -= pkt1->pkt.size;
			*pkt = pkt1->pkt;
			av_free(pkt1);
			ret = 1;
			break;
		} else if (!block) {
			ret = 0;
			break;
		} else {
			SDL_CondWait(q->cond, q->mutex);
		}
	}
	SDL_UnlockMutex(q->mutex);
	return ret;
}

int getQueueSize(PacketQueue *q)
{
	return q->nb_packets;
}

int audio_decode_frame(AVCodecContext *aCodecCtx, uint8_t *audio_buf, int buf_size) {

	AVFrame *frame = av_frame_alloc();
	int data_size = 0;
	AVPacket pkt;
	int got_frame_ptr;

	SwrContext *swr_ctx;

	if (quit)
		return -1;
	if (packet_queue_get(&audioq, &pkt, 1) < 0)
		return -1;
	int ret = avcodec_decode_audio4(aCodecCtx, frame, &got_frame_ptr, &pkt);
	//int ret = avcodec_send_packet(aCodecCtx, &pkt);
	if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
		return -1;

	//ret = avcodec_receive_frame(aCodecCtx, frame);
	//if (ret < 0 && ret != AVERROR_EOF)
	//	return -1;

	// 设置通道数或channel_layout
	if (frame->channels > 0 && frame->channel_layout == 0)
		frame->channel_layout = av_get_default_channel_layout(frame->channels);
	else if (frame->channels == 0 && frame->channel_layout > 0)
		frame->channels = av_get_channel_layout_nb_channels(frame->channel_layout);

	enum AVSampleFormat dst_format = AV_SAMPLE_FMT_S16;//av_get_packed_sample_fmt((AVSampleFormat)frame->format);

	//重采样为立体声
	Uint64 dst_layout = AV_CH_LAYOUT_STEREO;
	// 设置转换参数
	swr_ctx = swr_alloc_set_opts(NULL, dst_layout, dst_format, frame->sample_rate,
		frame->channel_layout, (enum AVSampleFormat)frame->format, frame->sample_rate, 0, NULL);
	if (!swr_ctx || swr_init(swr_ctx) < 0)
		return -1;

	// 计算转换后的sample个数 a * b / c
	int dst_nb_samples = av_rescale_rnd(swr_get_delay(swr_ctx, frame->sample_rate) + frame->nb_samples, frame->sample_rate, frame->sample_rate, 1);
	// 转换，返回值为转换后的sample个数
	int nb = swr_convert(swr_ctx, &audio_buf, dst_nb_samples, (const uint8_t**)frame->data, frame->nb_samples);

	//根据布局获取声道数
	int out_channels = av_get_channel_layout_nb_channels(dst_layout);
	data_size = out_channels * nb * av_get_bytes_per_sample(dst_format);

	av_frame_free(&frame);
	swr_free(&swr_ctx);
	return data_size;
}


void audio_callback(void *userdata, Uint8 *stream, int len) {

	AVCodecContext *aCodecCtx = (AVCodecContext *)userdata;

	int len1, audio_size;

	static uint8_t audio_buff[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];
	static unsigned int audio_buf_size = 0;
	static unsigned int audio_buf_index = 0;

	SDL_memset(stream, 0, len);
	if (getQueueSize(&audioq) > 0)
	{
		LOGI("pkt nums: %d    queue size: %d\n", audioq.nb_packets, audioq.size);
		while (len > 0)// 想设备发送长度为len的数据
		{
			if (audio_buf_index >= audio_buf_size) // 缓冲区中无数据
			{
				// 从packet中解码数据
				audio_size = audio_decode_frame(aCodecCtx, audio_buff, sizeof(audio_buff));
				if (audio_size < 0) // 没有解码到数据或出错，填充0
				{
					audio_buf_size = 0;
					memset(audio_buff, 0, audio_buf_size);
				}
				else
					audio_buf_size = audio_size;

				audio_buf_index = 0;
			}
			len1 = audio_buf_size - audio_buf_index; // 缓冲区中剩下的数据长度
			if (len1 > len) // 向设备发送的数据长度为len
				len1 = len;

			SDL_MixAudio(stream, audio_buff + audio_buf_index, len, SDL_MIX_MAXVOLUME);

			len -= len1;
			stream += len1;
			audio_buf_index += len1;
		}
	}
	else
	{
		LOGI("pkt nums: %d    queue size: %d\n", audioq.nb_packets, audioq.size);
		LOGI("play complete");
		SDL_CloseAudio();
		SDL_Quit();
	}
}

int main(int argc, char* args[])
{

	LOGI(".............come from main............");
	LOGI("input url: %s", args[1]);
	av_register_all();
	avformat_network_init();

	AVFormatContext *pFormatCtx;
	if (avformat_open_input(&pFormatCtx, args[1], NULL, NULL) != 0)
		return -1;

	if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
		return -1;

//	av_dump_format(pFormatCtx, 0, args[1], 0);

	int audioStream = -1;
	int i = 0;
	for (; i < pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			audioStream = i;
			break;
		}
	}

	if (audioStream == -1)
		return -1;

	AVCodecContext* pCodecCtxOrg;
	AVCodecContext* pCodecCtx;

	AVCodec* pCodec;

	pCodecCtxOrg = pFormatCtx->streams[audioStream]->codec; // codec context

	// 找到audio stream的 decoder
	pCodec = avcodec_find_decoder(pCodecCtxOrg->codec_id);

	if (!pCodec)
	{
		LOGE("Unsupported codec!")
		return -1;
	}

	// 不直接使用从AVFormatContext得到的CodecContext，要复制一个
	pCodecCtx = avcodec_alloc_context3(pCodec);
	if (avcodec_copy_context(pCodecCtx, pCodecCtxOrg) != 0)
	{
		LOGE("Could not copy codec context!");
		return -1;
	}

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
	// Set audio settings from codec info
	SDL_AudioSpec wanted_spec, spec;
	wanted_spec.freq = pCodecCtx->sample_rate;
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.channels = pCodecCtx->channels;
	wanted_spec.silence = 0;
	wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;
	wanted_spec.callback = audio_callback;
	wanted_spec.userdata = pCodecCtx;

	if(SDL_OpenAudio(&wanted_spec, &spec) < 0)
	{
		LOGE("Open audio failed:");
		return -1;
	}

	LOGI("come here...");

	avcodec_open2(pCodecCtx, pCodec, NULL);

	SDL_PauseAudio(0);

	AVPacket packet;
	while (1)
	{
		if (getQueueSize(&audioq) < 50)
		{
			int ret = av_read_frame(pFormatCtx, &packet);
			if (ret >= 0)
			{
				if (packet.stream_index == audioStream)
					packet_queue_put(&audioq, &packet);
				else
					av_packet_unref(&packet);
			}
		}
	}

	avformat_close_input(&pFormatCtx);
	return 0;
}









