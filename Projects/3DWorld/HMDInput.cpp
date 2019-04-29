#include <openvr.h>
#include <time.h>

#include "HMDInput.h"
#include "Utility.h"

using namespace glm;
using namespace vr;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * /
OpenVRHMDInput Functions
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// initilize VR system
OpenVRHMDInput::OpenVRHMDInput(vr::IVRSystem *hmd, float nP, float fP)
   : mHmd(hmd), mNearPlain(nP), mFarPlain(fP) {
   if (!VR_IsHmdPresent())
      throw WorldException("HMD Not Present");
}

// Column-Row conversion, adding extra Column
mat4x4 OpenVRHMDInput::SteamVRPoseToMat4x4(const HmdMatrix34_t &pose) {
   return mat4x4(
      pose.m[0][0], pose.m[1][0], pose.m[2][0], 0.0,
      pose.m[0][1], pose.m[1][1], pose.m[2][1], 0.0,
      pose.m[0][2], pose.m[1][2], pose.m[2][2], 0.0,
      pose.m[0][3], pose.m[1][3], pose.m[2][3], 1.0f
   );
}

// Get fixed eye-to-head transform, convert it to GLM format, and invert
// it, returning a head-to-eye transform as a mat4x4
mat4x4 OpenVRHMDInput::ComputeEyePerspective(Eye eye) {

   // Get Perspective matrix for indicated eye
   HmdMatrix44_t ovrPrj = 
    mHmd->GetProjectionMatrix((vr::EVREye)eye, mNearPlain, mFarPlain);

   // Build corresponding column-major GLM mat4x4 matrix
   mat4x4 glmPrj(
      vec4(ovrPrj.m[0][0], ovrPrj.m[1][0], ovrPrj.m[2][0], ovrPrj.m[3][0]),
      vec4(ovrPrj.m[0][1], ovrPrj.m[1][1], ovrPrj.m[2][1], ovrPrj.m[3][1]),
      vec4(ovrPrj.m[0][2], ovrPrj.m[1][2], ovrPrj.m[2][2], ovrPrj.m[3][2]),
      vec4(ovrPrj.m[0][3], ovrPrj.m[1][3], ovrPrj.m[2][3], ovrPrj.m[3][3])
   );

   // Get eye-to-head transform
   HmdMatrix34_t ovrE2H = mHmd->GetEyeToHeadTransform((vr::EVREye) eye);

   // Build corresponding mat4x4 eye-to-head, invert it to head-to-eye,
   // and multiply by perspective transform to get eye perspective transform.
   return glmPrj * inverse(SteamVRPoseToMat4x4(ovrE2H));
}

// returns center position of HMD
mat4x4 OpenVRHMDInput::GetViewTransform() {
   TrackedDevicePose_t devicePoses[k_unMaxTrackedDeviceCount], *hmdPose;

   VRCompositor()->WaitGetPoses(devicePoses, k_unMaxTrackedDeviceCount, NULL, 0);

   hmdPose = devicePoses + k_unTrackedDeviceIndex_Hmd;
   if (hmdPose->bPoseIsValid) {
      return SteamVRPoseToMat4x4(hmdPose->mDeviceToAbsoluteTracking);
   }
   else
      throw WorldException("No HMD Pose");
}

// Input for controllers, whenever
void OpenVRHMDInput::FieldVREvent(VREvent_t) {
   // Something probably goes here...
}

// check for esc
int OpenVRHMDInput::FieldEvent(SDL_Event evt) {
   int keysPressed;
   const Uint8 *keyEvents = SDL_GetKeyboardState(&keysPressed);

   if (keysPressed) {
      if (keyEvents[SDL_SCANCODE_ESCAPE])
         return 1;
   }
   return 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * /
KeyboardHMDInput Functions
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// initilization for KB input
KeyboardHMDInput::KeyboardHMDInput
 (int winHt, int winWd, float keyPress, float mSensitivity)
 : mTransSensitivity(keyPress), mRotSensitivity(mSensitivity),
   mLoc(0, 0, 0), mRot(0, 0, 0) {

   mWinHeight = winHt / (2);
   mWinWidth = winWd / (2);
}

// Interperates KB presses, changes specific values
int KeyboardHMDInput::FieldEvent(SDL_Event evt) {
   int keysPressed;
   float yDist = 0, zDist = 0;
   const Uint8 *keyEvents = SDL_GetKeyboardState(&keysPressed);

   if (evt.type == SDL_MOUSEMOTION) {
      mRot.y += mRotSensitivity * evt.motion.xrel;
      mRot.z -= mRotSensitivity * evt.motion.yrel;
   }

   if (keysPressed) {
      if (keyEvents[SDL_SCANCODE_W])
         mLoc.y -= mTransSensitivity;
      if (keyEvents[SDL_SCANCODE_S])
         mLoc.y += mTransSensitivity;
      if (keyEvents[SDL_SCANCODE_A])
         mLoc.x -= mTransSensitivity;
      if (keyEvents[SDL_SCANCODE_D])
         mLoc.x += mTransSensitivity;
      if (keyEvents[SDL_SCANCODE_Q])
         mLoc.z -= mTransSensitivity;
      if (keyEvents[SDL_SCANCODE_E])
         mLoc.z += mTransSensitivity;
      if (keyEvents[SDL_SCANCODE_F])
         mRot.x -= mTransSensitivity;
      if (keyEvents[SDL_SCANCODE_G])
         mRot.x += mTransSensitivity;
      if (keyEvents[SDL_SCANCODE_R])
         mRot.y -= mTransSensitivity;
      if (keyEvents[SDL_SCANCODE_T])
         mRot.y += mTransSensitivity;
      if (keyEvents[SDL_SCANCODE_V])
         mRot.z -= mTransSensitivity;
      if (keyEvents[SDL_SCANCODE_B])
         mRot.z += mTransSensitivity;
      if (keyEvents[SDL_SCANCODE_P])
         mRot.x = mRot.z = mRot.y = mLoc.x = mLoc.y = mLoc.z = 0;
      if (keyEvents[SDL_SCANCODE_ESCAPE])
         return 1;
   }
   return 0;
}

// returns per frame transformation, based on all input events prior
mat4x4 KeyboardHMDInput::GetViewTransform() {

   mat4x4 pose(
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      mLoc.x, mLoc.y, mLoc.z, 1
   );

   mat4x4 yaw(
      1, 0, 0, 0,
      0, cos(mRot.z), sin(mRot.z), 0,
      0, -sin(mRot.z), cos(mRot.z), 0,
      0, 0, 0, 1
   );

   mat4x4 pitch(
      cos(mRot.y), 0, -sin(mRot.y), 0,
      0, 1, 0, 0,
      sin(mRot.y), 0, cos(mRot.y), 0,
      0, 0, 0, 1
   );

   mat4x4 roll(
      cos(mRot.x), -sin(mRot.x), 0, 0,
      sin(mRot.x), cos(mRot.x), 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1
   );

   return yaw * pitch * roll * pose;
}
