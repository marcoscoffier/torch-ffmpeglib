require 'torch'
require 'image'
require 'libffmpeg'

fname = "/home/data/gopro_360/washsq_2/0.mp4"
ffmpeglib.init()
f = ffmpeg.open(fname)
a = torch.Tensor()

ffmpeg.double.getFrame(f,a)
win = image.display(a)

for i = 1,1000 do  
   sys.tic() 
   ffmpeg.double.getFrame(f,a)
   win = image.display{win=win,image={a}}
   print("FPS: ".. 1/sys.toc())
end