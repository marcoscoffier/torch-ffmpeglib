DESCRIPTION:

Allow access to frames of a video through the ffmpeg library.  No intermediate files are stored to disc.  Everything resides in memory.

FUNCTIONS:

+ open


USAGE:

INSTALLING FFMPEG:

The ubuntu version in Oneric Ocelot is a bit old and there are many
changes to the ffmpeg libraries.  This code works with the recent
ffmpeg git master.  To install on Ubuntu follow these steps (following
http://ubuntuforums.org/showthread.php?t=786095 except that we add the
--enabled-shared flag to configure and don't require x264)

git clone git://source.ffmpeg.org/ffmpeg
cd ffmpeg
./configure --enable-gpl --enable-libfaac --enable-libmp3lame \
 --enable-libopencore-amrnb --enable-libopencore-amrwb --enable-libtheora \
 --enable-libvorbis --enable-nonfree --enable-postproc --enable-version3 \
 --enable-x11grab --enable-shared
make
sudo checkinstall --pkgname=ffmpeg --pkgversion="5:$(date +%Y%m%d%H%M)-git" \
 --backup=no --deldoc=yes --fstrans=no --default
hash ffmpeg ffplay ffprobe
