require 'torch'
require 'libffmpeg'
require 'image'

ffmpeglib.init()

w = 320
h = 240

fnames = {
   "/home/data/gopro_360/washsq_2/0.mp4",
   "/home/data/gopro_360/washsq_2/1.mp4",
   "/home/data/gopro_360/washsq_2/2.mp4",
   "/home/data/gopro_360/washsq_2/3.mp4"
}

offsets = {
   "0:00:53.430", -- 0.mp4 
   "0:01:33.430", -- 0.mp4 
   "0:02:33.430", -- 0.mp4 
   "0:03:33.430", -- 0.mp4 
--   "0:00:30.096", -- 1.mp4 
--   "0:00:33.389", -- 2.mp4 
--   "0:00:30.000"  -- 3.mp4 
}

fd = {
   ffmpeg.open(fnames[1],w,h),
   ffmpeg.open(fnames[2],w,h),
   ffmpeg.open(fnames[3],w,h),
   ffmpeg.open(fnames[4],w,h)
}

a = torch.Tensor(3,h,4*w)
an = {
   a:narrow(3,  1,w),
   a:narrow(3,  w,w),
   a:narrow(3,2*w,w),
   a:narrow(3,3*w,w)
}
 
aw = {
   torch.Tensor(3,h,w),
   torch.Tensor(3,h,w),
   torch.Tensor(3,h,w),
   torch.Tensor(3,h,w)
}

for i = 1,4 do 
   print(ffmpeg.filename(fd[i]) ..": seeking to "..offsets[i])
   ffmpeg.seek(fd[i],offsets[i])
   ffmpeg.double.getFrame(fd[i],aw[i])
   an[i]:copy(aw[i])
end

win = image.display(a)

-- for i = 1,1000 do  
--    sys.tic() 
--    for i = 1,4 do 
--       ffmpeg.double.getFrame(fd[i],aw[i])
--       an[i]:copy(aw[i])
--    end
--    win = image.display{win=win,image={a}}
--    print("FPS: ".. 1/sys.toc())
-- end
