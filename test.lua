require 'torch'
require 'libffmpeg'

fname = "/home/data/gopro_360/washsq_2/0.mp4"
fname2 = "/home/data/gopro_360/washsq_2/1.mp4"
ffmpeglib.init()
f = ffmpeg.open(fname)
f2 = ffmpeg.open(fname2)

a = torch.Tensor()

ffmpeg.double.getFrame(f,a)

