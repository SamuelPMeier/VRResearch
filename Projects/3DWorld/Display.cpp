#include "Display.h"
#include "Utility.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

using namespace std;
using namespace glm;
using namespace vr;

// Singleton GL Context shared by all GL displays
SDL_GLContext Display::mContext = nullptr;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * /
Display Functions
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Initialize mContext if needed, using |window|
void Display::InitContext(SDL_Window *window) {
   if (mContext == nullptr)
      mContext = SDL_GL_CreateContext(window);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * /
SimpleDisplay Functions
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Set up a wd x ht window, assuming that SDL is already initialized.  Throw
// WorldExceptions for any difficulties.
SimpleDisplay::SimpleDisplay(int wd, int ht) {

   mWindow = SDL_CreateWindow(
    "SDL Tutorial",
    SDL_WINDOWPOS_UNDEFINED,
    SDL_WINDOWPOS_UNDEFINED,
    wd,
    ht,
    SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

   if (mWindow == NULL) {
      throw WorldException(StringPrintf
       ("Window could not be created! SDL_Err: %s\n", SDL_GetError()));
   }

   InitContext(mWindow);
   SDL_SetRelativeMouseMode(SDL_TRUE);

   mViewXForm = lookAt(
      vec3(0, 0, -0.05), // Camera is at (0, 0, -10), in World Space
      vec3(0, 0, 0),   // And looks at the origin
      vec3(0, 1, 0)    // Head is up
   );

   mPspXForm = perspective(
      0.4f,       // Field of view ( how far out to the sides we can see )
      1.0f,       // Aspect ratio ( stretch image in widh or height )
      0.5f,       // Near plane ( anything close than this will be cut off )
      100.0f      // Far plane ( anything further away than this will be cut off )
   );
}

// Redraw window content given viewTrans, shader, and num of preloaded indices
void SimpleDisplay::Redraw( shared_ptr<Shader> sdr, uint numIdx) {
   glDrawElements(GL_TRIANGLES, numIdx, GL_UNSIGNED_INT, (void*)0); GLChkErr;
}

// output pre-drawn buffer
void SimpleDisplay::SwapWindows() {
   SDL_GL_SwapWindow(mWindow);
}

// get buffer ready for drawing
void SimpleDisplay::PrepareWindow(std::shared_ptr<Shader> sdr,
 shared_ptr<HMDInput> inp) {
   glClearColor(0.1, 0.1, 0.0, 1.0);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   mat4 xfm = inp->GetViewTransform();
   vec3 absPos = vec3(xfm[3][0], xfm[3][1], xfm[3][2]);

   sdr->Run(mPspXForm * mViewXForm * xfm, absPos);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * /
HMDDisplay Functions
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// initilize and prepare bland SDL window for input
HMDDisplay::HMDDisplay(IVRSystem *HMD) {
   HMD->GetRecommendedRenderTargetSize(&mHMDDisplayWD, &mHMDDisplayHT);

   mWindow = SDL_CreateWindow(
      "SDL Tutorial",
      SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED,
      500,
      500,
      SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

   if (mWindow == NULL) {
      throw WorldException(StringPrintf
      ("Window could not be created! SDL_Err: %s\n", SDL_GetError()));
   }

   InitContext(mWindow);
}

// create the different FBs for LR displays
void HMDDisplay::CreateFBs(shared_ptr<HMDInput> input) {
   mLeft = shared_ptr<FrameBuffer>(new FrameBuffer());
   mRight = shared_ptr<FrameBuffer>(new FrameBuffer());
   mLeftPsp = input->ComputeEyePerspective(HMDInput::cLeft);
   mRightPsp = input->ComputeEyePerspective(HMDInput::cRight);
   CreateFrameBuffer(mLeft);
   CreateFrameBuffer(mRight);
}

// Create and manage a single FB for either L or R
void HMDDisplay::CreateFrameBuffer(shared_ptr<FrameBuffer> buff) {
   glGenFramebuffers(1, &buff->renderFramebufferId); GLChkErr;
   glBindFramebuffer(
    GL_FRAMEBUFFER, buff->renderFramebufferId); GLChkErr;

   glGenRenderbuffers(1, &buff->depthBufferId); GLChkErr;
   glBindRenderbuffer(GL_RENDERBUFFER, buff->depthBufferId); GLChkErr;
   glRenderbufferStorageMultisample(
    GL_RENDERBUFFER, 8, GL_DEPTH_COMPONENT, mHMDDisplayWD, mHMDDisplayHT);
   GLChkErr;
   glFramebufferRenderbuffer(
    GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
    buff->depthBufferId); GLChkErr;

   glGenTextures(1, &buff->renderTextureId); GLChkErr;
   glBindTexture(GL_TEXTURE_2D_MULTISAMPLE,
    buff->renderTextureId); GLChkErr;

   glTexImage2DMultisample(
    GL_TEXTURE_2D_MULTISAMPLE, 8, GL_RGBA8, mHMDDisplayWD, mHMDDisplayHT, 1);
   GLChkErr;

   glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
    buff->renderTextureId, 0); GLChkErr;

   glGenFramebuffers(1, &buff->resolveFramebufferId); GLChkErr;
   glBindFramebuffer(
    GL_FRAMEBUFFER, buff->resolveFramebufferId); GLChkErr;

   glGenTextures(1, &buff->resolveTextureId); GLChkErr;
   glBindTexture(GL_TEXTURE_2D, buff->resolveTextureId); GLChkErr;
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); GLChkErr;
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0); GLChkErr;
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mHMDDisplayWD, mHMDDisplayHT,
    0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr); GLChkErr;

   glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
    buff->resolveTextureId, 0); GLChkErr;

   // check FBO status
   GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER); GLChkErr;
   if (status != GL_FRAMEBUFFER_COMPLETE) {
      throw WorldException(
         "Frame Render Error");
   }

   glBindFramebuffer(GL_FRAMEBUFFER, 0); GLChkErr;
}

// redraw both FBs for LR
void HMDDisplay::Redraw(shared_ptr<Shader> sdr, uint numIdx) {
   glEnable(GL_MULTISAMPLE); GLChkErr;

   // Right Eye
   glBindFramebuffer(GL_FRAMEBUFFER, mRight->renderFramebufferId); GLChkErr;
   glViewport(0, 0, mHMDDisplayWD, mHMDDisplayHT); GLChkErr;
   RenderEye(mRightPsp * mHMDXfm, sdr, numIdx);
   glBindFramebuffer(GL_FRAMEBUFFER, 0); GLChkErr;

   glDisable(GL_MULTISAMPLE);

   glBindFramebuffer(
      GL_READ_FRAMEBUFFER, mRight->renderFramebufferId); GLChkErr;
   glBindFramebuffer(
      GL_DRAW_FRAMEBUFFER, mRight->resolveFramebufferId); GLChkErr;

   glBlitFramebuffer(0, 0, mHMDDisplayWD, mHMDDisplayHT, 0, 0,
      mHMDDisplayWD, mHMDDisplayHT, GL_COLOR_BUFFER_BIT, GL_LINEAR); GLChkErr;

   glBindFramebuffer(GL_READ_FRAMEBUFFER, 0); GLChkErr;
   glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); GLChkErr;

   glEnable(GL_MULTISAMPLE); GLChkErr;

   // Left Eye
   glBindFramebuffer(GL_FRAMEBUFFER, mLeft->renderFramebufferId); GLChkErr;
   glViewport(0, 0, mHMDDisplayWD, mHMDDisplayHT); GLChkErr;

   RenderEye(mLeftPsp * mHMDXfm, sdr, numIdx);

   glBindFramebuffer(GL_FRAMEBUFFER, 0); GLChkErr;

   glDisable(GL_MULTISAMPLE);

   glBindFramebuffer(
    GL_READ_FRAMEBUFFER, mLeft->renderFramebufferId); GLChkErr;
   glBindFramebuffer(
    GL_DRAW_FRAMEBUFFER, mLeft->resolveFramebufferId); GLChkErr;

   glBlitFramebuffer(0, 0, mHMDDisplayWD, mHMDDisplayHT, 0, 0,
    mHMDDisplayWD, mHMDDisplayHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);

   glBindFramebuffer(GL_READ_FRAMEBUFFER, 0); GLChkErr;
   glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); GLChkErr;


}

// draw either L or R framebuffers
void HMDDisplay::RenderEye(const glm::mat4x4 &eye,
 shared_ptr<Shader> sdr, uint numIdx) {
   sdr->Run(eye, mAbsPos);

   ClearConsole();
   PrintVec(mAbsPos);
   glDrawElements(GL_TRIANGLES, numIdx, GL_UNSIGNED_INT, (void*)0); GLChkErr;
}

// create textures from FBs and load to LR HMD displays
void HMDDisplay::SwapWindows() {
   Texture_t rightEyeTexture = {
      (void*)(uintptr_t)mRight->resolveTextureId, TextureType_OpenGL,
      ColorSpace_Gamma};
   VRCompositor()->Submit(Eye_Right, &rightEyeTexture);

   Texture_t leftEyeTexture = {
      (void*)(uintptr_t)mLeft->resolveTextureId, TextureType_OpenGL,
      ColorSpace_Gamma};
   VRCompositor()->Submit(Eye_Left, &leftEyeTexture);

   SDL_GL_SwapWindow(mWindow);
}

// set up both FB for new draw frames
void HMDDisplay::PrepareWindow(std::shared_ptr<Shader> sdr, 
 shared_ptr<HMDInput> inp) {
   try {
      mHMDXfm = inp->GetViewTransform();
      mAbsPos = vec3(mHMDXfm[3][0], mHMDXfm[3][1], mHMDXfm[3][2]);
      mHMDXfm = inverse(mHMDXfm);
   }
   catch(WorldException e) {
      ClearConsole();
      printf("%s\n", e.what());
   }

   glClearColor(0.1, 0.1, 0.0, 1.0);
   glEnable(GL_MULTISAMPLE); GLChkErr;

   // Left Eye
   glBindFramebuffer(GL_FRAMEBUFFER, mLeft->renderFramebufferId); GLChkErr;
   glViewport(0, 0, mHMDDisplayWD, mHMDDisplayHT); GLChkErr;

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glBindFramebuffer(GL_FRAMEBUFFER, 0); GLChkErr;

   glDisable(GL_MULTISAMPLE);

   glBindFramebuffer(
      GL_READ_FRAMEBUFFER, mLeft->renderFramebufferId); GLChkErr;
   glBindFramebuffer(
      GL_DRAW_FRAMEBUFFER, mLeft->resolveFramebufferId); GLChkErr;

   glBlitFramebuffer(0, 0, mHMDDisplayWD, mHMDDisplayHT, 0, 0,
      mHMDDisplayWD, mHMDDisplayHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);

   glBindFramebuffer(GL_READ_FRAMEBUFFER, 0); GLChkErr;
   glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); GLChkErr;

   glEnable(GL_MULTISAMPLE); GLChkErr;

   // Right Eye
   glBindFramebuffer(GL_FRAMEBUFFER, mRight->renderFramebufferId); GLChkErr;
   glViewport(0, 0, mHMDDisplayWD, mHMDDisplayHT); GLChkErr;
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glBindFramebuffer(GL_FRAMEBUFFER, 0); GLChkErr;

   glDisable(GL_MULTISAMPLE);

   glBindFramebuffer(
      GL_READ_FRAMEBUFFER, mRight->renderFramebufferId); GLChkErr;
   glBindFramebuffer(
      GL_DRAW_FRAMEBUFFER, mRight->resolveFramebufferId); GLChkErr;

   glBlitFramebuffer(0, 0, mHMDDisplayWD, mHMDDisplayHT, 0, 0,
      mHMDDisplayWD, mHMDDisplayHT, GL_COLOR_BUFFER_BIT, GL_LINEAR); GLChkErr;

   glBindFramebuffer(GL_READ_FRAMEBUFFER, 0); GLChkErr;
   glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); GLChkErr;
}