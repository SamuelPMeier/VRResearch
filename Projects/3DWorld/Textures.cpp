#include <vector>
#include <cstdlib>
#include <algorithm>

#include "Utility.h"
#include "Textures.h"
#include "lodepng.h"

using namespace std;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * /
Base Texture class
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/// base texture initilization
Texture::Texture(string n) : mName(n) {
   glGenTextures(1, &mId);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * /
PNG Texture Class
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/// Creates a texture from PNG, binds to active texture 0 (tex in shader)
TexturePng::TexturePng(string n, string fileName, bool repeat) : Texture(n) {
   uint mipMax, ht, wd;
   vector<uchar> pixels;

   auto err = lodepng::decode(pixels, wd, ht, fileName);

   if (!err) {
      glBindTexture(GL_TEXTURE_2D, mId);     // Set as 2D texture type
      GLChkErr;

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ht, wd,
       0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
      GLChkErr;

      glGenerateMipmap(GL_TEXTURE_2D);
      GLChkErr;

      // Set clamping for repeated pattern or mirrored repeat
      auto clamp = repeat ? GL_REPEAT : GL_MIRRORED_REPEAT;
      GLChkErr;
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp);
      GLChkErr;
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp);
      GLChkErr;
     
      // Expensive but visually optimal linear interpolation on both ends of
      // the resolution spectrum.
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      GLChkErr;
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
       GL_LINEAR_MIPMAP_LINEAR);   GLChkErr;

      // set AA filtering
      GLfloat fLargest;
      glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
      GLChkErr;
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
      GLChkErr;
      
      glBindTexture(GL_TEXTURE_2D, mId);
      GLChkErr;
   }
}

/// binds texture to correct active texture for shader (tex)
void TexturePng::UseTexture() {
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, mId);
   GLChkErr;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * /
Normal Map Texture Class
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/// creates normal map for correct active texture (normalMap in shader)
TextureNormal::TextureNormal(std::string n, std::string fN, bool repeat)
 : Texture(n){
   uint mipMax, ht, wd;
   vector<uchar> pixels;

   auto err = lodepng::decode(pixels, wd, ht, fN);

   // Sufficient MIP levels to bring largest dimension down to 1
   mipMax = (int)floor(log2(max(wd, ht)));

   if (!err) {
      glBindTexture(GL_TEXTURE_2D, mId);     // Set as 2D texture type
      GLChkErr;

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ht, wd,
         0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
      GLChkErr;

      glGenerateMipmap(GL_TEXTURE_2D);
      GLChkErr;

      // Set clamping for repeated pattern or mirrored repeat
      auto clamp = repeat ? GL_REPEAT : GL_MIRRORED_REPEAT;
      GLChkErr;
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp);
      GLChkErr;
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp);
      GLChkErr;

      // Expensive but visually optimal linear interpolation on both ends of
      // the resolution spectrum.
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      GLChkErr;
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
         GL_LINEAR_MIPMAP_LINEAR);
      GLChkErr;

      // set AA filtering
      GLfloat fLargest;
      glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
      GLChkErr;
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
      GLChkErr;

      glBindTexture(GL_TEXTURE_2D, mId);
      GLChkErr;
   }
}

/// Binds texture to active texture 1 for shader (normalMap)
void TextureNormal::UseTexture() {
   glActiveTexture(GL_TEXTURE1);
   glBindTexture(GL_TEXTURE_2D, mId);
   GLChkErr;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * /
Shadow Map Texture Class
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/// binds shadow map to active texture 2 in shader (shadowMap)
void TextureShadow::UseTexture() {
   glActiveTexture(GL_TEXTURE2);
   glBindTexture(GL_TEXTURE_2D, mId);
   GLChkErr;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * /
Clear Map Texture Class
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/// sets a blank texture of color clr
TextureClr::TextureClr(string n, unsigned char clr[]) : Texture(n) {
   glBindTexture(GL_TEXTURE_2D, mId);     // Set as 2D texture type
   GLChkErr;
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1,
      0, GL_RGBA, GL_UNSIGNED_BYTE, clr);
   GLChkErr;
}

/// binds a cleared texture to current active texture slot
void TextureClr::UseTexture() {
   glBindTexture(GL_TEXTURE_2D, mId);
   GLChkErr;
}
