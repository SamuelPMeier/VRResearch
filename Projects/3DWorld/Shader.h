#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <GL/glew.h>

#include "Utility.h"

/// position and intesity of multiple light sources in scene
struct LightSource {
   glm::vec4 location;
   glm::vec3 intensity;

   LightSource(glm::vec4 pos, glm::vec3 clr) : location(pos), intensity(clr) {}
};

/// single instance shader class
class Shader {
protected:
   std::vector<LightSource> mLights;
   GLuint mProgramID;
   GLuint mShdwPID;
   GLuint mTexLoc;
   GLuint mNormalMap;

   static GLuint CompileShader(const char *, GLenum);
   GLuint LinkShaders(std::vector<GLuint>);

public:
   Shader();

   void UseShader() {glUseProgram(mProgramID);}
   void Configure(std::vector<LightSource>, glm::mat4);
   void Run(const glm::mat4x4 &, glm::vec3);
   void SetNMap(bool);
   void SetShadowMap(glm::mat4);
};