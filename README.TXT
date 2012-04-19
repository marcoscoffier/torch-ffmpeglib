DESCRIPTION:

Allow access to frames of a video through the ffmpeg library.  No intermediate files are stored to disc.  Everything resides in memory.

FUNCTIONS:

+ open


USAGE:

INSTALLING FFMPEG:

The api for ffmpeg keeps changing and this is quite frustrating.

Rather than use the most recent ffmpeg which broke too many things on ubuntu.
I changed the code to work with the recent ubuntu's libav*-dev packaged libs
(version 7.3).  

  apt-get install libavcodec-dev libavdevice-dev libavfilter-dev \
    libavformat-dev libavutil-dev libpostproc-dev libswscale-dev

