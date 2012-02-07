-- load C lib
require 'libffmpeglib'
-- require 'XLearn'

local load_help_desc =
   [[Loads a video using the ffmpeg library.]]

if not ffmpeglib then

   module('ffmpeglib', package.seeall)

   load = function (...)
	     local args, filename = dok.unpack(
		{...},
		'ffmpeglib.load', load_help_desc,
		{arg='filename', type='string', 
		 help='filename to be loaded', req=true})
	     return libffmpeglib.load(filename)
	  end

   getFrame = function (...)
		 local args, v, w, h, tensor = dok.unpack(
		    {...},
		    'ffmpeglib.getFrame',
		    [[ gets the next frame from the video ]],
		    {arg='v', type='ffmpeglib.vid', 
		     help='the loaded video object', req=true},
		    {arg='w', type='number',
		     help='width for rescaled video', default=0},
		    {arg='h', type='number',
		     help='height for rescaled video', default=0},
		    {arg='tensor', type='torch.Tensor',
		     help='tensor which will contain image information',
		     default=torch.Tensor()}
		 )
		 return libffmpeglib.getFrame(v,w,h,tensor)
	      end
   rawWidth = function (...)
		 local args, v = dok.unpack(
		    {...},
		    'ffmpeglib.rawWidth',
		    [[ return width of raw video ]],
		    {arg='v', type='ffmpeglib.vid', 
		     help='the loaded video object', req=true})
		 return libffmpeglib.rawWidth(v)
	      end
   rawHeight = function (...)
		 local args, v = dok.unpack(
		    {...},
		    'ffmpeglib.rawHeight',
		    [[ return height of raw video ]],
		    {arg='v', type='ffmpeglib.vid', 
		     help='the loaded video object', req=true})
		 return libffmpeglib.rawHeight(v)
	      end
   dstWidth = function (...)
		 local args, v = dok.unpack(
		    {...},
		    'ffmpeglib.dstWidth',
		    [[ return width of scaled video ]],
		    {arg='v', type='ffmpeglib.vid', 
		     help='the loaded video object', req=true})
		 return libffmpeglib.dstWidth(v)
	      end
   dstHeight = function (...)
		 local args, v = dok.unpack(
		    {...},
		    'ffmpeglib.dstHeight',
		    [[ return height of scaled video ]],
		    {arg='v', type='ffmpeglib.vid', 
		     help='the loaded video object', req=true})
		 return libffmpeglib.dstHeight(v)
	      end
   play = function (...)
	     local args, filename, w, h = dok.unpack(
		{...},
		'ffmpeglib.play',
		[[ simple player using the ffmpeg lib and image.display ]],
		{arg='filename', type='string', 
		 help='filename to be loaded', req=true},
		{arg='w', type='int',
		 help='width to show the video',default=320},
		{arg='h', type='int',
		 help='height to show the video',default=180}
	     )
	     local v = load(filename)
	     local f = getFrame(v,w,h)
	     if f:nElement() == 0 then 
		error('no frame found')
	     else
		local gotFrame = true
		local win = image.display{image=f,gui=false}
		local i = 0
		while (gotFrame) do
		   f=getFrame(v,w,h)
		   if f:nElement() == 0 then 
		      gotFrame = false
		      print('BREAKING')
		      break 
		   end
		   image.display{image=f,win=win,gui=false}
		   print('got:'..i)
		   i = i + 1
		end		
	     end
	  end
end

return ffmpeglib