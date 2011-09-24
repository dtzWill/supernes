/*
 * ===========================================================================
 *
 *       Filename:  RegionTimer.h
 *
 *    Description:  Timer that prints its lifetime
 *
 *        Version:  1.0
 *        Created:  09/23/2011 04:18:55 AM
 *
 *         Author:  Will Dietz (WD), w@wdtz.org
 *        Company:  dtzTech
 *
 * ===========================================================================
 */

#ifndef _REGION_TIMER_H_
#define _REGION_TIMER_H_

#include <SDL.h>
#include <iostream>

// #define USE_TIMER

#ifdef USE_TIMER
struct RegionTimer {
  int start;
  const char * msg;
  RegionTimer(const char * m) : msg(m)
  {
    start = SDL_GetTicks();
  }

  ~RegionTimer() {
    std::cerr << "Timer " << msg << ": ";
    std::cerr << SDL_GetTicks() - start << std::endl;
  }
};
#else
struct RegionTimer {
  RegionTimer(const char * m) {};
};
#endif

#endif // _REGION_TIMER_H_
