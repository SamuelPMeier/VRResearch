#pragma once

#include <string>
#include <GL/glew.h>

#include "Utility.h"

// Base Texture type with GL handle for the texture, and a string name
// for readable identification
class Texture {
protected:
   GLuint mId;
   std::string mName;

public:
   Texture(std::string);

   virtual void UseTexture() = 0;
   std::string GetName() {return mName;}
   GLuint GetId() {return mId;}
};

// Texture subclass initialized by a png file. Presumed use is either for
// a single repetition, in which case repeated stamping, or for a
// repeated pattern, in which case clamping is mirrored repeat (so that
// inter-tile edges are continuous)
class TexturePng : public Texture {
public:
   TexturePng(std::string n, std::string fN, bool repeat);
   void UseTexture() override;
};

// Class for normal maps or bump maps, must be GL_REPEAT
class TextureNormal : public Texture {
public:
   TextureNormal(std::string, std::string, bool);
   void UseTexture() override;
};

// Class for Shadow Maps, must be GL_REPEAT
class TextureShadow : public Texture {
public:
   TextureShadow(GLuint t, std::string n) : Texture(n) {mId = t;};
   void UseTexture() override;
};

// Texture subclass initialized by a single color
class TextureClr : public Texture {
public:
   // Name and 4-element RGBA color array
   TextureClr(std::string n, unsigned char clr[]);
   void UseTexture() override;
};