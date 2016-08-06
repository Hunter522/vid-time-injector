// /*
//  * Copyright (c) 2013 Stefano Sabatini
//  *
//  * Permission is hereby granted, free of charge, to any person obtaining a copy
//  * of this software and associated documentation files (the "Software"), to deal
//  * in the Software without restriction, including without limitation the rights
//  * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  * copies of the Software, and to permit persons to whom the Software is
//  * furnished to do so, subject to the following conditions:
//  *
//  * The above copyright notice and this permission notice shall be included in
//  * all copies or substantial portions of the Software.
//  *
//  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  * THE SOFTWARE.
//  */
// /**
//  * @file
//  * libavformat/libavcodec demuxing and muxing API example.
//  *
//  * Remux streams from one container format to another.
//  * @example remuxing.c
//  */
//  #include <getopt.h>
//  #include <stdio.h>
//  #include <string>
//  #include <iostream>
//  #include <fstream>
//  #include <log4cpp/Category.hh>
//  #include <log4cpp/FileAppender.hh>
//  #include <log4cpp/OstreamAppender.hh>
//
//  extern "C" {
//  #include "libavcodec/avcodec.h"
//  #include "libavformat/avformat.h"
//  #include <libavutil/common.h>
//  #include "libavutil/dict.h"
//  #include <libavutil/imgutils.h>
//  #include <libavutil/samplefmt.h>
//  #include <libavutil/timestamp.h>
//  #include <libavutil/opt.h>
//  #include <libavutil/channel_layout.h>
//  #include <libavutil/mathematics.h>
//  #include <libavfilter/avfilter.h>
//  }
//  #define ts2str(ts) av_ts_make_string(new char[AV_TS_MAX_STRING_SIZE], ts)
//  #define ts2timestr(ts, tb) av_ts_make_time_string(new char[AV_TS_MAX_STRING_SIZE], ts, tb)
//
//
//  static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag) {
//      AVRational* time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;
//
//      char* pts = ts2str(pkt->pts);
//      char* pts_time_base = ts2timestr(pkt->pts, time_base);
//      char* dts = ts2str(pkt->dts);
//      char* dts_time_base = ts2timestr(pkt->dts, time_base);
//      char* duration = ts2str(pkt->duration);
//      char* duration_time_base = ts2timestr(pkt->duration, time_base);
//      printf("%s: pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
//             tag,
//             pts, pts_time_base,
//             dts, dts_time_base,
//             duration, duration_time_base,
//             pkt->stream_index);
//      delete pts;
//      delete pts_time_base;
//      delete dts;
//      delete dts_time_base;
//      delete duration;
//      delete duration_time_base;
//  }
// int main(int argc, char **argv)
// {
//     AVOutputFormat *ofmt = NULL;
//     AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
//     AVPacket pkt;
//     const char *in_filename, *out_filename;
//     int ret, i;
//     if (argc < 3) {
//         printf("usage: %s input output\n"
//                "API example program to remux a media file with libavformat and libavcodec.\n"
//                "The output format is guessed according to the file extension.\n"
//                "\n", argv[0]);
//         return 1;
//     }
//     in_filename  = argv[1];
//     out_filename = argv[2];
//     av_register_all();
//     if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
//         fprintf(stderr, "Could not open input file '%s'", in_filename);
//         goto end;
//     }
//     if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
//         fprintf(stderr, "Failed to retrieve input stream information");
//         goto end;
//     }
//     av_dump_format(ifmt_ctx, 0, in_filename, 0);
//     avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
//     if (!ofmt_ctx) {
//         fprintf(stderr, "Could not create output context\n");
//         ret = AVERROR_UNKNOWN;
//         goto end;
//     }
//     ofmt = ofmt_ctx->oformat;
//     for (i = 0; i < ifmt_ctx->nb_streams; i++) {
//         AVStream *in_stream = ifmt_ctx->streams[i];
//         AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
//         if (!out_stream) {
//             fprintf(stderr, "Failed allocating output stream\n");
//             ret = AVERROR_UNKNOWN;
//             goto end;
//         }
//         ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
//         if (ret < 0) {
//             fprintf(stderr, "Failed to copy context from input to output stream codec context\n");
//             goto end;
//         }
//         out_stream->codec->codec_tag = 0;
//         if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
//             out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
//     }
//     av_dump_format(ofmt_ctx, 0, out_filename, 1);
//     if (!(ofmt->flags & AVFMT_NOFILE)) {
//         ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
//         if (ret < 0) {
//             fprintf(stderr, "Could not open output file '%s'", out_filename);
//             goto end;
//         }
//     }
//     ret = avformat_write_header(ofmt_ctx, NULL);
//     if (ret < 0) {
//         fprintf(stderr, "Error occurred when opening output file\n");
//         goto end;
//     }
//     while (1) {
//         AVStream *in_stream, *out_stream;
//         ret = av_read_frame(ifmt_ctx, &pkt);
//         if (ret < 0)
//             break;
//         in_stream  = ifmt_ctx->streams[pkt.stream_index];
//         out_stream = ofmt_ctx->streams[pkt.stream_index];
//         log_packet(ifmt_ctx, &pkt, "in");
//         /* copy packet */
//         pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
//         pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
//         pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
//         pkt.pos = -1;
//         log_packet(ofmt_ctx, &pkt, "out");
//         ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
//         if (ret < 0) {
//             fprintf(stderr, "Error muxing packet\n");
//             break;
//         }
//         av_free_packet(&pkt);
//     }
//     av_write_trailer(ofmt_ctx);
// end:
//     avformat_close_input(&ifmt_ctx);
//     /* close output */
//     if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
//         avio_closep(&ofmt_ctx->pb);
//     avformat_free_context(ofmt_ctx);
//     if (ret < 0 && ret != AVERROR_EOF) {
//         fprintf(stderr, "Error occurred: bad stuff\n");
//         return 1;
//     }
//     return 0;
// }




/*
 * Consumes a video stream, transmuxes to MPEG2-TS, and injects a timestamp
 * in the data stream of the MPEG2-TS.
 *
 * General program flow:
 * 1. Init ffmpeg libs
 * 2. Open video stream
 * 3. Init input video stream context
 * 4. Init output MPEG2-TS video and data stream contexts
 * 5. For each input video frame
 *   a. Put in output MPEG2-TS video stream (no decoding)
 *   b. Call inject_timestamp() callback and put value into MPEG2-TS data stream
 *
 *
 * NOTES:
 * - avformat_open_input() can open up network streams as well...
 */

#include <getopt.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <log4cpp/Category.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/OstreamAppender.hh>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include <libavutil/common.h>
#include "libavutil/dict.h"
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/mathematics.h>
#include <libavfilter/avfilter.h>
}

static log4cpp::Appender *appender = new log4cpp::OstreamAppender("console", &std::cout);
static log4cpp::Category& logger = log4cpp::Category::getRoot();

/**
 * Callback that is called each time a video frame is processed. The returned
 * timestamp value will be stored in data frame corresponding to the video frame.
 * This method should not spend too much time doing work or block. A good
 * practice would be to poll some shared state for the timestamp value.
 *
 * @return timestamp as uint64_t, ms since Unix Epoch
 */
uint64_t inject_timestamp_callback() {
    return 0;
}

#define ts2str(ts) av_ts_make_string(new char[AV_TS_MAX_STRING_SIZE], ts)
#define ts2timestr(ts, tb) av_ts_make_time_string(new char[AV_TS_MAX_STRING_SIZE], ts, tb)


static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag) {
    AVRational* time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

    char* pts = ts2str(pkt->pts);
    char* pts_time_base = ts2timestr(pkt->pts, time_base);
    char* dts = ts2str(pkt->dts);
    char* dts_time_base = ts2timestr(pkt->dts, time_base);
    char* duration = ts2str(pkt->duration);
    char* duration_time_base = ts2timestr(pkt->duration, time_base);
    printf("%s: pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
           tag,
           pts, pts_time_base,
           dts, dts_time_base,
           duration, duration_time_base,
           pkt->stream_index);
    delete pts;
    delete pts_time_base;
    delete dts;
    delete dts_time_base;
    delete duration;
    delete duration_time_base;
}

/**
 * Prints usage
 */
void usage() {
    printf("vid-time-injector - DESCRIPTION\n\n");
    printf("Usage:\n");
    printf("vid-time-injector [options]\n\n");
    printf("Options:\n");
    printf("-h, --help                Displays this information.\n");
    printf("-v, --verbose             Verbose mode on.\n");
    printf("-i, --input <string>      input stream. Required.\n");
    printf("-o, --output <string>     output stream. Required.\n");
}

int main (int argc, char **argv) {
    // INPUT
    AVFormatContext *       input_fmt_ctx = NULL;         // Input Format context
    int                     input_vid_stream_idx;
    // OUTPUT
    AVOutputFormat*         output_fmt = NULL;      // output format
    AVFormatContext*        output_fmt_ctx = NULL;  // output format context

    AVFilterGraph*          filter_graph = NULL;

    AVBitStreamFilter*      h264_mp4toannexb_filter = NULL;
    AVBitStreamFilterContext* h264_mp4toannexb_filter_ctx = NULL;

    AVPacket                pkt;                    // Packet received by AV
    int                     ret;                    // General return val
    long vid_pkt_cnt = 0;
    long frame_ctr = 0;

    // cmd line arg vars
    bool verbose = false;
    std::string input;
    std::string output;
    int next_option;
	const char* const short_options = "hvi:o:";
	const struct option long_options[] =
	{
		{ "help", no_argument, NULL, 'h' },
		{ "verbose", required_argument, NULL, 'v' },
		{ "input", required_argument, NULL, 'i' },
		{ NULL, 0, NULL, 0 }
	};

    // init logger
	appender->setLayout(new log4cpp::BasicLayout());
	logger.setPriority(log4cpp::Priority::WARN);
	logger.addAppender(appender);

    // get cmd line args
    // parse command line args using getopt
	while(1) {
		// obtain a option
		next_option = getopt_long(argc, argv, short_options, long_options, NULL);

		if(next_option == -1)
			break;  // no more options

		switch(next_option) {
		case 'h':      // -h or --help
			usage();
            exit(EXIT_SUCCESS);
		case 'v':      // -v or --verbose
			verbose = true;
			break;
		case 'i':      // -i or --input
			input = std::string(optarg);
			break;
		case 'o':      // -o or --output
			output = std::string(optarg);
			break;
		case '?':      // Invalid option
			usage();
            exit(EXIT_FAILURE);
		case -1:      // No more options
			break;
		default:      // shouldn't happen :-)
			return(EXIT_FAILURE);
		}
	}

    if(verbose)
		logger.setPriority(log4cpp::Priority::DEBUG);

    // Register all formats, codecs, and network facilities
    av_register_all();
    avfilter_register_all();
    avformat_network_init();

    // open input file, and allocate format context
    logger.debug("Opening input %s", input.c_str());
    if(avformat_open_input(&input_fmt_ctx, input.c_str(), NULL, NULL) < 0) {
        logger.error("Could not open %s", input.c_str());
        exit(EXIT_FAILURE);
    }

    // init h264_mp4toannexb_filter, this will convert an H.264 bitstream from
    // length prefixed mode to start code prefixed mode (as defined in the
    // Annex B of the ITU-T H.264 specification).
    // This is required by some streaming formats, typically the MPEG-2
    // transport stream format ("mpegts").for MPEG2-TS streams that need
    // a startcode in the h264.
    // If this does not used, then you will get an error like this:
    // [mpegts @ 0x6c6240] H.264 bitstream malformed, no startcode found, use the video bitstream filter 'h264_mp4toannexb' to fix it ('-bsf:v h264_mp4toannexb' option with ffmpeg)
    // h264_mp4toannexb_filter = avfilter_get_by_name("h264_mp4toannexb");
    h264_mp4toannexb_filter_ctx = av_bitstream_filter_init("h264_mp4toannexb");
    if(!h264_mp4toannexb_filter_ctx) {
        logger.error("Could not initialize h264_mp4toannexb filter");
        exit(EXIT_FAILURE);
    }

    av_register_bitstream_filter(h264_mp4toannexb_filter_ctx->filter);

    // retrieve stream information
    logger.debug("Retrieving stream information");
    if(avformat_find_stream_info(input_fmt_ctx, NULL) < 0) {
        logger.error("Could not find stream information");
        exit(EXIT_FAILURE);
    }

    // dump input information to stderr
    av_dump_format(input_fmt_ctx, 0, input.c_str(), 0);

    // init output
    avformat_alloc_output_context2(&output_fmt_ctx, NULL, NULL, output.c_str());
    if(!output_fmt_ctx) {
        logger.error("Could not create output context");
        exit(EXIT_FAILURE);
    }
    output_fmt = output_fmt_ctx->oformat;

    // analyze the input container streams and create the corresponding streams
    // in the output container
    for(unsigned int i = 0; i < input_fmt_ctx->nb_streams; i++) {
        AVStream *in_stream = input_fmt_ctx->streams[i];
        AVStream *out_stream = avformat_new_stream(output_fmt_ctx, in_stream->codec->codec);
        if(!out_stream) {
            logger.error("Failed allocating output stream");
            exit(EXIT_FAILURE);
        }

        // copy codec context
        if(avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
            logger.error("Failed to copy context from input to output stream codec context");
            exit(EXIT_FAILURE);
        }

        out_stream->codec->codec_tag = 0;
        if(output_fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
            out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
        }

        // find video stream idx
        if(in_stream->codec->codec_type == AVMEDIA_TYPE_VIDEO)
            input_vid_stream_idx = i;
    }

    // dump output information to stderr
    av_dump_format(output_fmt_ctx, 0, output.c_str(), 1);

    // create output file if it doesn't exit already
    if(!(output_fmt->flags & AVFMT_NOFILE)) {
        if(avio_open(&output_fmt_ctx->pb, output.c_str(), AVIO_FLAG_WRITE) < 0) {
            logger.error("Could not open output file '%s'", output.c_str());
            exit(EXIT_FAILURE);
        }
    }

    ret = avformat_write_header(output_fmt_ctx, NULL);
    if (ret < 0) {
        logger.error("Error occurred when opening output file");
        exit(EXIT_FAILURE);
    }


    // main loop
    // process the input video, copy data
    while(1) {
        logger.debug("Processing frame...");
        AVStream *in_stream, *out_stream;
        ret = av_read_frame(input_fmt_ctx, &pkt);
        if(ret < 0)
            break;
        in_stream  = input_fmt_ctx->streams[pkt.stream_index];
        out_stream = output_fmt_ctx->streams[pkt.stream_index];
        log_packet(input_fmt_ctx, &pkt, "in");

        /* copy packet */
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        log_packet(output_fmt_ctx, &pkt, "out");

        if(pkt.stream_index == input_vid_stream_idx) {
            // filter packet using the h264_mp4toannexb bitstream filter
            AVPacket filtered_pkt = pkt;

            logger.debug("Filtering...");
            int f = pkt.flags & AV_PKT_FLAG_KEY;


            // FIXME I think the problem why its segfauulting here is because the 2nd and 3rd arg
            // are needed...cant be null
            ret = av_bitstream_filter_filter(h264_mp4toannexb_filter_ctx,     // bitstream filter context
                                             NULL,                            // AVCodecContext, may be NULL
                                             NULL,                            // filter config args, may be NULL
                                             &filtered_pkt.data,              // output pointer to filter buffer
                                             &filtered_pkt.size,              // output pointer to filter buffer len
                                             pkt.data,                        // input buffer
                                             pkt.size,                        // input buffer size
                                             f);    // if input buf corresponds to key-frame pkt data

            if(ret < 0) {
                logger.error("Error filtering packet");
                break;
            } else {
                logger.debug("Creating buffer...");
                filtered_pkt.buf = av_buffer_create(filtered_pkt.data, filtered_pkt.size,
                                                    av_buffer_default_free, NULL, 0);
                if(!filtered_pkt.buf) {
                    logger.error("Failed to create filtered pkt buffer");
                    break;
                }
            }

            logger.debug("Writing filtered frame...");
            ret = av_interleaved_write_frame(output_fmt_ctx, &filtered_pkt);
        } else {
            logger.debug("Writing frame...");
            ret = av_interleaved_write_frame(output_fmt_ctx, &pkt);
        }



        if(ret < 0) {
            logger.error("Error muxing packet");
            break;
        }
        av_free_packet(&pkt);
        frame_ctr++;
    }
    av_write_trailer(output_fmt_ctx);





//     // Read frames from input
//     logger.debug("Reading frames...");
//     while (av_read_frame(fmt_ctx, &pkt) >= 0) {
//         if (pkt.stream_index == vid_idx_stream) {         // packet from video stream
//             //printf("Parsing video packet pos = %d\n", pkt.pos);
// //            avcodec_decode_video2(vid_dec_ctx, in_frame, &fF, &pkt);
// //            sws_scale(cvt_ctx, in_frame->data,
// //                      in_frame->linesize, 0,
// //                      vid_dec_ctx->height,
// //                      out_frame->data,
// //                      out_frame->linesize);
// //            sleep_ms = pkt.duration / 90;
//             vid_pkt_cnt++;
//         }
//
//         av_free_packet(&pkt);
//     }

    logger.debug("Cleaning up...");
    logger.debug("read %ld video packets", vid_pkt_cnt);
    logger.debug("processed %ld frames", frame_ctr);
    // printf("read %d data packets\n", data_pkt_cnt);
    // printf("read %d unkown packets\n", unkown_pkt_cnt);
    // data_dump_file.close();
    // avcodec_close(vid_dec_ctx);
    // avcodec_close(data_dec_ctx);
//    av_frame_free(&frame);
    avformat_close_input(&input_fmt_ctx);
    /* close output */
    if(output_fmt_ctx && !(output_fmt->flags & AVFMT_NOFILE))
        avio_closep(&output_fmt_ctx->pb);
    avformat_free_context(output_fmt_ctx);
    if(ret < 0 && ret != AVERROR_EOF) {
        // fprintf(stderr, "Error occurred: %s\n", av_err2str(ret));
        logger.error("Error occured: %d", ret);
        return 1;
    }
    if(h264_mp4toannexb_filter_ctx)
        av_bitstream_filter_close(h264_mp4toannexb_filter_ctx);
    return 0;
}
