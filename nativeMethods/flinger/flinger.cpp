/*
	 droid vnc server - Android VNC server
	 Copyright (C) 2009 Jose Pereira <onaips@gmail.com>

	 This library is free software; you can redistribute it and/or
	 modify it under the terms of the GNU Lesser General Public
	 License as published by the Free Software Foundation; either
	 version 3 of the License, or (at your option) any later version.

	 This library is distributed in the hope that it will be useful,
	 but WITHOUT ANY WARRANTY; without even the implied warranty of
	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	 Lesser General Public License for more details.

	 You should have received a copy of the GNU Lesser General Public
	 License along with this library; if not, write to the Free Software
	 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
	 */

#include "flinger.h"
#include "screenFormat.h"

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#include <binder/IMemory.h>
#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ui/DisplayInfo.h>
#include <ui/PixelFormat.h>

using namespace android;

ScreenshotClient *screenshotClient=NULL;

extern "C" screenFormat getscreenformat_flinger()
{
	screenFormat format;

#if 1 //wqm,20161227
	struct fb_var_screeninfo vinfo;
	unsigned int bytespp;
	int fd;
	const char* fbpath = "/dev/graphics/fb0";
	fd = open(fbpath, O_RDONLY);
	if(fd >= 0)
	{
		if(ioctl(fd, FBIOGET_VSCREENINFO, &vinfo) >= 0)
		{
			bytespp = vinfo.bits_per_pixel / 8;
			format.bitsPerPixel = vinfo.bits_per_pixel;
			format.size 		= vinfo.xres * vinfo.yres * bytespp;
			format.width 		= vinfo.xres;
			format.height 		= vinfo.yres;
			format.redShift 	= vinfo.red.offset;
			format.redMax 		= vinfo.red.length;
			format.greenShift 	= vinfo.green.offset;
			format.greenMax 	= vinfo.green.length;
			format.blueShift 	= vinfo.blue.offset;
			format.blueMax 		= vinfo.blue.length;
			format.alphaShift 	= vinfo.transp.offset;
			format.alphaMax 	= vinfo.transp.length;
		}
		else
		{
			L("ioctl failed, %s", strerror(errno));
		}
	}
	else
	{
		L("--fbpath=%s open failed!--\n",fbpath);
	}

	close(fd);

#else
	 //get format on PixelFormat struct
	PixelFormat f=screenshotClient->getFormat();

	format.bitsPerPixel = bitsPerPixel(f);
	format.width = screenshotClient->getWidth();
	format.height = screenshotClient->getHeight();
	format.size = format.bitsPerPixel*format.width*format.height/CHAR_BIT;

	format.redShift = pf.l_red;
	format.redMax = pf.h_red;
	format.greenShift = pf.l_green;
	format.greenMax = pf.h_green-pf.h_red;
	format.blueShift = pf.l_blue;
	format.blueMax = pf.h_blue-pf.h_green;
	format.alphaShift = pf.l_alpha;
	format.alphaMax = pf.h_alpha-pf.h_blue;
#endif

#if 0
	PixelFormatInfo pf;
	getPixelFormatInfo(f,&pf);

	format.bitsPerPixel = pf.bitsPerPixel;
	format.width = screenshotClient->getWidth();
	format.height =     screenshotClient->getHeight();
	format.size = pf.bitsPerPixel*format.width*format.height/CHAR_BIT;
	format.redShift = pf.l_red;
	format.redMax = pf.h_red;
	format.greenShift = pf.l_green;
	format.greenMax = pf.h_green-pf.h_red;
	format.blueShift = pf.l_blue;
	format.blueMax = pf.h_blue-pf.h_green;
	format.alphaShift = pf.l_alpha;
	format.alphaMax = pf.h_alpha-pf.h_blue;
#endif

	return format;
}

#if 1 //wqm,20161227
static int screenshotClient_update()
{
	static uint32_t DEFAULT_DISPLAY_ID = ISurfaceComposer::eDisplayIdMain;
	int32_t displayId = 1;//DEFAULT_DISPLAY_ID;

#if 0
	 sp<IBinder> display = SurfaceComposerClient::getBuiltInDisplay(displayId);
	    if (display == NULL) {
	        L("Unable to get handle for display %d\n", displayId);
	        return 1;
	    }
#endif

	return 0;
}
#endif

extern "C" int init_flinger()
{
	int er;

	L("--Initializing gingerbread access method--\n");

  screenshotClient = new ScreenshotClient();

#if 0 //wqm,20161227
	errno = screenshotClient->update();
#else
	er = screenshotClient_update();
#endif

  if (!screenshotClient->getPixels())
    return -1;

  if (er != NO_ERROR) {
		return -1;
	}
	return 0;
}

extern "C" unsigned int *readfb_flinger()
{

#if 0//wqm,20161227
	errno = screenshotClient->update();
#else
	errno = screenshotClient_update();
#endif

	return (unsigned int*)screenshotClient->getPixels();
}

extern "C" void close_flinger()
{
  free(screenshotClient);
}
