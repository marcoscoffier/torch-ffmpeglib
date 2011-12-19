#ifndef TH_GENERIC_FILE
#define TH_GENERIC_FILE "generic/ffmpeg.c"
#else


static THTensor * Lffmpeg_(frame2tensor)(ffmpeg_ctx *v, THTensor *tensor) {
  THTensor_(resize3d)(tensor, v->dstW, v->dstH, 3);
  int x,y;
  int linesize = v->pFrameRGB->linesize[0];
  for (y=0; y<tensor->size[1]; y++)
    for (x=0; x<tensor->size[0]; x++) {
      THTensor_(set3d)(tensor, x, y, 0, 
		     (double)*(v->pFrameRGB->data[0]+(x*3)+(y*linesize)));
      THTensor_(set3d)(tensor, x, y, 1, 
		     (double)*(v->pFrameRGB->data[0]+(x*3)+(y*linesize)+1));
      THTensor_(set3d)(tensor, x, y, 2, 
		     (double)*(v->pFrameRGB->data[0]+(x*3)+(y*linesize)+2));
    }
  return tensor;
}


static int Lffmpeg_(getFrame)(lua_State *L) {
  int w = 0;
  int h = 0;
  AVPacket pkt1, *pkt = &pkt1;
    
  ffmpeg_ctx *v = (ffmpeg_ctx *)luaL_checkudata(L, 1, FFMPEG_CONTEXT);
  THTensor *tensor =
    (THTensor *)luaT_checkudata(L, 2, torch_(Tensor_id));
  if (lua_isnumber(L, 3)) w = lua_tonumber(L, 2);
  if (lua_isnumber(L, 4)) h = lua_tonumber(L, 3);


  if (w==0) { w = v->pCodecCtx->width; }
  if (h==0) { h = v->pCodecCtx->height;}
  v->frameFinished = 0;
  /*
   * check if we need a new software scaling contex. */
  int resample_changed =  w != v->dstW || h != v->dstH; 
  av_init_packet(pkt);
  pkt->data = NULL;
  pkt->size = 0;
  /*
   * Read frames and save first five frames to disk */
  while(! v->frameFinished && (av_read_frame(v->pFormatCtx, pkt)>=0)){
    /* 
     * Is this a packet from the video stream? */
    if(pkt->stream_index==v->videoStream)
      {
	/*
         * Only decode video frames of interest */
	avcodec_decode_video2(v->pCodecCtx, v->pFrame,
                              &v->frameFinished, pkt);
        /*
         * Did we get a video frame? */
	if(v->frameFinished)
	  {
	    /*
             * Convert the image into RBG24 */
	    if(v->img_convert_ctx == NULL || resample_changed ) {

	      
	      v->img_convert_ctx =  NULL;
	      v->img_convert_ctx = 
		sws_getContext(v->pCodecCtx->width, v->pCodecCtx->height, 
			       v->pCodecCtx->pix_fmt, 
			       w, h, PIX_FMT_RGB24, SWS_BICUBIC,
			       NULL, NULL, NULL);
	      if(v->img_convert_ctx == NULL) {
		THError("<ffmpeg.getFrame> Cannot initialize the conversion context!");
	      }
	      /*
               * store the current output w and height as I don't have
               * access to the malformed img_convert_ctx struct */
	      v->dstW = w;
	      v->dstH = h;
	    }
	    int ret = sws_scale(v->img_convert_ctx, 
				(const uint8_t * const*)v->pFrame->data,
                                v->pFrame->linesize, 0, 
				v->pCodecCtx->height, v->pFrameRGB->data, 
				v->pFrameRGB->linesize);
	  }
      }
    
    /* Free the packet that was allocated by av_read_frame */
    av_free_packet(pkt);
  }

  /* copy frame to tensor */
  Lffmpeg_(frame2tensor)(v, tensor);

  return 0;
}

static const struct luaL_Reg Lffmpeg_(Methods) [] = {
  {"getFrame",  Lffmpeg_(getFrame)},
  {NULL, NULL}
};

void Lffmpeg_(Init)(lua_State *L)
{
  luaT_pushmetaclass(L, torch_(Tensor_id));
  luaT_registeratname(L, Lffmpeg_(Methods), "ffmpeg");
}


#endif
