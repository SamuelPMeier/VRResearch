#pragma once
#include <vector>
#include <SDL.h>
#include <openvr.h>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>



class HMDInput {
public:
   enum Eye {cLeft, cRight};

   HMDInput() {};
   virtual ~HMDInput() {};

   // Return a viewport transform suitable for the indicated eye
   virtual glm::mat4x4 GetViewTransform() = 0;
   virtual int FieldEvent(SDL_Event) = 0 {};
   virtual void FieldVREvent(vr::VREvent_t) {};
   virtual glm::mat4x4 ComputeEyePerspective(Eye)  = 0;
};

// VR input method, for HMD and controllers
class OpenVRHMDInput : public HMDInput {
protected:
   float mNearPlain;
   float mFarPlain;
   vr::IVRSystem *mHmd;
   double seconds, pastSec = 0;
   int reports;


   static glm::mat4x4 SteamVRPoseToMat4x4(const vr::HmdMatrix34_t &pose);
public:
   OpenVRHMDInput(vr::IVRSystem *, float, float);
   glm::mat4x4 ComputeEyePerspective(Eye) override;
   glm::mat4x4 GetViewTransform();
   void FieldVREvent(vr::VREvent_t) override;
   int FieldEvent(SDL_Event) override;
};

// keyboard input, for Simple display only
class KeyboardHMDInput : public HMDInput {
protected:
   glm::vec3 mLoc, mRot;
   float mTransSensitivity = 0.05f, mRotSensitivity = 0.001f;
   int mWinHeight, mWinWidth;
public:
   KeyboardHMDInput(int, int, float, float);

   glm::mat4x4 GetViewTransform();
   glm::mat4x4 ComputeEyePerspective(Eye) override {return glm::mat4(1.0f);}
   int FieldEvent(SDL_Event) override ;
};
