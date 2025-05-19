#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>

#include <SDL2/SDL.h>

#include <stdio.h>

SDL_Renderer *renderer;
SDL_Texture *texture;
SDL_Rect r;

void pgm_save(unsigned char *buf, int wrap, int xsize, int ysize, FILE *f)
{
	// write header
	fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);
	// loop until all rows are written to file
	for (int i = 0; i < ysize; i++)
		fwrite(buf + i * wrap, 1, xsize, f);
}

void display_frame(AVFrame *frame, AVCodecContext *dec_ctx)
{
	SDL_UpdateYUVTexture(texture, &r, frame->data[0], frame->linesize[0], frame->data[1], frame->linesize[1], frame->data[2], frame->linesize[2]);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

int init_sdl(AVCodecContext *codec_ctx)
{
	SDL_Window *window = NULL;
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		fprintf(stderr, "Could not init sdl %s\n", SDL_GetError());
		return -1;
	}

	window = SDL_CreateWindow("Preview", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, codec_ctx->width, codec_ctx->height, 0);
	if (!window)
	{
		fprintf(stderr, "Could not create sdl window\n");
		return -1;
	}

	r.x = 0;
	r.y = 0;
	r.w = codec_ctx->width;
	r.h = codec_ctx->height;

	renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer)
	{
		fprintf(stderr, "Could not create sdl renderer\n");
		return -1;
	}

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING, codec_ctx->width, codec_ctx->height);
	if (!texture)
	{
		fprintf(stderr, "Could not create sdl texture\n");
		return -1;
	}

	return 0;
}

void decode(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt, FILE *f)
{
	int ret;

	ret = avcodec_send_packet(dec_ctx, pkt);
	if (ret < 0)
	{
		if (AVERROR(EAGAIN) == ret || AVERROR(EOF) == ret)
			return;
		av_log(NULL, AV_LOG_ERROR, "Error sending a packet for decoding(%d): %s\n", ret, av_err2str(ret));
		exit(1);
	}
	while (ret >= 0)
	{
		ret = avcodec_receive_frame(dec_ctx, frame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			break;
		else if (ret < 0)
		{
			av_log(NULL, AV_LOG_ERROR, "Error during decoding\n");
			exit(1);
		}
		av_log(NULL, AV_LOG_INFO, "Saving frame %3lld\n", dec_ctx->frame_num);
		display_frame(frame, dec_ctx);

		pgm_save(frame->data[0], frame->linesize[0], frame->width, frame->height, f);
	}
}

int main()
{
	AVFormatContext *fmt_ctx = NULL;
	AVCodecContext *codec_ctx = NULL;
	const AVCodec *codec = NULL;
	int ret = 0;
	char *infilename = "/Users/pengshouhua/project/mujs/note/ffmpeg/bbb_sunflower_1080p_30fps_normal.mp4";
	char *outfilename = "/Users/pengshouhua/project/mujs/note/ffmpeg/bbb.yuv";
	// char *infilename = "bbb_sunflower_1080p_30fps_normal.mp4";
	// char *outfilename = "bbb.yuv";
	int video_stream_index = -1;

	FILE *fin = NULL;
	FILE *fout = NULL;
	AVFrame *frame = NULL;
	AVPacket *pkt = NULL;

	if ((ret = avformat_open_input(&fmt_ctx, infilename, NULL, NULL)) < 0)
	{
		av_log(NULL, AV_LOG_ERROR, "Cannot open avfromat input\n");
		goto end;
	}

	if ((ret = avformat_find_stream_info(fmt_ctx, NULL)) < 0)
	{
		av_log(NULL, AV_LOG_ERROR, "Cannot find stream info\n");
		goto end;
	}

	for (int i = 0; i < fmt_ctx->nb_streams; i++)
	{
		if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			video_stream_index = i;
			break;
		}
	}

	if (video_stream_index == -1)
	{
		av_log(NULL, AV_LOG_ERROR, "There is no video stream\n");
		goto end;
	}
	av_dump_format(fmt_ctx, video_stream_index, infilename, 0);

	codec_ctx = avcodec_alloc_context3(NULL);
	if ((ret = avcodec_parameters_to_context(codec_ctx, fmt_ctx->streams[video_stream_index]->codecpar)) < 0)
	{
		av_log(NULL, AV_LOG_ERROR, "Cannot get codec parameters\n");
		goto end;
	}

	codec = avcodec_find_decoder(codec_ctx->codec_id);
	if (codec == NULL)
	{
		av_log(NULL, AV_LOG_ERROR, "No decoder found\n");
		goto end;
	}

	if ((ret = avcodec_open2(codec_ctx, codec, NULL)) < 0)
	{
		av_log(NULL, AV_LOG_ERROR, "Cannot open video decoder\n");
		goto end;
	}
	av_log(NULL, AV_LOG_INFO, "Decoding codec is: %s\n", codec->long_name);

	pkt = av_packet_alloc();
	if (!pkt)
	{
		av_log(NULL, AV_LOG_ERROR, "Cannot init packet\n");
		goto end;
	}

	frame = av_frame_alloc();
	if (!frame)
	{
		av_log(NULL, AV_LOG_ERROR, "Cannot init frame\n");
		goto end;
	}
	fin = fopen(infilename, "rb");
	if (!fin)
	{
		av_log(NULL, AV_LOG_ERROR, "Cannot open input file: %s\n", strerror(errno));
		goto end;
	}

	fout = fopen(outfilename, "w");
	if (!fout)
	{
		av_log(NULL, AV_LOG_ERROR, "Cannot open output file\n");
		goto end;
	}

	if ((ret = init_sdl(codec_ctx)) < 0)
	{
		av_log(NULL, AV_LOG_ERROR, "Init sdl failed\n");
		goto end;
	}

	while (1)
	{
		ret = av_read_frame(fmt_ctx, pkt);
		if (ret < 0)
		{
			// if (AVERROR(EAGAIN) == ret || ret == AVERROR(EOF))
			// 	continue;
			av_log(NULL, AV_LOG_ERROR, "Cannot read frame(%d): %s\n", ret, av_err2str(ret));
			break;
		}
		if (pkt->stream_index == video_stream_index)
		{
			decode(codec_ctx, frame, pkt, fout);
		}
		av_packet_unref(pkt);
	}

end:
	if (frame)
		av_frame_free(&frame);
	if (pkt)
		av_packet_free(&pkt);
	if (codec_ctx)
		avcodec_free_context(&codec_ctx);
	if (fmt_ctx)
		avformat_close_input(&fmt_ctx);
	if (fin)
		fclose(fin);
	if (fout)
		fclose(fout);

	return ret;
}