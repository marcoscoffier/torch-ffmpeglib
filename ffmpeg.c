#include <TH.h>
#include <luaT.h>
#include <stdbool.h>
#include <libswscale/swscale.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <luaT.h>
#include <TH.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define FFMPEG_CONTEXT "ffmpeg_ctx"
#define FFMPEG_LIBRARY "ffmpeg_lib"

/*
 * basic structure to represent a video and the frames */
typedef struct {
  AVFormatContext *pFormatCtx;
  int             videoStream;
  AVCodecContext  *pCodecCtx;
  AVCodec         *pCodec;
  AVFrame         *pFrame; 
  AVFrame         *pFrameRGB;
  AVPacket        packet;
  struct SwsContext *img_convert_ctx;
  int             frameFinished;
  int             numBytes;
  int             dstW;
  int             dstH; 
  uint8_t         *buffer;
  const char      *filename;
} ffmpeg_ctx;

void ffmpeg_ctx_print(ffmpeg_ctx *v) {
  /* dump_format is a rather complicated function, which calls many
   * sub functions for the different modules (for codec, container, video
   * and audio etc.).  Unfortunately all the subfunctions use printf() so
   * it is difficult to get a string for toString() without redirecting the
   * output somehow. */
  av_dump_format(v->pFormatCtx, 0, v->filename, false);   
}

ffmpeg_ctx * ffmpeg_ctx_new (const char* fn){
  ffmpeg_ctx *v = (ffmpeg_ctx *)malloc(sizeof(ffmpeg_ctx));
   v->filename = fn;
   int i = 0;
   /* Open video file */
   if(avformat_open_input_file(&(v->pFormatCtx), v->filename, NULL, 0, NULL)!=0)
     THError("<ffmeg.vid> Couldn't open file"); 
   /* Retrieve stream information */
   if(av_find_stream_info(v->pFormatCtx)<0)
     THError("<ffmeg.vid> Couldn't find stream information");
   /* Print info about the stream */
   Lffmpeg_ctx_print(v);
   /* Find the first video stream */
   /* FIXME should also be able to request a specific channel for stereo vids */
   v->videoStream=-1;
   for(i=0; i<v->pFormatCtx->nb_streams; i++)
     if(v->pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
     {
        v->videoStream=i;
        break;
     }
     
   if(v->videoStream==-1)
     THError("<ffmeg.vid> Didn't find a video stream");
   /* Get a pointer to the codec context for the video stream */
   v->pCodecCtx=v->pFormatCtx->streams[v->videoStream]->codec;
   /* Find the decoder for the video stream */
   v->pCodec=avcodec_find_decoder(v->pCodecCtx->codec_id);
   if(v->pCodec==NULL)
     THError("<ffmeg.vid> Codec not found");
   /* Open codec */
   if(avcodec_open(v->pCodecCtx, v->pCodec)<0)
     THError("<ffmeg.vid> Could not open codec"); 
   /*
    * Hack to correct wrong frame rates that seem to be generated by
    *  some codecs  */
    if(v->pCodecCtx->time_base.num>1000 && v->pCodecCtx->time_base.den==1)
		v->pCodecCtx->time_base.den=1000;
    /* Allocate video frame */
    v->pFrame=avcodec_alloc_frame();
    /* Allocate an AVFrame structure */
    v->pFrameRGB=avcodec_alloc_frame();
    if(v->pFrameRGB==NULL)
      THError("<ffmeg.vid> could not allocate AVFrame structure");
    /* Determine required buffer size */
    v->numBytes=avpicture_get_size(PIX_FMT_RGB24, v->pCodecCtx->width,
        v->pCodecCtx->height);
    /* Allocate buffer */
    v->buffer=malloc(v->numBytes); 
    /* Assign appropriate parts of buffer to image planes in pFrameRGB */
    avpicture_fill((AVPicture *)v->pFrameRGB, v->buffer, PIX_FMT_RGB24,
        v->pCodecCtx->width, v->pCodecCtx->height);
    /*
     * Initialize the destination sizes for the software re-scaling
     * called in get_frame */
    v->dstW = 0;
    v->dstH = 0;
    return v;
}

/*
 * Clean up.  Free all the parts of the ffmpeg_ctx*/
void ffmpeg_ctx_free (ffmpeg_ctx *v){
  // Free the RGB image
  if(v->buffer) {  
    free(v->buffer);
    printf("free buffer\n");
  }

  if(v->pFrameRGB){
    av_free(v->pFrameRGB);
    printf("free pFrameRGB\n");
  }

  // Free the YUV frame
  if(v->pFrame){
    av_free(v->pFrame);
    printf("free pFrame\n");
  }

  // Close the Software Scaler
  if (v->img_convert_ctx){
    sws_freeContext(v->img_convert_ctx);
    printf("free swsContext\n");
  }

  // Close the codec
  if(v->pCodecCtx){
    avcodec_close(v->pCodecCtx);
    printf("free codec\n");
  }

    // Close the video file
  if(v->pFormatCtx){
    av_close_input_file(v->pFormatCtx);
    printf("free Format\n");
  }  
}

void Lffmpeg_ctx_push(lua_State *L, ffmpeg_ctx *v){
   ffmpeg_ctx *nv = (ffmpeg_ctx *)lua_newuserdata(L, sizeof(ffmpeg_ctx));
   luaL_getmetatable(L, FFMPEG_CONTEXT);
   lua_setmetatable(L, -2);
   memcpy(nv, v, sizeof(ffmpeg_ctx));
   ffmpeg_ctx_free(v);
   printf("free ffmpeg_ctx\n");
}

ffmpeg_ctx *Lffmpeg_ctx_check(lua_State *L, int pos){
   ffmpeg_ctx *v = (ffmpeg_ctx *)luaL_checkudata(L, pos, FFMPEG_CONTEXT);
   if (v == NULL) THError("<ffmpeg_ctx> requires a valid ffmpeg_ctx");
   return v;
}


static int Lffmpeg_ctx_rawWidth(lua_State *L) {
  ffmpeg_ctx *v = (ffmpeg_ctx *)luaL_checkudata(L, 1, FFMPEG_CONTEXT);
  int w = v->pCodecCtx->width; 
  lua_pushnumber(L,w);
  return 1;
}

static int Lffmpeg_ctx_rawHeight (lua_State *L) {
  ffmpeg_ctx *v = (ffmpeg_ctx *)luaL_checkudata(L, 1, FFMPEG_CONTEXT);
  int h = v->pCodecCtx->height; 
  lua_pushnumber(L,h);
  return 1;
}

static int Lffmpeg_ctx_dstWidth (lua_State *L) {
  ffmpeg_ctx *v = (ffmpeg_ctx *)luaL_checkudata(L, 1, FFMPEG_CONTEXT);
  int w = v->dstW; 
  lua_pushnumber(L,w);
  return 1;
}

static int Lffmpeg_ctx_dstHeight (lua_State *L) {
  ffmpeg_ctx *v = (ffmpeg_ctx *)luaL_checkudata(L, 1, FFMPEG_CONTEXT);
  int h = v->dstH; 
  lua_pushnumber(L,h);
  return 1;
}

/*
 * Lffmpeg_ctx cleanup function */
static int Lffmpeg_ctx_close(lua_State *L) {
  ffmpeg_ctx *v = Lffmpeg_ctx_check(L, 1);
  ffmpeg_ctx_free(v);
  return 0;
}

/*
 * Loads a video from a filename, returns ffmpeg struct 
 * discovers codec, all that jazz */
static int Lffmpeg_ctx_open (lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  ffmpeg_ctx* v = ffmpeg_ctx_new(filename);
  Lffmpeg_ctx_push(L, v);
  return 1;
}

static int Lffmpeg_init(lua_State *L){
  /* initialize the lib */
  av_register_all();
}



/*
 * example of seeking from ffplay.c */
/* if (cur_stream) { */
/*   if (seek_by_bytes) { */
/*     if (cur_stream->video_stream >= 0 && cur_stream->video_current_pos>=0){ */
/*       pos= cur_stream->video_current_pos; */
/*     }else if(cur_stream->audio_stream >= 0 && cur_stream->audio_pkt.pos>=0){ */
/*       pos= cur_stream->audio_pkt.pos; */
/*     }else */
/*       pos = avio_tell(cur_stream->ic->pb); */
/*     if (cur_stream->ic->bit_rate) */
/*       incr *= cur_stream->ic->bit_rate / 8.0; */
/*     else */
/*       incr *= 180000.0; */
/*     pos += incr; */
/*     stream_seek(cur_stream, pos, incr, 1); */
/*   } else { */
/*     pos = get_master_clock(cur_stream); */
/*     pos += incr; */
/*     stream_seek(cur_stream, (int64_t)(pos * AV_TIME_BASE), (int64_t)(incr * AV_TIME_BASE), 0); */
/*   } */
/*  } */



static const struct luaL_reg ffmpeg_ctx_methods [] = {
  {"__gc",       Lffmpeg_ctx_close},
  {"close",      Lffmpeg_ctx_close},
  {"open",       Lffmpeg_ctx_open},
  {"rawWidth",   Lffmpeg_ctx_rawWidth},
  {"rawHeight",  Lffmpeg_ctx_rawHeight}, 
  {"dstWidth",   Lffmpeg_ctx_dstWidth},
  {"dstHeight",  Lffmpeg_ctx_dstHeight},
  /* {"seek",       Lffmpeg_ctx_seek}, */
  {NULL, NULL}  /* sentinel */
};

static const struct luaL_reg ffmpeg_lib [] = {
  {"init",       Lffmpeg_init},
  {NULL, NULL} /* sentinel */
};


#define torch_(NAME) TH_CONCAT_3(torch_, Real, NAME)
#define torch_string_(NAME) TH_CONCAT_STRING_3(torch., Real, NAME)
#define Lffmpeg_(NAME) TH_CONCAT_3(Lffmpeg_, Real, NAME)

static const void* torch_FloatTensor_id = NULL;
static const void* torch_DoubleTensor_id = NULL;

#include "generic/ffmpeg.c"
#include "THGenerateFloatTypes.h"

DLL_EXPORT int luaopen_libffmpeg(lua_State *L)
{
  /* create ffmpeg_ctx metatable */
   luaL_newmetatable(L, FFMPEG_CONTEXT);
   lua_createtable(L, 0, sizeof(ffmpeg_ctx_methods) / sizeof(luaL_reg) - 1);
   luaL_register(L,NULL,ffmpeg_ctx_methods);
   lua_setfield(L, -2 , "__index");

   luaL_register(L, "ffmpeg", ffmpeg_lib);
   
   /* load the functions which copy frames to Tensors */
  torch_FloatTensor_id = luaT_checktypename2id(L, "torch.FloatTensor");
  torch_DoubleTensor_id = luaT_checktypename2id(L, "torch.DoubleTensor");

  Lffmpeg_FloatMain_init(L);
  Lffmpeg_DoubleMain_init(L);

  luaL_register(L, "ffmpeg.double", Lffmpeg_DoubleMain__); 
  luaL_register(L, "ffmpeg.float", Lffmpeg_FloatMain__);

  return 1;
}