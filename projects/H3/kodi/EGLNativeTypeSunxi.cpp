/*
 *      Copyright (C) 2011-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "EGLNativeTypeSunxi.h"
#include "guilib/gui3d.h"
#include "utils/StringUtils.h"
#include "utils/SysfsUtils.h"
#include "utils/log.h"

#include <stdio.h>
#include <stdlib.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <EGL/egl.h>
#include "sunxi_display2.h"

#define SCREENID 0

//fixme
#ifndef _FBDEV_WINDOW_H_
#define _FBDEV_WINDOW_H_
typedef struct fbdev_window
{
  unsigned short width;
  unsigned short height;
} fbdev_window;
#endif

#if 0
static const unsigned long supportedModes[] = 
{
  DISP_TV_MOD_480I,
  DISP_TV_MOD_576I,
  DISP_TV_MOD_480P,
  DISP_TV_MOD_576P,
  DISP_TV_MOD_720P_50HZ,
  DISP_TV_MOD_720P_60HZ,
  DISP_TV_MOD_1080I_50HZ,
  DISP_TV_MOD_1080I_60HZ,
  DISP_TV_MOD_1080P_24HZ,
  DISP_TV_MOD_1080P_50HZ,
  DISP_TV_MOD_1080P_60HZ,
  DISP_TV_MOD_1080P_25HZ,
  DISP_TV_MOD_1080P_30HZ,
};
#endif

CEGLNativeTypeSunxi::CEGLNativeTypeSunxi() :
m_disp(-1)
{
  const char *env_framebuffer = getenv("FRAMEBUFFER");

  // default to framebuffer 0
  m_framebuffer_name = "fb0";
  if (env_framebuffer)
  {
    std::string framebuffer(env_framebuffer);
    std::string::size_type start = framebuffer.find("fb");
    m_framebuffer_name = framebuffer.substr(start);
  }
  m_nativeWindow = NULL;
}

CEGLNativeTypeSunxi::~CEGLNativeTypeSunxi()
{
}

bool CEGLNativeTypeSunxi::CheckCompatibility()
{
  std::string name;
  std::string modalias = "/sys/class/graphics/" + m_framebuffer_name + "/device/modalias";

  SysfsUtils::GetString(modalias, name);
  StringUtils::Trim(name);
  if (name == "platform:disp")
    return true;
  return false;
}

void CEGLNativeTypeSunxi::Initialize()
{
  unsigned long args[4];
  
  if((m_disp = open("/dev/disp", O_RDWR)) == -1)
  {
    CLog::Log(LOGERROR, "%s - Can't open display. Nothing will work!\n", __FUNCTION__);
    return;
  }

#if 0
  args[0] = SCREENID;
  args[1] = 1;
  args[2] = 0;
  args[3] = 0;
  ioctl(m_disp, DISP_VSYNC_EVENT_EN, args);
#endif
}
void CEGLNativeTypeSunxi::Destroy()
{
  if(m_disp != -1)
  {
    close(m_disp);
    m_disp = -1;
  }
}

bool CEGLNativeTypeSunxi::CreateNativeDisplay()
{
  m_nativeDisplay = EGL_DEFAULT_DISPLAY;
  return true;
}

bool CEGLNativeTypeSunxi::CreateNativeWindow()
{
  unsigned long args[4];
  fbdev_window *nativeWindow = new fbdev_window;
  
  if (!nativeWindow)
    return false;
  
  if (m_disp == -1)
    return false;

  args[0] = SCREENID;
  args[1] = 0;
  args[2] = 0;
  args[3] = 0;
  nativeWindow->width = ioctl(m_disp, DISP_GET_SCN_WIDTH, args);
  nativeWindow->height = ioctl(m_disp, DISP_GET_SCN_HEIGHT, args);
  m_nativeWindow = nativeWindow;

  return true;
}

bool CEGLNativeTypeSunxi::GetNativeDisplay(XBNativeDisplayType **nativeDisplay) const
{
  if (!nativeDisplay)
    return false;
  
  *nativeDisplay = (XBNativeDisplayType*) &m_nativeDisplay;
  return true;
}

bool CEGLNativeTypeSunxi::GetNativeWindow(XBNativeWindowType **nativeWindow) const
{
  if (!nativeWindow)
    return false;
  *nativeWindow = (XBNativeWindowType*) &m_nativeWindow;
  return true;
}

bool CEGLNativeTypeSunxi::DestroyNativeDisplay()
{
  return true;
}

bool CEGLNativeTypeSunxi::DestroyNativeWindow()
{
  delete (fbdev_window*)m_nativeWindow, m_nativeWindow = NULL;

  return true;
}

bool CEGLNativeTypeSunxi::GetNativeResolution(RESOLUTION_INFO *res) const
{
  unsigned long args[4];
  disp_output para;
  
  args[0] = SCREENID;
  args[1] = (unsigned long)(&para);
  args[2] = 0;
  args[3] = 0;
  ioctl(m_disp, DISP_GET_OUTPUT, args);
  
  if(!ModeToResolution(para.mode, res))
    return false;
 
  return res->iWidth > 0 && res->iHeight> 0;
}

bool CEGLNativeTypeSunxi::SetNativeResolution(const RESOLUTION_INFO &res)
{
  return false;
#if 0
  if (m_nativeWindow)
  {
    ((fbdev_window *)m_nativeWindow)->width = res.iScreenWidth;
    ((fbdev_window *)m_nativeWindow)->height = res.iScreenHeight;
  }

  switch((int)(0.5 + res.fRefreshRate))
  {
    case 60:
      switch(res.iScreenHeight)
      {
        case 480:
          SetDisplayResolution(DISP_TV_MOD_480I);
          break;
        default:
        case 720:
          SetDisplayResolution(DISP_TV_MOD_720P_60HZ);
          break;
        case 1080:
          if (res.dwFlags & D3DPRESENTFLAG_INTERLACED)
            SetDisplayResolution(DISP_TV_MOD_1080I_60HZ);
          else
            SetDisplayResolution(DISP_TV_MOD_1080P_60HZ);
          break;
      }
      break;
    default:
    case 50:
      switch(res.iScreenHeight)
      {
        case 576:
          SetDisplayResolution(DISP_TV_MOD_576I);
          break;
        default:
        case 720:
          SetDisplayResolution(DISP_TV_MOD_720P_50HZ);
          break;
        case 1280:
          if (res.dwFlags & D3DPRESENTFLAG_INTERLACED)
            SetDisplayResolution(DISP_TV_MOD_1080I_50HZ);
          else
            SetDisplayResolution(DISP_TV_MOD_1080P_50HZ);
          break;
      }
      break;
    case 30:
      switch(res.iScreenHeight)
      {
        case 480:
          SetDisplayResolution(DISP_TV_MOD_480P);
          break;
        default:
        case 1080:
          SetDisplayResolution(DISP_TV_MOD_1080P_30HZ);
          break;
      }
      break;
    case 25:
      switch(res.iScreenHeight)
      {
        case 576:
          SetDisplayResolution(DISP_TV_MOD_576P);
          break;
        default:
        case 1080:
          SetDisplayResolution(DISP_TV_MOD_1080P_25HZ);
          break;
      }
      break;
    case 24:
      SetDisplayResolution(DISP_TV_MOD_1080P_24HZ);
      break;
  }
  
  return true;
#endif
}

bool CEGLNativeTypeSunxi::ProbeResolutions(std::vector<RESOLUTION_INFO> &resolutions)
{
  RESOLUTION_INFO res;
  if(GetNativeResolution(&res))
    resolutions.push_back(res);
#if 0
  for(unsigned int i = 0; i < sizeof(supportedModes) / sizeof(supportedModes[0]); i++)
  {
    RESOLUTION_INFO res;
    //TODO: add hdmi check
    ModeToResolution(supportedModes[i], &res);
    resolutions.push_back(res);
  }
#endif

  return resolutions.size() > 0;
}

bool CEGLNativeTypeSunxi::GetPreferredResolution(RESOLUTION_INFO *res) const
{
  return GetNativeResolution(res);
#if 0
  // check display/mode, it gets defaulted at boot
  if (!GetNativeResolution(res))
  {
    // put to 720p if we get nothing
    ModeToResolution(DISP_TV_MOD_720P_50HZ, res);
  }

  return true;
#endif
}

bool CEGLNativeTypeSunxi::ShowWindow(bool show)
{
  std::string blank_framebuffer = "/sys/class/graphics/" + m_framebuffer_name + "/blank";
  SysfsUtils::SetInt(blank_framebuffer.c_str(), show ? 0 : 1);
  return true;
}

bool CEGLNativeTypeSunxi::SetDisplayResolution(unsigned int mode)
{
  return false;
#if 0
  unsigned long args[4];
  disp_output para;
  
  args[0] = SCREENID;
  args[1] = (unsigned long)(&para);
  args[2] = 0;
  args[3] = 0;
  ioctl(m_disp, DISP_GET_OUTPUT, args);
  
  args[0] = SCREENID;
  args[1] = para.type;
  args[2] = mode;
  args[3] = 0;
  if(ioctl(m_disp, DISP_DEVICE_SWITCH, args) != 0)
    return false;
  
  RESOLUTION_INFO res;
  ModeToResolution(mode, &res);
  SetFramebufferResolution(res.iScreenWidth, res.iScreenHeight);

  return true;
#endif
}

void CEGLNativeTypeSunxi::SetFramebufferResolution(int width, int height) const
{
#if 0
  int fd0;
  std::string framebuffer = "/dev/" + m_framebuffer_name;

  if ((fd0 = open(framebuffer.c_str(), O_RDWR)) >= 0)
  {
    struct fb_var_screeninfo vinfo;
    if (ioctl(fd0, FBIOGET_VSCREENINFO, &vinfo) == 0)
    {
      vinfo.xres = width;
      vinfo.yres = height;
      vinfo.xres_virtual = width;
      vinfo.yres_virtual = height * 2;
      vinfo.bits_per_pixel = 16;
      vinfo.activate = FB_ACTIVATE_ALL;
      ioctl(fd0, FBIOPUT_VSCREENINFO, &vinfo);
    }
    close(fd0);
  }
#endif
}

bool CEGLNativeTypeSunxi::ModeToResolution(unsigned int mode, RESOLUTION_INFO *res) const
{
  
  if (!res)
    return false;

  res->iWidth = 0;
  res->iHeight= 0;
  
  switch(mode)
  {
    case DISP_TV_MOD_480I:
      res->iWidth = 720;
      res->iHeight= 480;
      res->iScreenWidth = 720;
      res->iScreenHeight= 480;
      res->fRefreshRate = 60;
      res->dwFlags = D3DPRESENTFLAG_INTERLACED;
      break;
    case DISP_TV_MOD_576I:
      res->iWidth = 720;
      res->iHeight= 576;
      res->iScreenWidth = 720;
      res->iScreenHeight= 576;
      res->fRefreshRate = 50;
      res->dwFlags = D3DPRESENTFLAG_INTERLACED;
      break;
    case DISP_TV_MOD_480P:
      res->iWidth = 720;
      res->iHeight= 480;
      res->iScreenWidth = 720;
      res->iScreenHeight= 480;
      res->fRefreshRate = 30;
      res->dwFlags = D3DPRESENTFLAG_PROGRESSIVE;
      break;
    case DISP_TV_MOD_576P:
      res->iWidth = 720;
      res->iHeight= 576;
      res->iScreenWidth = 720;
      res->iScreenHeight= 576;
      res->fRefreshRate = 25;
      res->dwFlags = D3DPRESENTFLAG_PROGRESSIVE;
      break;
    case DISP_TV_MOD_720P_50HZ:
      res->iWidth = 1280;
      res->iHeight= 720;
      res->iScreenWidth = 1280;
      res->iScreenHeight= 720;
      res->fRefreshRate = 50;
      res->dwFlags = D3DPRESENTFLAG_PROGRESSIVE;
      break;
    case DISP_TV_MOD_720P_60HZ:
      res->iWidth = 1280;
      res->iHeight= 720;
      res->iScreenWidth = 1280;
      res->iScreenHeight= 720;
      res->fRefreshRate = 60;
      res->dwFlags = D3DPRESENTFLAG_PROGRESSIVE;
      break;
    case DISP_TV_MOD_1080I_50HZ:
      res->iWidth = 1920;
      res->iHeight= 1080;
      res->iScreenWidth = 1920;
      res->iScreenHeight= 1080;
      res->fRefreshRate = 50;
      res->dwFlags = D3DPRESENTFLAG_INTERLACED;
      break;
    case DISP_TV_MOD_1080I_60HZ:
      res->iWidth = 1920;
      res->iHeight= 1080;
      res->iScreenWidth = 1920;
      res->iScreenHeight= 1080;
      res->fRefreshRate = 60;
      res->dwFlags = D3DPRESENTFLAG_INTERLACED;
      break;
    case DISP_TV_MOD_1080P_24HZ:
      res->iWidth = 1920;
      res->iHeight= 1080;
      res->iScreenWidth = 1920;
      res->iScreenHeight= 1080;
      res->fRefreshRate = 24;
      res->dwFlags = D3DPRESENTFLAG_PROGRESSIVE;
      break;
    case DISP_TV_MOD_1080P_50HZ:
      res->iWidth = 1920;
      res->iHeight= 1080;
      res->iScreenWidth = 1920;
      res->iScreenHeight= 1080;
      res->fRefreshRate = 50;
      res->dwFlags = D3DPRESENTFLAG_PROGRESSIVE;
      break;
    case DISP_TV_MOD_1080P_60HZ:
      res->iWidth = 1920;
      res->iHeight= 1080;
      res->iScreenWidth = 1920;
      res->iScreenHeight= 1080;
      res->fRefreshRate = 60;
      res->dwFlags = D3DPRESENTFLAG_PROGRESSIVE;
      break;
    case DISP_TV_MOD_1080P_25HZ:
      res->iWidth = 1920;
      res->iHeight= 1080;
      res->iScreenWidth = 1920;
      res->iScreenHeight= 1080;
      res->fRefreshRate = 25;
      res->dwFlags = D3DPRESENTFLAG_PROGRESSIVE;
      break;
    case DISP_TV_MOD_1080P_30HZ:
      res->iWidth = 1920;
      res->iHeight= 1080;
      res->iScreenWidth = 1920;
      res->iScreenHeight= 1080;
      res->fRefreshRate = 30;
      res->dwFlags = D3DPRESENTFLAG_PROGRESSIVE;
      break;
    /*case DISP_TV_MOD_3840_2160P_30HZ:
      res->iWidth = 3840;
      res->iHeight= 2160;
      res->iScreenWidth = 3840;
      res->iScreenHeight= 2160;
      res->fRefreshRate = 30;
      res->dwFlags = D3DPRESENTFLAG_PROGRESSIVE;
      break;
    case DISP_TV_MOD_3840_2160P_25HZ:
      res->iWidth = 3840;
      res->iHeight= 2160;
      res->iScreenWidth = 3840;
      res->iScreenHeight= 2160;
      res->fRefreshRate = 25;
      res->dwFlags = D3DPRESENTFLAG_PROGRESSIVE;
      break;
    case DISP_TV_MOD_3840_2160P_24HZ:
      res->iWidth = 3840;
      res->iHeight= 2160;
      res->iScreenWidth = 3840;
      res->iScreenHeight= 2160;
      res->fRefreshRate = 24;
      res->dwFlags = D3DPRESENTFLAG_PROGRESSIVE;
      break;*/
    /*case DISP_TV_MOD_PAL:
      res->iWidth = 720;
      res->iHeight= 576;
      res->iScreenWidth = 720;
      res->iScreenHeight= 576;
      res->fRefreshRate = 50;
      res->dwFlags = D3DPRESENTFLAG_INTERLACED;
      break;
    case DISP_TV_MOD_NTSC:
      res->iWidth = 720;
      res->iHeight= 480;
      res->iScreenWidth = 720;
      res->iScreenHeight= 480;
      res->fRefreshRate = 60;
      res->dwFlags = D3DPRESENTFLAG_INTERLACED;
      break;*/
    default:
      return false;
  }
  
  res->iScreen       = 0;
  res->bFullScreen   = true;
  res->iSubtitles    = (int)(0.965 * res->iHeight);
  res->fPixelRatio   = 1.0f;
  res->strMode       = StringUtils::Format("%dx%d @ %.2f%s - Full Screen", res->iScreenWidth, res->iScreenHeight, res->fRefreshRate,
    res->dwFlags & D3DPRESENTFLAG_INTERLACED ? "i" : "");
  
  return true;
}
