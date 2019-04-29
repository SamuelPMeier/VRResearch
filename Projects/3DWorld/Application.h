#pragma once
#include <memory>
#include <GL/glew.h>
#include "Model.h"
#include "Display.h"
#include "ModelMaker.h"
#include "Shader.h"

// Application class for 3D World Project
class Application {
   static Application *mApp;
   vr::IVRSystem *hmd;
 
   std::shared_ptr<Model> mMdl; // Current Model

   std::vector<std::shared_ptr<Display>> mDisplays;
   std::vector<std::shared_ptr<HMDInput>> mInputs;

   void InitSDL();
   void InitOpenVR();
   void InitOpenGL();

public:
   // Initialize libraries and set up basic entities, subject to passed
   // commandline parameters.  Initialize mApp
   Application(int argc, char **argv);

   // Loop to process events, and redraw all displays
   void Run();

   // Call these to add different kinds of display
   void AddHMDDisplay();
   void AddSimpleDisplay(int, int);
   std::string GetTrackedDeviceString(vr::IVRSystem *,
    vr::TrackedDeviceIndex_t, vr::TrackedDeviceProperty, 
    vr::TrackedPropertyError *);

   static Application *GetApp() { return mApp; }
};