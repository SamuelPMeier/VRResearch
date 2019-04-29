#pragma once
#include <vector>
#include <glm/mat4x4.hpp>

#include "Display.h"
#include "Model.h"
#include "Shader.h"

class Renderer {
protected:
   // Member Data
   std::vector<std::shared_ptr<Display>> mDisplays;
   std::vector<std::shared_ptr<HMDInput>> mInputs;
   std::vector<std::shared_ptr<Texture>> mTexs;
   std::vector<std::shared_ptr<Texture>> mTexsNormal;
   std::vector<uint> mIndSizes;
   std::vector<GLuint> mVAOs, mElmBuffs, mVBOs;
   std::vector<LightSource> mLightSources;

   std::shared_ptr<Texture> mDepthTex;
   glm::mat4 mLSM;
   std::shared_ptr<Model> mMdl;
   std::shared_ptr<Shader> mSdr;
   std::shared_ptr<Shader> mShadowShader;
   SDL_GLContext *mContext;
   uint mShadowMap;

   // Private Functions
   void RenderDisplay(std::shared_ptr<Display>, std::shared_ptr<HMDInput>);
   void RenderShadowMap();
   int HandleInput(std::shared_ptr<HMDInput>, SDL_Event*);
   void CreateBuffers();
   void CreateShader();
   void CreateShadowMap();

public:
   // Configure Renderer to use indicated model, displays, and HMDInput.
   // Initialize shader automatically since we have only one type.
   Renderer(std::shared_ptr<Model>, std::vector<std::shared_ptr<Display>>,
    std::vector<std::shared_ptr<HMDInput>>);

   // Immediately draw current image.  Respond to perspective-change events
   // from HMDInput by adjusting mvp, reconfiguring the Shader, and redrawing.
   void Run();
};