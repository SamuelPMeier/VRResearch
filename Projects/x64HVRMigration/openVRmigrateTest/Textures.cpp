#include <vector>
#include <cstdlib>
#include <algorithm>
#include "MyLib.h"
#include "Textures.h"
#include "lodepng.h"

using namespace std;

Texture::Texture(string n) : mName(n) {
   glGenTextures(1, &mId);
}

TexturePng::TexturePng(string n, string fileName, bool repeat) : Texture(n) {
   uint mipMax, ht, wd, clamp;
   vector<uchar> pixels;

   auto err = lodepng::decode(pixels, wd, ht, fileName);

   // Sufficient MIP levels to bring largest dimension down to 1
   mipMax = (int) floor(log2(max(wd, ht)));

   if (!err) {
      glBindTexture(GL_TEXTURE_2D, mId);     // Set as 2D texture type
      glTexImage2D(GL_TEXTURE_2D, mipMax, GL_RGBA, ht, wd,
       0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

      glGenerateMipmap(GL_TEXTURE_2D);
     
      // Set clamping for repeated pattern or single instance
      clamp = repeat ? GL_MIRRORED_REPEAT : GL_CLAMP_TO_EDGE;
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp);
     
      // Expensive but visually optimal linear interpolation on both ends of
      // the resolution spectrum.
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
       GL_LINEAR_MIPMAP_LINEAR);

      GLfloat fLargest;
      glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);

      
      glBindTexture(GL_TEXTURE_2D, 0);
   }
}

TextureClr::TextureClr(string n, unsigned char clr[]) : Texture(n) {
   glBindTexture(GL_TEXTURE_2D, mId);     // Set as 2D texture type
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1,
      0, GL_RGBA, GL_UNSIGNED_BYTE, clr);
}
