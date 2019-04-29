#pragma once
#include <string>
#include <GL/glew.h>
#include "MyLib.h"

// Base Texture type with GL handle for the texture, and a string name
// for readable identification
class Texture {
protected:
   GLuint mId;
   std::string mName;

public:
   Texture(std::string);
};

// Texture subclass initialized by a png file.  Presumed use is either for
// a single repetition, in which case clamping is to the edge, or for a
// repeated pattern, in which case clamping is mirrored repeat (so that
// inter-tile edges are continuous)
class TexturePng : public Texture {
public:
   TexturePng(std::string n, std::string fN, bool repeat);
};

// Texture subclass initialized by a single color
class TextureClr : public Texture {
public:
   // Name and 4-element RGBA color array
   TextureClr(std::string n, unsigned char clr[]);
};