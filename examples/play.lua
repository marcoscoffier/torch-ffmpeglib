require 'torch'
require 'image'
require 'ffmpeglib'

-- do everything in float
torch.setdefaulttensortype('torch.FloatTensor')

w = 640
h = 480

fname = "/home/data/gopro_360/washsq_2/0.mp4"

ffmpeglib.init()

f = ffmpeg.open(fname,w,h)
a = torch.Tensor()

a.ffmpeg.getFrame(f,a)

win = image.display(a)


for i = 1,1000 do  
   sys.tic() 
   a.ffmpeg.getFrame(f,a)
   win = image.display{win=win,image={a}}
   print("FPS: ".. 1/sys.toc())
end