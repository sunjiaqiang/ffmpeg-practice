#include <stdio.h>

#include<libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include<libavformat/avformat.h>

void saveFrame(AVFrame *pframe, int width, int height, int iframe) {

    FILE *fp = NULL;
    int y;
    char filename[60];
    sprintf(filename, "../img/frame%d.ppm", iframe);

    fp = fopen(filename, "w+");
    fprintf(fp, "P6\n%d %d\n255\n", width, height); // header
    for (y=0; y<height; y++)
        fwrite(pframe->data[0]+y*pframe->linesize[0], 1, width*3, fp);
    fclose(fp);
}

int main() {
    AVFormatContext	*pFormatCtx;
    int i, videoindex;
    AVCodecContext  *pCodecCtx;
    AVCodec *pCodec;
    AVFrame *pFrame,*pFrameRgb;
    AVPacket *packet;
    int res, get_picture_ptr;
    char filePath[] = "../video/test.flv";

    av_register_all();
    avformat_network_init();
    pFormatCtx = avformat_alloc_context();

    if (avformat_open_input(&pFormatCtx, filePath, NULL, NULL) != 0) {
        printf("Couldn't open input stream.\n");
        return -1;
    }

    if (avformat_find_stream_info(pFormatCtx, NULL)<0){
        printf("Couldn't find stream information.\n");
        return -1;
    }

    videoindex = -1;
    for (i = 0; i < pFormatCtx -> nb_streams; i++)
        if (pFormatCtx -> streams[i] -> codec -> codec_type == AVMEDIA_TYPE_VIDEO){
            videoindex = i;
            break;
        }
    if (videoindex == -1) {
        printf("Didn't find a video stream.\n");
        return -1;
    }

    pCodecCtx = pFormatCtx -> streams[videoindex] -> codec;   // 获取视频 codec
    pCodec = avcodec_find_decoder(pCodecCtx -> codec_id);   //

    struct SwsContext *swsCtx = sws_getContext(
        pCodecCtx->width,
        pCodecCtx->height,
        pCodecCtx->pix_fmt,
        pCodecCtx->width,
        pCodecCtx->height,
        AV_PIX_FMT_BGR24,
        SWS_BILINEAR,
        NULL,
        NULL,
        NULL
    );

    if ( pCodec == NULL) {
        printf("Codec not found.\n");
        return -1;
    }
    if( avcodec_open2(pCodecCtx, pCodec,NULL) < 0) {
        printf("Could not open codec.\n");
        return -1;
    }

    pFrame = av_frame_alloc();
    pFrameRgb = av_frame_alloc();

    int num_bytes = avpicture_get_size(AV_PIX_FMT_RGB24,
                                       pCodecCtx->width, pCodecCtx->height);
    uint8_t *buffer = av_malloc(num_bytes * sizeof(uint8_t));

    packet = (AVPacket *) av_malloc(sizeof(AVPacket));

    avpicture_fill(
        (AVPicture *)pFrameRgb,
        buffer,
        AV_PIX_FMT_RGB24,
        pCodecCtx->width,
        pCodecCtx->height
    );

    av_dump_format(pFormatCtx, 0, filePath, 0);

    int index = 0;
    while (av_read_frame(pFormatCtx, packet) >= 0) {
        if (packet -> stream_index == videoindex){
            res = avcodec_decode_video2(pCodecCtx, pFrame, &get_picture_ptr, packet);
            if (res < 0) {
                printf("decode error.\n");
                return -1;
            }

            if (get_picture_ptr != 0) {
                sws_scale(swsCtx, pFrame->data, pFrame->linesize,
                          0, pCodecCtx->height, pFrameRgb->data, pFrameRgb->linesize);

                if (++index < 10) {
                    saveFrame(pFrameRgb, pCodecCtx->width, pCodecCtx->height, index);
                } else {
                    av_free_packet(packet);
                    break;
                }

                printf("picture_ptr: %d \n", get_picture_ptr);
            }
        }
        av_free_packet(packet);
    }

    av_free(buffer);
    av_free(pFrameRgb);
    av_free(pFrame);

    avcodec_close(pCodecCtx);

    return 0;
}
