#pragma once
#include "HMDInput.h"
#include "Utility.h"
#include "Shader.h"
#include <SDL.h>
#include <glm/glm.hpp>

// for creating LR framebuffers for HMD
struct FrameBuffer {
   GLuint depthBufferId;
   GLuint renderTextureId;
   GLuint renderFramebufferId;
   GLuint resolveTextureId;
   GLuint resolveFramebufferId;

   FrameBuffer() { depthBufferId  = renderTextureId =
   renderFramebufferId = resolveTextureId = resolveFramebufferId = 0;}
};

// base display class
class Display {
protected:
   // member data
   SDL_Window *mWindow;
   static SDL_GLContext mContext;

public:
   Display() {}
   virtual ~Display() {}

   static void InitContext(SDL_Window *);

   // Redraw the display, given eyepoint perpsective transform, a Shader
   // and data on GL vertex buffers to use.
   virtual void Redraw(std::shared_ptr<Shader> sdr, uint numIndices) = 0;
   virtual void CreateFBs(std::shared_ptr<HMDInput>) = 0;
   virtual void SwapWindows() = 0;
   virtual void PrepareWindow(std::shared_ptr<Shader> sdr, 
    std::shared_ptr<HMDInput>) = 0;
};

// One-window monocular view
class SimpleDisplay : public Display {
   // member data
   glm::mat4 mViewXForm;  // Camera "back off" from origin and LH -> RH shift
   glm::mat4 mPspXForm;   // Perspective transform
public:
   // Add constructor parameters as needed
   SimpleDisplay(int, int);

   void Redraw(std::shared_ptr<Shader> sdr,uint numIndices) override;
   void CreateFBs(std::shared_ptr<HMDInput>) override {};
   void SwapWindows() override;
   void PrepareWindow(std::shared_ptr<Shader> sdr, 
    std::shared_ptr<HMDInput>) override;
};

// HMD dual framebuffer display
class HMDDisplay : public Display {
   // member data
   uint mHMDDisplayHT;
   uint mHMDDisplayWD;
   glm::mat4x4 mLeftPsp;
   glm::mat4x4 mRightPsp;
   std::shared_ptr<FrameBuffer> mLeft;
   std::shared_ptr<FrameBuffer> mRight;
   glm::mat4x4 mHMDXfm;
   glm::vec3 mAbsPos;

   void CreateFrameBuffer(std::shared_ptr<FrameBuffer>);
   void RenderEye(const glm::mat4x4 &, std::shared_ptr<Shader>, uint);

public:
   // Add constructor parameters as needed
   HMDDisplay(vr::IVRSystem *);

   void Redraw(std::shared_ptr<Shader> sdr, uint numIndices) override;
   void CreateFBs(std::shared_ptr<HMDInput>) override;
   void SwapWindows() override;
   void PrepareWindow(std::shared_ptr<Shader> sdr, 
    std::shared_ptr<HMDInput>) override;
};

