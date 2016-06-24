#include <stdio.h>
#include "exif.h"

int Orientation(const char* filename) {
  // Read the JPEG file into a buffer
  FILE *fp = fopen(filename, "rb");
  if (!fp) {
    return -1;
  }
  fseek(fp, 0, SEEK_END);
  unsigned long fsize = ftell(fp);
  rewind(fp);
  unsigned char *buf = new unsigned char[fsize];
  if (fread(buf, 1, fsize, fp) != fsize) {
    delete[] buf;
    return -2;
  }
  fclose(fp);

  // Parse EXIF
  easyexif::EXIFInfo result;
  int code = result.parseFrom(buf, fsize);
  delete[] buf;
  if (code) {
    return -3;
  }
  return result.Orientation;
}
