// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

extern "C" {
#include <stdio.h>
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
}

#include <assert.h>

#include "texture.hh"
#include "basedisplay.hh"
#include "image.hh"
#include "screen.hh"
#include "blackbox.hh"

using std::string;


BTexture::BTexture(const BaseDisplay * const _display,
                   unsigned int _screen, BImageControl* _ctrl)
  : c(_display, _screen), ct(_display, _screen),
    lc(_display, _screen), sc(_display, _screen), bc(_display, _screen), t(0),
    dpy(_display), ctrl(_ctrl), scrn(_screen) { }


BTexture::BTexture(const string &d, const BaseDisplay * const _display,
                   unsigned int _screen, BImageControl* _ctrl)
  : c(_display, _screen), ct(_display, _screen),
    lc(_display, _screen), sc(_display, _screen), bc(_display, _screen), t(0),
    dpy(_display), ctrl(_ctrl), scrn(_screen) {
  setDescription(d);
}


void BTexture::setColor(const BColor &cc) {
  c = cc;
  c.setDisplay(display(), screen());

  unsigned char r, g, b, rr, gg, bb;

  // calculate the light color
  r = c.red();
  g = c.green();
  b = c.blue();
  rr = r + (r >> 1);
  gg = g + (g >> 1);
  bb = b + (b >> 1);
  if (rr < r) rr = ~0;
  if (gg < g) gg = ~0;
  if (bb < b) bb = ~0;
  lc = BColor(rr, gg, bb, display(), screen());

  // calculate the shadow color
  r = c.red();
  g = c.green();
  b = c.blue();
  rr = (r >> 2) + (r >> 1);
  gg = (g >> 2) + (g >> 1);
  bb = (b >> 2) + (b >> 1);
  if (rr > r) rr = 0;
  if (gg > g) gg = 0;
  if (bb > b) bb = 0;
  sc = BColor(rr, gg, bb, display(), screen());
}


void BTexture::setDescription(const string &d) {
  descr.erase();
  descr.reserve(d.length());

  string::const_iterator it = d.begin(), end = d.end();
  for (; it != end; ++it)
    descr += tolower(*it);

  if (descr.find("parentrelative") != string::npos) {
    setTexture(BTexture::Parent_Relative);
  } else {
    setTexture(0);

    if (descr.find("gradient") != string::npos) {
      addTexture(BTexture::Gradient);
      if (descr.find("crossdiagonal") != string::npos)
        addTexture(BTexture::CrossDiagonal);
      else if (descr.find("rectangle") != string::npos)
        addTexture(BTexture::Rectangle);
      else if (descr.find("pyramid") != string::npos)
        addTexture(BTexture::Pyramid);
      else if (descr.find("pipecross") != string::npos)
        addTexture(BTexture::PipeCross);
      else if (descr.find("elliptic") != string::npos)
        addTexture(BTexture::Elliptic);
      else if (descr.find("horizontal") != string::npos)
        addTexture(BTexture::Horizontal);
      else if (descr.find("vertical") != string::npos)
        addTexture(BTexture::Vertical);
      else
        addTexture(BTexture::Diagonal);
    } else {
      addTexture(BTexture::Solid);
    }

    if (descr.find("sunken") != string::npos)
      addTexture(BTexture::Sunken);
    else if (descr.find("flat") != string::npos)
      addTexture(BTexture::Flat);
    else
      addTexture(BTexture::Raised);

    if (texture() & BTexture::Flat) {
      if (descr.find("border") != string::npos)
        addTexture(BTexture::Border);
    } else {
      if (descr.find("bevel2") != string::npos)
        addTexture(BTexture::Bevel2);
      else
        addTexture(BTexture::Bevel1);
    }

    if (descr.find("interlaced") != string::npos)
      addTexture(BTexture::Interlaced);
  }
}

void BTexture::setDisplay(const BaseDisplay * const _display,
                          const unsigned int _screen) {
  if (_display == display() && _screen == screen()) {
    // nothing to do
    return;
  }

  dpy = _display;
  scrn = _screen;
  c.setDisplay(_display, _screen);
  ct.setDisplay(_display, _screen);
  lc.setDisplay(_display, _screen);
  sc.setDisplay(_display, _screen);
  bc.setDisplay(_display, _screen);
}


BTexture& BTexture::operator=(const BTexture &tt) {
  c  = tt.c;
  ct = tt.ct;
  lc = tt.lc;
  sc = tt.sc;
  bc = tt.bc;
  descr = tt.descr;
  t  = tt.t;
  dpy = tt.dpy;
  scrn = tt.scrn;
  ctrl = tt.ctrl;

  return *this;
}


Pixmap BTexture::render(const unsigned int width, const unsigned int height,
                        const Pixmap old) {
  assert(display() != 0);
  assert(texture() != BTexture::NoTexture);

  if (texture() == (BTexture::Flat | BTexture::Solid))
    return None;
  if (texture() == BTexture::Parent_Relative)
    return ParentRelative;

  if (screen() == ~(0u))
    scrn = DefaultScreen(display()->getXDisplay());

  assert(ctrl != 0);
  Pixmap ret = ctrl->renderImage(width, height, *this);

  if (old)
    ctrl->removeImage(old);

  return ret;
}
