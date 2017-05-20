/* loadppm.h */

#ifndef __LOADPPM_H
#define __LOADPPM_H

#include <iostream>
#include <fstream>
#include <math.h>
#if defined(_WIN32)
#include <windows.h>
#endif
#include <GL/gl.h>

using namespace std;

class PPMImage {
public:
  int sizeX, sizeY;
  GLubyte *data;
  PPMImage(const char *filename);
  ~PPMImage(){
    if (data)
      delete [] data;
  }
};

#endif
