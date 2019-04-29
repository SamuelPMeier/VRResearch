#include <windows.h>
#include <stdio.h>
#include <openvr.h>
#include <SDL.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Application.h"
#include "Utility.h"
#include "ModelMaker.h"
#include "Renderer.h"

using namespace std;
using namespace glm;
using namespace vr;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * /
Application Functions
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

string Application::GetTrackedDeviceString(IVRSystem *hmd,
   TrackedDeviceIndex_t device, TrackedDeviceProperty prop,
   TrackedPropertyError *error = NULL) {

   uint32_t requiredBufferLen = hmd->GetStringTrackedDeviceProperty
    (device, prop, NULL, 0, error);

   if (requiredBufferLen == 0)
      return "";

   char *buffer = new char[requiredBufferLen];
   requiredBufferLen = hmd->GetStringTrackedDeviceProperty
    (device, prop, buffer, requiredBufferLen, error);

   string result = buffer;
   delete[] buffer;
   return result;
}

bool BInitCompositor() {
   EVRInitError peError = VRInitError_None;

   if (!VRCompositor())
      throw WorldException(
       "Compositor initialization failed. See log file for details\n");

   return true;
}

Application::Application(int argc, char **argv) {
   unique_ptr<ModelMaker> mdlMaker;

   // Initialize SDL first.  InitOpenGL awaits commandline-induced window
   // creation.  InitOpenVR awaits possible setup of an HMD.
   InitSDL();

   EVRInitError hmdStatus = VRInitError_None;
   VR_Init(&hmdStatus, VRApplication_Scene);

   for (argv++; *argv; argv++) {
      if (!((string)*argv).compare("-D")) {
         argv++;{}
         if (!((string)*argv).compare("simple")) {
            AddSimpleDisplay(1000, 1000);
         }
         else if (!((string)*argv).compare("stereo")) {
            AddStereoDisplay();
         }
         else if (!((string)*argv).compare("hmd")) {
            AddHMDDisplay();
         }
         else {
            throw WorldException("-D requires simple, stereo, or hmd");
         }
      }
      if (!((string)*argv).compare("-S")) {
         argv++;
         if (!((string)*argv).compare("cube")) {
            argv++;
            vector<string> temp = Split(*argv, ':');
            //mdlMaker = unique_ptr<ModelMaker>(new MultiCubeMaker(stof(temp[0])));
            mdlMaker = unique_ptr<ModelMaker>(new CubeMaker(stof(temp[0])));
         }
         else if (!((string)*argv).compare("room")) {
            argv++;
            vector<string> temp = Split(*argv, ':');
            mdlMaker = unique_ptr<ModelMaker>(new RoomMaker(stof(temp[0])));
         }
         else if (!((string)*argv).compare("table")) {
            argv++;
            vector<string> temp = Split(*argv, ':');
            mdlMaker = unique_ptr<ModelMaker>(new TableMaker(
               stof(temp[0]), stof(temp[1]), stof(temp[2])));
         }
         else
            throw WorldException("-S requires cube,... ");
      }
   }

   // By this point, some -S arg should have generated a mdlMaker
   if (!mdlMaker)
      throw WorldException("No model specified");

   mMdl = mdlMaker->MakeModel();
}

// set up SDL, should only be done once
void Application::InitSDL() {
   if (SDL_Init(SDL_INIT_VIDEO) < 0)  // CAS FIX: Will we want TIMER as well?
      throw WorldException(StringPrintf(
       "SDL could not initialize! SDL_Error: %s\n", SDL_GetError()));
}

// set up OpenVR, should only be done once
void Application::InitOpenVR() {
   // Loading the SteamVR Runtime
   EVRInitError eError = VRInitError_None;
   hmd = VR_Init(&eError, VRApplication_Scene);

   if (eError != VRInitError_None)
      throw WorldException(StringPrintf(
       "Unable to init VR runtime: %s",
       VR_GetVRInitErrorAsEnglishDescription(eError)));

   if (!BInitCompositor())
      throw WorldException(StringPrintf(
       "%s - Failed to initialize VR Compositor!\n", __FUNCTION__));
}

// set all parameters for OpenGL, should only be done once
void Application::InitOpenGL() {
   glEnable(GL_BLEND); GLChkErr;
   glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST); GLChkErr;
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); GLChkErr;
   glEnable(GL_DEPTH_TEST); GLChkErr;
   glDepthFunc(GL_LEQUAL); GLChkErr;

   SDL_GL_SetAttribute(
      SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
   SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

   SDL_GL_SetSwapInterval(1);
   glewExperimental = GL_TRUE;
   auto glewRtn = glewInit();
   if (glewRtn != GLEW_OK) 
      throw WorldException(StringPrintf(
       "%s - Error initializing GLEW! %s\n", __FUNCTION__,
       glewGetErrorString(glewRtn)));

   glGetError(); // to clear the error caused deep in GLEW

   if (SDL_GL_SetSwapInterval(true ? 1 : 0) <0) 
      throw WorldException(StringPrintf(
       "%s - Warning: Unable to set VSync! SDL Error: %s\n",
       __FUNCTION__, SDL_GetError()));
}

// create a new HMD display and input, and initilize FBs
void Application::AddHMDDisplay() {
   InitOpenVR();
   mDisplays.push_back(
    shared_ptr<Display>(new HMDDisplay(hmd)));
   mInputs.push_back(
    shared_ptr<HMDInput>(new OpenVRHMDInput(hmd, 0.005f, 30.0f)));
   InitOpenGL();

   mDisplays.at(mDisplays.size()-1)->CreateFBs(mInputs.at(mInputs.size()-1));
}

// add a simple square display and input
void Application::AddSimpleDisplay(int wd, int ht) {
   mDisplays.push_back(
    shared_ptr<SimpleDisplay>(new SimpleDisplay(wd, ht)));
   mInputs.push_back(
    shared_ptr<HMDInput>(new KeyboardHMDInput(wd, ht, 0.005f, 0.01f)));
   InitOpenGL(); 
}

// create a new renderer and run, after model has been
void Application::Run() {
   Renderer(mMdl, mDisplays, mInputs).Run();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * /
Main Function
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int main(int argc, char **argv) {
   try {
      Application(argc, argv).Run();
   }
   catch (WorldException err) {
      cout << "WorldException: " << err.what() << endl;
      system("pause");
      return 1;
   }
   catch (exception err) {
      cout << "General exception: " << err.what() << endl;
      system("pause");
      return 1;
   }

   return 0;
}

