//========= Copyright Valve Corporation ============//

#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <stdio.h>
#include <string>
#include <windows.h>
#include <openvr.h>

#include "lodepng.h"
#include "compat.h"
#include "Matrices.h"
#include "pathtools.h"
#include "Model.h"

#if defined(POSIX)
#include "unistd.h"
#endif

#ifndef _WIN32
#define APIENTRY
#endif

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof((x)[0]))
#endif

using namespace vr;
using namespace glm;
using namespace std;

void ThreadSleep(unsigned long Milliseconds) {
   SDL_Delay(Milliseconds);
}

#define BreakPoint(x) if (x) __debugbreak();
#define glCall(x) \
   x;\
   BreakPoint(glError())

static vector<float> angles = { 1.5708f, 3.14159f, 4.71239f, 0.0f };


static bool glError() {
   string error;
   bool err = true;

   GLenum errorCode = glGetError();

   switch (errorCode) {
   case GL_NO_ERROR:
      return false;
   case GL_INVALID_OPERATION:      
      error = "INVALID_OPERATION";
      break;
   case GL_INVALID_ENUM:          
      error = "INVALID_ENUM";
      break;
   case GL_INVALID_VALUE:          
      error = "INVALID_VALUE";
      break;
   case GL_OUT_OF_MEMORY:          
      error = "OUT_OF_MEMORY";
      break;
   case GL_INVALID_FRAMEBUFFER_OPERATION:
      error = "INVALID_FRAMEBUFFER_OPERATION";  
      break;

   default:
      error = " :Unkown Error, Check Code";
      cout << errorCode;
      break;
   }

   cout << error << endl;
   return err;
}

class CGLRenderModel {
public:
	CGLRenderModel(const string & sRenderModelName);
	~CGLRenderModel();

	bool BInit(const RenderModel_t & vrModel, 
    const RenderModel_TextureMap_t & vrDiffuseTexture);
	void Cleanup();
	void Draw();
	const string & GetName() const {return ModelName;}

private:
	GLuint VertBuffer;
	GLuint IndexBuffer;
	GLuint VertArray;
	GLuint Texture;
	GLsizei VertexCount;
	string ModelName;
};

static bool PrintfBool = true;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CMainApplication
{
public:
	CMainApplication(int argc, char *argv[]);
	virtual ~CMainApplication();

	bool BInit();
	bool BInitGL();
	bool BInitCompositor();

	void SetupRenderModels();

	void Shutdown();

	void RunMainLoop();
	bool HandleInput();
	void ProcessVREvent(const VREvent_t & event);
	void RenderFrame();

	bool SetupTexturemaps();

	void SetupScene();
	void AddCubeToScene(Matrix4 mat, vector<float> &vertdata);
	void AddCubeVertex(float fl0, float fl1, float fl2, float fl3, float fl4, 
    vector<float> &vertdata);

	void RenderControllerAxes();

	bool SetupStereoRenderTargets();
	void SetupCompanionWindow();
	void SetupCameras();

	void RenderStereoTargets();
	void RenderCompanionWindow();
	void RenderScene(Hmd_Eye Eye);

	Matrix4 GetHMDMatrixProjectionEye(Hmd_Eye Eye);
	Matrix4 GetHMDMatrixPoseEye(Hmd_Eye Eye);
	Matrix4 GetCurrentViewProjectionMatrix(Hmd_Eye Eye);
	void UpdateHMDMatrixPose();

	Matrix4 ConvertSteamVRMatrixToMatrix4(const HmdMatrix34_t &Pose);

	GLuint CompileGLShader(const char *ShaderName, const char 
    *VertexShader, const char *FragmentShader);

	bool CreateAllShaders();

	void SetupRenderModelForTrackedDevice(
    TrackedDeviceIndex_t rackedDeviceIndex);

	CGLRenderModel *FindOrLoadRenderModel(const char *pchRenderModelName);

private: 
	bool DebugOpenGL;
	bool Verbose;
	bool Perf;
	bool Vblank;
	bool FinishHack;

	IVRSystem *HMD;
	IVRRenderModels *RenderModels;
	string Driver;
	string Display;
	TrackedDevicePose_t TrackedDevicePose
    [k_unMaxTrackedDeviceCount];
	Matrix4 DevicePose[k_unMaxTrackedDeviceCount];
	bool ShowTrackedDevice[k_unMaxTrackedDeviceCount];

private: // SDL bookkeeping
	SDL_Window *CompanionWindow;
	uint32_t CompanionWindowWidth;
	uint32_t CompanionWindowHeight;

	SDL_GLContext Context;

private: // OpenGL bookkeeping
	int TrackedControllerCount;
	int TrackedControllerCount_Last;
	int ValidPoseCount;
	int ValidPoseCount_Last;
	bool ShowCubes;

   // what classes we saw poses for this frame
	string PoseClasses;                   
   // for each device, a character representing its class
	char DevClassChar[k_unMaxTrackedDeviceCount];   

	int SceneVolumeWidth;
	int SceneVolumeHeight;
	int SceneVolumeDepth;
	float ScaleSpacing;
	float Scale;
	
   // if you want something other than the default 20x20x20
	int SceneVolumeInit;                                 
	
	float NearClip;
	float FarClip;

	GLuint Texture;

	unsigned int Vertcount;

   CircleCylinderModel cyl;

	GLuint SceneVertBuffer;
	GLuint SceneVAO;
	GLuint CompanionWindowVAO;
	GLuint CompanionWindowIDVertBuffer;
	GLuint CompanionWindowIDIndexBuffer;
	unsigned int CompanionWindowIndexSize;

	GLuint ControllerVertBuffer;
	GLuint ControllerVAO;
	unsigned int ControllerVertcount;

	Matrix4 HMDPose;
	Matrix4 eyePosLeft;
	Matrix4 eyePosRight;

	Matrix4 ProjectionCenter;
	Matrix4 ProjectionLeft;
	Matrix4 ProjectionRight;

	struct VertexDataScene {
		Vector3 position;
		Vector2 texCoord;
	};

	struct VertexDataWindow {
		Vector2 position;
		Vector2 texCoord;

		VertexDataWindow(const Vector2 & pos, const Vector2 tex) 
       :  position(pos), texCoord(tex) {	}
	};

	GLuint SceneProgramID;
	GLuint CompanionWindowProgramID;
	GLuint ControllerTransformProgramID;
	GLuint RenderModelProgramID;

	GLint SceneMatrixLocation;
	GLint ControllerMatrixLocation;
	GLint RenderModelMatrixLocation;

	struct FramebufferDesc {
		GLuint DepthBufferId;
		GLuint RenderTextureId;
		GLuint RenderFramebufferId;
		GLuint ResolveTextureId;
		GLuint ResolveFramebufferId;
	};

	FramebufferDesc leftEyeDesc;
	FramebufferDesc rightEyeDesc;

	bool CreateFrameBuffer(int nWidth, int nHeight, 
    FramebufferDesc &framebufferDesc);
	
	uint32_t RenderWidth;
	uint32_t RenderHeight;

	vector<CGLRenderModel * > RenderModelVectors;
	CGLRenderModel *TrackedDeviceToRenderModel
    [k_unMaxTrackedDeviceCount];
};

//-----------------------------------------------------------------------------
// Purpose: Outputs a set of optional arguments to debugging output, using
//          the printf format setting specified in fmt*.
//-----------------------------------------------------------------------------
void dprintf(const char *fmt, ...) {
	va_list args;
	char buffer[2048];

	va_start(args, fmt);
	vsprintf_s(buffer, fmt, args);
	va_end(args);

	if (PrintfBool)
		printf("%s", buffer);

	OutputDebugStringA(buffer);
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMainApplication::CMainApplication(int argc, char *argv[])
	: CompanionWindow(NULL)
	, Context(NULL)
	, CompanionWindowWidth(640)
	, CompanionWindowHeight(320)
	, SceneProgramID(0)
	, CompanionWindowProgramID(0)
	, ControllerTransformProgramID(0)
	, RenderModelProgramID(0)
	, HMD(NULL)
	, RenderModels(NULL)
	, DebugOpenGL(false)
	, Verbose(false)
	, Perf(false)
	, Vblank(false)
	, FinishHack(true)
	, ControllerVertBuffer(0)
	, ControllerVAO(0)
	, SceneVAO(0)
	, SceneMatrixLocation(-1)
	, ControllerMatrixLocation(-1)
	, RenderModelMatrixLocation(-1)
	, TrackedControllerCount(0)
	, TrackedControllerCount_Last(-1)
	, ValidPoseCount(0)
	, ValidPoseCount_Last(-1)
	, SceneVolumeInit(20)
	, PoseClasses("")
	, ShowCubes(true) 
   , cyl("Test", NULL, 1, 1, angles) {

	for(int i = 1; i <argc; i++) {
		if(!_stricmp(argv[i], "-gldebug"))
			DebugOpenGL = true;

		else if(!_stricmp(argv[i], "-verbose"))
			Verbose = true;

		else if(!_stricmp(argv[i], "-novblank"))
			Vblank = false;

		else if(!_stricmp(argv[i], "-noglfinishhack"))
			FinishHack = false;

		else if(!_stricmp(argv[i], "-noprintf"))
			PrintfBool = false;

		else if (!_stricmp(argv[i], "-cubevolume") 
       && (argc > i + 1) && (*argv[i + 1] != '-')) {

			SceneVolumeInit = atoi(argv[i + 1]);
			i++;
		}
	}
	// other initialization tasks are done in BInit
	memset(DevClassChar, 0, sizeof(DevClassChar));
};


//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CMainApplication::~CMainApplication() {
	// work is done in Shutdown
	dprintf("Shutdown");
}


//-----------------------------------------------------------------------------
// Purpose: Helper to get a string from a tracked device property and turn it
//			into a string
//-----------------------------------------------------------------------------
string GetTrackedDeviceString(IVRSystem *Hmd, 
 TrackedDeviceIndex_t Device, TrackedDeviceProperty prop, 
 TrackedPropertyError *Error = NULL) {

	uint32_t RequiredBufferLen = Hmd->GetStringTrackedDeviceProperty
   (Device, prop, NULL, 0, Error);

	if(RequiredBufferLen == 0)
		return "";

	char *Buffer = new char[RequiredBufferLen];
	RequiredBufferLen = Hmd->GetStringTrackedDeviceProperty
   (Device, prop, Buffer, RequiredBufferLen, Error);

	string Result = Buffer;
	delete [] Buffer;
	return Result;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::BInit() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) <0) {
		printf("%s - SDL could not initialize! SDL Error: %s\n", 
       __FUNCTION__, SDL_GetError());
		return false;
	}

	// Loading the SteamVR Runtime
	EVRInitError eError = VRInitError_None;
	HMD = VR_Init(&eError, VRApplication_Scene);

	if (eError != VRInitError_None) {
		HMD = NULL;
		char buf[1024];
		sprintf_s(buf, sizeof(buf), "Unable to init VR runtime: %s", 
       VR_GetVRInitErrorAsEnglishDescription(eError));
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "VR_Init Failed", 
       buf, NULL);
		return false;
	}


	RenderModels = (IVRRenderModels *)VR_GetGenericInterface
   (IVRRenderModels_Version, &eError);
	if(!RenderModels) {
		HMD = NULL;
		VR_Shutdown();

		char buf[1024];
		sprintf_s(buf, sizeof(buf), 
       "Unable to get render model interface: %s", 
       VR_GetVRInitErrorAsEnglishDescription(eError));
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, 
       "VR_Init Failed", buf, NULL);
		return false;
	}

	int WindowPosX = 700;
	int WindowPosY = 100;
	Uint32 WindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	//SDL_GL_SetAttribute
   (SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	SDL_GL_SetAttribute
   (SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
	if(DebugOpenGL)
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

	CompanionWindow = SDL_CreateWindow("hellovr", WindowPosX, WindowPosY, 
    CompanionWindowWidth, CompanionWindowHeight, WindowFlags);

	if (CompanionWindow == NULL) {
		printf("%s - Window could not be created! SDL Error: %s\n", 
       __FUNCTION__, SDL_GetError());
		return false;
	}

	Context = SDL_GL_CreateContext(CompanionWindow);
	if (Context == NULL) {
		printf("%s - OpenGL context could not be created! SDL Error: %s\n", 
       __FUNCTION__, SDL_GetError());

		return false;
	}

	glewExperimental = GL_TRUE;
	GLenum GlewError = glewInit();
	if (GlewError != GLEW_OK) {
		printf("%s - Error initializing GLEW! %s\n", __FUNCTION__, 
       glewGetErrorString(GlewError));
		return false;
	}
	glGetError(); // to clear the error caused deep in GLEW

	if (SDL_GL_SetSwapInterval(Vblank ? 1 : 0) <0) {
		printf("%s - Warning: Unable to set VSync! SDL Error: %s\n", 
       __FUNCTION__, SDL_GetError());
		return false;
	}


	Driver = "No Driver";
	Display = "No Display";

	Driver = GetTrackedDeviceString(
    HMD, k_unTrackedDeviceIndex_Hmd, 
    Prop_TrackingSystemName_String);
	Display = GetTrackedDeviceString(
    HMD, k_unTrackedDeviceIndex_Hmd, Prop_SerialNumber_String);

	string strWindowTitle 
    = "hellovr - " + Driver + " " + Display;
	SDL_SetWindowTitle(CompanionWindow, strWindowTitle.c_str());
	
	// cube array
 	SceneVolumeWidth = SceneVolumeInit;
 	SceneVolumeHeight = SceneVolumeInit;
 	SceneVolumeDepth = SceneVolumeInit;
 		
 	Scale = 0.3f;
 	ScaleSpacing = 4.0f;
 
 	NearClip = 0.1f;
 	FarClip = 30.0f;
 
 	Texture = 0;
 	Vertcount = 0;
 
// 		m_MillisecondsTimer.start(1, this);
// 		m_SecondsTimer.start(1000, this);
	
	if (!BInitGL()) {
		printf("%s - Unable to initialize OpenGL!\n", __FUNCTION__);
		return false;
	}

	if (!BInitCompositor()) {
		printf("%s - Failed to initialize VR Compositor!\n", __FUNCTION__);
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Outputs the string in message to debugging output.
//          All other parameters are ignored.
//          Does not return any meaningful value or reference.
//-----------------------------------------------------------------------------
void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, 
 GLenum severity, GLsizei length, const char* message, const void* userParam) {
	dprintf("GL Error: %s\n", message);
}


//-----------------------------------------------------------------------------
// Purpose: Initialize OpenGL. Returns true if OpenGL has been successfully
//          initialized, false if shaders could not be created.
//          If failure occurred in a module other than shaders, the function
//          may return true or throw an error. 
//-----------------------------------------------------------------------------
bool CMainApplication::BInitGL() {
	if(DebugOpenGL) {
		glDebugMessageCallback((GLDEBUGPROC)DebugCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0,
       nullptr, GL_TRUE);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}

	if(!CreateAllShaders())
		return false;

	SetupTexturemaps();
	SetupScene();
	SetupCameras();
	SetupStereoRenderTargets();
	SetupCompanionWindow();
	SetupRenderModels();

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Initialize Compositor. Returns true if the compositor was
//          successfully initialized, false otherwise.
//-----------------------------------------------------------------------------
bool CMainApplication::BInitCompositor() {
	EVRInitError peError = VRInitError_None;

	if (!VRCompositor()) {
		printf("Compositor initialization failed. See log file for details\n");
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::Shutdown() {
	if(HMD) {
		VR_Shutdown();
		HMD = NULL;
	}

	for(vector<CGLRenderModel * >::iterator i = RenderModelVectors.begin(); 
    i != RenderModelVectors.end(); i++) {

		delete (*i);
	}
	RenderModelVectors.clear();
	
	if(Context) {
		if(DebugOpenGL) {
			glDebugMessageControl(
          GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE);

			glDebugMessageCallback(nullptr, nullptr);
		}
		glDeleteBuffers(1, &SceneVertBuffer);

		if (SceneProgramID) {
			glDeleteProgram(SceneProgramID);
		}
		if (ControllerTransformProgramID) {
			glDeleteProgram(ControllerTransformProgramID);
		}
		if (RenderModelProgramID) {
			glDeleteProgram(RenderModelProgramID);
		}
		if (CompanionWindowProgramID) {
			glDeleteProgram(CompanionWindowProgramID);
		}

		glDeleteRenderbuffers(1, &leftEyeDesc.DepthBufferId);
		glDeleteTextures(1, &leftEyeDesc.RenderTextureId);
		glDeleteFramebuffers(1, &leftEyeDesc.RenderFramebufferId);
		glDeleteTextures(1, &leftEyeDesc.ResolveTextureId);
		glDeleteFramebuffers(1, &leftEyeDesc.ResolveFramebufferId);

		glDeleteRenderbuffers(1, &rightEyeDesc.DepthBufferId);
		glDeleteTextures(1, &rightEyeDesc.RenderTextureId);
		glDeleteFramebuffers(1, &rightEyeDesc.RenderFramebufferId);
		glDeleteTextures(1, &rightEyeDesc.ResolveTextureId);
		glDeleteFramebuffers(1, &rightEyeDesc.ResolveFramebufferId);

		if(CompanionWindowVAO != 0) {
			glDeleteVertexArrays(1, &CompanionWindowVAO);
		}
		if(SceneVAO != 0) {
			glDeleteVertexArrays(1, &SceneVAO);
		}
		if(ControllerVAO != 0) {
			glDeleteVertexArrays(1, &ControllerVAO);
		}
	}

	if(CompanionWindow) {
		SDL_DestroyWindow(CompanionWindow);
		CompanionWindow = NULL;
	}

	SDL_Quit();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::HandleInput() {
	SDL_Event Event;
	bool QuitBool = false;

	while (SDL_PollEvent(&Event) != 0) {
		if (Event.type == SDL_QUIT) {
			QuitBool = true;
		}
		else if (Event.type == SDL_KEYDOWN) {
			if (Event.key.keysym.sym == SDLK_ESCAPE 
			     || Event.key.keysym.sym == SDLK_q) {

				QuitBool = true;
			}
			if(Event.key.keysym.sym == SDLK_c) {
				ShowCubes = !ShowCubes;
			}
		}
	}

	// Process SteamVR events
	VREvent_t event;
	while(HMD->PollNextEvent(&event, sizeof(event))) {
		ProcessVREvent(event);
	}

	// Process SteamVR controller state
	for(TrackedDeviceIndex_t DeviceCount = 0;
    DeviceCount <k_unMaxTrackedDeviceCount; DeviceCount++) {

		VRControllerState_t state;
		if(HMD->GetControllerState(DeviceCount, &state, sizeof(state))) {
			ShowTrackedDevice[DeviceCount] = state.ulButtonPressed == 0;
		}
	}

	return QuitBool;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::RunMainLoop() {
	bool bQuit = false;

	SDL_StartTextInput();
	SDL_ShowCursor(SDL_DISABLE);

	while (!bQuit) {
		bQuit = HandleInput();

		RenderFrame();
	}

	SDL_StopTextInput();
}


//-----------------------------------------------------------------------------
// Purpose: Processes a single VR event
//-----------------------------------------------------------------------------
void CMainApplication::ProcessVREvent(const VREvent_t & event) {
	switch(event.eventType) {
	case VREvent_TrackedDeviceActivated:
      SetupRenderModelForTrackedDevice(event.trackedDeviceIndex);
      dprintf("Device %u attached. Setting up render model.\n", 
       event.trackedDeviceIndex);
		break;
	case VREvent_TrackedDeviceDeactivated:
		dprintf("Device %u detached.\n", event.trackedDeviceIndex);
		break;
	case VREvent_TrackedDeviceUpdated:
		dprintf("Device %u updated.\n", event.trackedDeviceIndex);
		break;
	}
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::RenderFrame() {
	// for now as fast as possible
	if (HMD) {
		RenderControllerAxes();
		RenderStereoTargets();
		RenderCompanionWindow();

		Texture_t leftEyeTexture = {
       (void*)(uintptr_t)leftEyeDesc.ResolveTextureId, TextureType_OpenGL, 
       ColorSpace_Gamma};
		VRCompositor()->Submit(Eye_Left, &leftEyeTexture);
		Texture_t rightEyeTexture = {
       (void*)(uintptr_t)rightEyeDesc.ResolveTextureId, TextureType_OpenGL, 
       ColorSpace_Gamma};
		VRCompositor()->Submit(Eye_Right, &rightEyeTexture);
	}

	if (Vblank && FinishHack) {
	// $ HACKHACK. From gpuview profiling, it looks like there is a bug where 
   // two renders and a present happen right before and after the vsync causing 
   // all kinds of jittering issues. This glFinish() appears to clear that up. 
   // Temporary fix while I try to get nvidia to investigate this problem.
   // 1/29/2014 mikesart
		glFinish();
	}

	// SwapWindow
	SDL_GL_SwapWindow(CompanionWindow);

	// Clear
	// We want to make sure the glFinish waits for the entire present to 
   // complete, not just the submission of the command. So, we do a clear here 
   //right here so the glFinish will wait fully for the swap.
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Flush and wait for swap.
	if (Vblank) {
		glFlush();
		glFinish();
	}

	// Spew out the controller and pose count whenever they change.
	if (TrackedControllerCount != TrackedControllerCount_Last 
    || ValidPoseCount != ValidPoseCount_Last) {

		ValidPoseCount_Last = ValidPoseCount;
		TrackedControllerCount_Last = TrackedControllerCount;
		
		dprintf("PoseCount:%d(%s) Controllers:%d\n", ValidPoseCount, 
       PoseClasses.c_str(), TrackedControllerCount);
	}

	UpdateHMDMatrixPose();
}


//-----------------------------------------------------------------------------
// Purpose: Compiles a GL shader program and returns the handle. Returns 0 if
//			the shader couldn't be compiled for some reason.
//-----------------------------------------------------------------------------
GLuint CMainApplication::CompileGLShader(const char *ShaderName, 
 const char *VertexShader, const char *FragmentShader) {
	GLuint ProgramID = glCreateProgram();

	GLuint SceneVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(SceneVertexShader, 1, &VertexShader, NULL);
	glCompileShader(SceneVertexShader);

	GLint VertexShaderCompiled = GL_FALSE;
	glGetShaderiv(SceneVertexShader, GL_COMPILE_STATUS, &VertexShaderCompiled);
	if (VertexShaderCompiled != GL_TRUE) {
		dprintf("%s - Unable to compile vertex shader %d!\n", ShaderName, 
       SceneVertexShader);

		glDeleteProgram(ProgramID);
		glDeleteShader(SceneVertexShader);
		return 0;
	}
	glAttachShader(ProgramID, SceneVertexShader);
   // the program hangs onto this once it's attached
	glDeleteShader(SceneVertexShader); 

	GLuint  SceneFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(SceneFragmentShader, 1, &FragmentShader, NULL);
	glCompileShader(SceneFragmentShader);

	GLint FragmentShaderCompiled = GL_FALSE;
	glGetShaderiv(SceneFragmentShader, GL_COMPILE_STATUS, &FragmentShaderCompiled);
	if (FragmentShaderCompiled != GL_TRUE) {
		dprintf("%s - Unable to compile fragment shader %d!\n", ShaderName, 
       SceneFragmentShader);

		glDeleteProgram(ProgramID);
		glDeleteShader(SceneFragmentShader);
		return 0;	
	}

	glAttachShader(ProgramID, SceneFragmentShader);
   // the program hangs onto this once it's attached
	glDeleteShader(SceneFragmentShader);

	glLinkProgram(ProgramID);

	GLint programSuccess = GL_TRUE;
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &programSuccess);
	if (programSuccess != GL_TRUE) {
		dprintf("%s - Error linking program %d!\n", ShaderName, ProgramID);
		glDeleteProgram(ProgramID);
		return 0;
	}

	glUseProgram(ProgramID);
	glUseProgram(0);

	return ProgramID;
}


//-----------------------------------------------------------------------------
// Purpose: Creates all the shaders used by HelloVR SDL
//-----------------------------------------------------------------------------
bool CMainApplication::CreateAllShaders() {
	SceneProgramID = CompileGLShader(
		"Scene",

		// Vertex Shader
		"#version 410\n"
		"uniform mat4 matrix;\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec2 v2UVcoordsIn;\n"
		"layout(location = 2) in vec3 v3NormalIn;\n"
		"out vec2 v2UVcoords;\n"
		"void main()\n"
		"{\n"
		"	v2UVcoords = v2UVcoordsIn;\n"
		"	gl_Position = matrix * position;\n"
		"}\n",

		// Fragment Shader
		"#version 410 core\n"
		"uniform sampler2D mytexture;\n"
		"in vec2 v2UVcoords;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"   outputColor = texture(mytexture, v2UVcoords);\n"
		"}\n"
		);
	SceneMatrixLocation = glGetUniformLocation
   (SceneProgramID, "matrix");

	if(SceneMatrixLocation == -1) {
		dprintf("Unable to find matrix uniform in scene shader\n");
		return false;
	}

	ControllerTransformProgramID = CompileGLShader(
		"Controller",

		// vertex shader
		"#version 410\n"
		"uniform mat4 matrix;\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec3 v3ColorIn;\n"
		"out vec4 v4Color;\n"
		"void main()\n"
		"{\n"
		"	v4Color.xyz = v3ColorIn; v4Color.a = 1.0;\n"
		"	gl_Position = matrix * position;\n"
		"}\n",

		// fragment shader
		"#version 410\n"
		"in vec4 v4Color;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"   outputColor = v4Color;\n"
		"}\n"
		);
	ControllerMatrixLocation = glGetUniformLocation
   (ControllerTransformProgramID, "matrix");

	if(ControllerMatrixLocation == -1) {
		dprintf("Unable to find matrix uniform in controller shader\n");
		return false;
	}

	RenderModelProgramID = CompileGLShader(
		"render model",

		// vertex shader
		"#version 410\n"
		"uniform mat4 matrix;\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec3 v3NormalIn;\n"
		"layout(location = 2) in vec2 v2TexCoordsIn;\n"
		"out vec2 v2TexCoord;\n"
		"void main()\n"
		"{\n"
		"	v2TexCoord = v2TexCoordsIn;\n"
		"	gl_Position = matrix * vec4(position.xyz, 1);\n"
		"}\n",

		//fragment shader
		"#version 410 core\n"
		"uniform sampler2D diffuse;\n"
		"in vec2 v2TexCoord;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"   outputColor = texture(diffuse, v2TexCoord);\n"
		"}\n"

		);
	RenderModelMatrixLocation = glGetUniformLocation
   (RenderModelProgramID, "matrix");

	if(RenderModelMatrixLocation == -1) {
		dprintf("Unable to find matrix uniform in render model shader\n");
		return false;
	}

	CompanionWindowProgramID = CompileGLShader(
		"CompanionWindow",

		// vertex shader
		"#version 410 core\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec2 v2UVIn;\n"
		"noperspective out vec2 v2UV;\n"
		"void main()\n"
		"{\n"
		"	v2UV = v2UVIn;\n"
		"	gl_Position = position;\n"
		"}\n",

		// fragment shader
		"#version 410 core\n"
		"uniform sampler2D mytexture;\n"
		"noperspective in vec2 v2UV;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"		outputColor = texture(mytexture, v2UV);\n"
		"}\n"
		);

	return SceneProgramID != 0 
		&& ControllerTransformProgramID != 0
		&& RenderModelProgramID != 0
		&& CompanionWindowProgramID != 0;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::SetupTexturemaps() {

	string strFullPath = "cube_texture.png";
	
	vector<unsigned char> imageRGBA;
	unsigned ImageWidth, ImageHeight;
	unsigned Error = lodepng::decode(imageRGBA, ImageWidth, ImageHeight, 
    strFullPath.c_str());
	
	if (Error != 0)
		return false;

	glGenTextures(1, &Texture);
	glBindTexture(GL_TEXTURE_2D, Texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ImageWidth, ImageHeight,
		0, GL_RGBA, GL_UNSIGNED_BYTE, &imageRGBA[0]);

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri
   (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	GLfloat Largest;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &Largest);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, Largest);
	 	
	glBindTexture(GL_TEXTURE_2D, 0);

	return (Texture != 0);
}


//-----------------------------------------------------------------------------
// Purpose: create a sea of cubes
//-----------------------------------------------------------------------------
void CMainApplication::SetupScene() {
	if (!HMD)
		return;

	vector<float> vertdataarray;

   
   //vector<Vertex> Verts;
   //int i = 0;
   //mat4x4 identity = mat4x4(1.0f);
   //
   //Verts = cyl.getVertices(identity);

   //for (Vertex v : Verts) {
   // vertdataarray.push_back(v.loc.x);
   // vertdataarray.push_back(v.loc.y);
   // vertdataarray.push_back(v.loc.z);
   // vertdataarray.push_back(v.texLoc.x);
   // vertdataarray.push_back(v.texLoc.y);
   //}

	Matrix4 ScaleMatrix;
	ScaleMatrix.scale(Scale, Scale, Scale);
	Matrix4 Transform;
	Transform.translate(
		-((float)SceneVolumeWidth * ScaleSpacing) / 2.f,
		-((float)SceneVolumeHeight * ScaleSpacing) / 2.f,
		-((float)SceneVolumeDepth * ScaleSpacing) / 2.f);
	
	Matrix4 CubePosition = ScaleMatrix * Transform;

	for(int z = 0; z<SceneVolumeDepth; z++) {
		for(int y = 0; y<SceneVolumeHeight; y++) {
			for(int x = 0; x<SceneVolumeWidth; x++) {

				AddCubeToScene(CubePosition, vertdataarray);
				CubePosition = CubePosition * Matrix4().translate(ScaleSpacing, 0, 0);
			}
			CubePosition = CubePosition * Matrix4().translate(
          -((float)SceneVolumeWidth) * ScaleSpacing, 
          ScaleSpacing, 0);
		}
		CubePosition = CubePosition * Matrix4().translate(0,
       -((float)SceneVolumeHeight) * ScaleSpacing, ScaleSpacing);
	}
	Vertcount = vertdataarray.size()/5;
	
   glCall(glGenVertexArrays(1, &SceneVAO));
   glCall(glBindVertexArray(SceneVAO));

   glCall(glGenBuffers(1, &SceneVertBuffer));
   glCall(glBindBuffer(GL_ARRAY_BUFFER, SceneVertBuffer));
   glCall(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertdataarray.size(),
    &vertdataarray[0], GL_STATIC_DRAW));

	GLsizei stride = sizeof(VertexDataScene);
	uintptr_t offset = 0;

   glCall(glEnableVertexAttribArray(0));
   glCall(glVertexAttribPointer(
    0, 3, GL_FLOAT, GL_FALSE, stride , (const void *)offset));

	offset += sizeof(Vector3);
   glCall(glEnableVertexAttribArray(1));
   glCall(glVertexAttribPointer(
    1, 2, GL_FLOAT, GL_FALSE, stride, (const void *)offset));

   glCall(glBindVertexArray(0));
   glCall(glDisableVertexAttribArray(0));
   glCall(glDisableVertexAttribArray(1));

}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::AddCubeVertex(float fl0, float fl1, float fl2, 
 float fl3, float fl4, vector<float> &vertdata) {
	vertdata.push_back(fl0);
	vertdata.push_back(fl1);
	vertdata.push_back(fl2);
	vertdata.push_back(fl3);
	vertdata.push_back(fl4);
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::AddCubeToScene(Matrix4 mat, vector<float> &vertdata) {
	// Matrix4 mat(outermat.data());

	Vector4 A = mat * Vector4(0, 0, 0, 1);
	Vector4 B = mat * Vector4(1, 0, 0, 1);
	Vector4 C = mat * Vector4(1, 1, 0, 1);
	Vector4 D = mat * Vector4(0, 1, 0, 1);
	Vector4 E = mat * Vector4(0, 0, 1, 1);
	Vector4 F = mat * Vector4(1, 0, 1, 1);
	Vector4 G = mat * Vector4(1, 1, 1, 1);
	Vector4 H = mat * Vector4(0, 1, 1, 1);

	// triangles instead of quads
	AddCubeVertex(E.x, E.y, E.z, 0, 1, vertdata); //Front
	AddCubeVertex(F.x, F.y, F.z, 1, 1, vertdata);
	AddCubeVertex(G.x, G.y, G.z, 1, 0, vertdata);
	AddCubeVertex(G.x, G.y, G.z, 1, 0, vertdata);
	AddCubeVertex(H.x, H.y, H.z, 0, 0, vertdata);
	AddCubeVertex(E.x, E.y, E.z, 0, 1, vertdata);
					 
	AddCubeVertex(B.x, B.y, B.z, 0, 1, vertdata); //Back
	AddCubeVertex(A.x, A.y, A.z, 1, 1, vertdata);
	AddCubeVertex(D.x, D.y, D.z, 1, 0, vertdata);
	AddCubeVertex(D.x, D.y, D.z, 1, 0, vertdata);
	AddCubeVertex(C.x, C.y, C.z, 0, 0, vertdata);
	AddCubeVertex(B.x, B.y, B.z, 0, 1, vertdata);
					
	AddCubeVertex(H.x, H.y, H.z, 0, 1, vertdata); //Top
	AddCubeVertex(G.x, G.y, G.z, 1, 1, vertdata);
	AddCubeVertex(C.x, C.y, C.z, 1, 0, vertdata);
	AddCubeVertex(C.x, C.y, C.z, 1, 0, vertdata);
	AddCubeVertex(D.x, D.y, D.z, 0, 0, vertdata);
	AddCubeVertex(H.x, H.y, H.z, 0, 1, vertdata);
				
	AddCubeVertex(A.x, A.y, A.z, 0, 1, vertdata); //Bottom
	AddCubeVertex(B.x, B.y, B.z, 1, 1, vertdata);
	AddCubeVertex(F.x, F.y, F.z, 1, 0, vertdata);
	AddCubeVertex(F.x, F.y, F.z, 1, 0, vertdata);
	AddCubeVertex(E.x, E.y, E.z, 0, 0, vertdata);
	AddCubeVertex(A.x, A.y, A.z, 0, 1, vertdata);
					
	AddCubeVertex(A.x, A.y, A.z, 0, 1, vertdata); //Left
	AddCubeVertex(E.x, E.y, E.z, 1, 1, vertdata);
	AddCubeVertex(H.x, H.y, H.z, 1, 0, vertdata);
	AddCubeVertex(H.x, H.y, H.z, 1, 0, vertdata);
	AddCubeVertex(D.x, D.y, D.z, 0, 0, vertdata);
	AddCubeVertex(A.x, A.y, A.z, 0, 1, vertdata);

	AddCubeVertex(F.x, F.y, F.z, 0, 1, vertdata); //Right
	AddCubeVertex(B.x, B.y, B.z, 1, 1, vertdata);
	AddCubeVertex(C.x, C.y, C.z, 1, 0, vertdata);
	AddCubeVertex(C.x, C.y, C.z, 1, 0, vertdata);
	AddCubeVertex(G.x, G.y, G.z, 0, 0, vertdata);
	AddCubeVertex(F.x, F.y, F.z, 0, 1, vertdata);
}


//-----------------------------------------------------------------------------
// Purpose: Draw all of the controllers as X/Y/Z lines
//-----------------------------------------------------------------------------
void CMainApplication::RenderControllerAxes() {
	// Don't attempt to update controllers if input is not available
	if(!HMD->IsInputAvailable())
		return;

	vector<float> vertdataarray;

	ControllerVertcount = 0;
	TrackedControllerCount = 0;

	for (TrackedDeviceIndex_t TrackedDevice = k_unTrackedDeviceIndex_Hmd + 1;
    TrackedDevice <k_unMaxTrackedDeviceCount; ++TrackedDevice) {
		if (!HMD->IsTrackedDeviceConnected(TrackedDevice))
			continue;

		if(HMD->GetTrackedDeviceClass(TrackedDevice) 
       != TrackedDeviceClass_Controller)
			continue;

		TrackedControllerCount += 1;

		if(!TrackedDevicePose[TrackedDevice].bPoseIsValid)
			continue;

		const Matrix4 & ContPos = DevicePose[TrackedDevice];

		Vector4 center = ContPos * Vector4(0, 0, 0, 1);

		for (int i = 0; i <3; ++i) {
			Vector3 color(0, 0, 0);
			Vector4 point(0, 0, 0, 1);
			point[i] += 0.05f;  // offset in X, Y, Z
			color[i] = 1.0;  // R, G, B
			point = ContPos * point;
			vertdataarray.push_back(center.x);
			vertdataarray.push_back(center.y);
			vertdataarray.push_back(center.z);

			vertdataarray.push_back(color.x);
			vertdataarray.push_back(color.y);
			vertdataarray.push_back(color.z);
		
			vertdataarray.push_back(point.x);
			vertdataarray.push_back(point.y);
			vertdataarray.push_back(point.z);
		
			vertdataarray.push_back(color.x);
			vertdataarray.push_back(color.y);
			vertdataarray.push_back(color.z);
		
			ControllerVertcount += 2;
		}

		Vector4 start = ContPos * Vector4(0, 0, -0.02f, 1);
		Vector4 end = ContPos * Vector4(0, 0, -39.f, 1);
		Vector3 color(.92f, .92f, .71f);

		vertdataarray.push_back(start.x);
      vertdataarray.push_back(start.y);
      vertdataarray.push_back(start.z);
		vertdataarray.push_back(color.x);
      vertdataarray.push_back(color.y);
      vertdataarray.push_back(color.z);

		vertdataarray.push_back(end.x);
      vertdataarray.push_back(end.y);
      vertdataarray.push_back(end.z);
		vertdataarray.push_back(color.x);
      vertdataarray.push_back(color.y);
      vertdataarray.push_back(color.z);

		ControllerVertcount += 2;
	}

	// Setup the VAO the first time through.
	if (ControllerVAO == 0) {
		glGenVertexArrays(1, &ControllerVAO);
		glBindVertexArray(ControllerVAO);

		glGenBuffers(1, &ControllerVertBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, ControllerVertBuffer);

		GLuint stride = 2 * 3 * sizeof(float);
		uintptr_t offset = 0;

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(
       0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		offset += sizeof(Vector3);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(
       1, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		glBindVertexArray(0);
	}

	glBindBuffer(GL_ARRAY_BUFFER, ControllerVertBuffer);

	// set vertex data if we have some
	if(vertdataarray.size() > 0) {
		//$ TODO: Use glBufferSubData for this...
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertdataarray.size(), 
       &vertdataarray[0], GL_STREAM_DRAW);
	}
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::SetupCameras() {
	ProjectionLeft = GetHMDMatrixProjectionEye(Eye_Left);
	ProjectionRight = GetHMDMatrixProjectionEye(Eye_Right);
	eyePosLeft = GetHMDMatrixPoseEye(Eye_Left);
	eyePosRight = GetHMDMatrixPoseEye(Eye_Right);
}


//-----------------------------------------------------------------------------
// Purpose: Creates a frame buffer. Returns true if the buffer was set up.
//          Returns false if the setup failed.
//-----------------------------------------------------------------------------
bool CMainApplication::CreateFrameBuffer(int nWidth, int nHeight, 
 FramebufferDesc &framebufferDesc) {
   glCall(glGenFramebuffers(1, &framebufferDesc.RenderFramebufferId));
   glCall(glBindFramebuffer(
      GL_FRAMEBUFFER, framebufferDesc.RenderFramebufferId));

   glCall(glGenRenderbuffers(1, &framebufferDesc.DepthBufferId));
   glCall(glBindRenderbuffer(GL_RENDERBUFFER, framebufferDesc.DepthBufferId));
   glCall(glRenderbufferStorageMultisample(
    GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, nWidth, nHeight));
   glCall(glFramebufferRenderbuffer(
    GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
    framebufferDesc.DepthBufferId));

   glCall(glGenTextures(1, &framebufferDesc.RenderTextureId));
   glCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE,
    framebufferDesc.RenderTextureId));

   glCall(glTexImage2DMultisample(
    GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, nWidth, nHeight, true));

   glCall(glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, 
    framebufferDesc.RenderTextureId, 0));

   glCall(glGenFramebuffers(1, &framebufferDesc.ResolveFramebufferId));
   glCall(glBindFramebuffer(
      GL_FRAMEBUFFER, framebufferDesc.ResolveFramebufferId));

   glCall(glGenTextures(1, &framebufferDesc.ResolveTextureId));
   glCall(glBindTexture(GL_TEXTURE_2D, framebufferDesc.ResolveTextureId));
   glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
   glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0));
   glCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, nWidth, nHeight,
    0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
   glCall(glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 
    framebufferDesc.ResolveTextureId, 0));

	// check FBO status
	GLenum status = glCall(glCheckFramebufferStatus(GL_FRAMEBUFFER));
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		return false;
	}

   glCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::SetupStereoRenderTargets() {
	if (!HMD)
		return false;

	HMD->GetRecommendedRenderTargetSize(&RenderWidth, &RenderHeight);

	CreateFrameBuffer(RenderWidth, RenderHeight, leftEyeDesc);
	CreateFrameBuffer(RenderWidth, RenderHeight, rightEyeDesc);
	
	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::SetupCompanionWindow() {
	if (!HMD)
		return;

	vector<VertexDataWindow> Verts;

	// left eye verts
	Verts.push_back(VertexDataWindow(Vector2(-1, -1), Vector2(0, 1)));
	Verts.push_back(VertexDataWindow(Vector2(0, -1), Vector2(1, 1)));
	Verts.push_back(VertexDataWindow(Vector2(-1, 1), Vector2(0, 0)));
	Verts.push_back(VertexDataWindow(Vector2(0, 1), Vector2(1, 0)));

	// right eye verts
	Verts.push_back(VertexDataWindow(Vector2(0, -1), Vector2(0, 1)));
	Verts.push_back(VertexDataWindow(Vector2(1, -1), Vector2(1, 1)));
	Verts.push_back(VertexDataWindow(Vector2(0, 1), Vector2(0, 0)));
	Verts.push_back(VertexDataWindow(Vector2(1, 1), Vector2(1, 0)));

	GLushort vIndices[] = {0, 1, 3,   0, 3, 2,   4, 5, 7,   4, 7, 6};
	CompanionWindowIndexSize = 12;

   glCall(glGenVertexArrays(1, &CompanionWindowVAO));
   glCall(glBindVertexArray(CompanionWindowVAO));

   glCall(glGenBuffers(1, &CompanionWindowIDVertBuffer));
   glCall(glBindBuffer(GL_ARRAY_BUFFER, CompanionWindowIDVertBuffer));
   glCall(glBufferData(GL_ARRAY_BUFFER, Verts.size()*sizeof(VertexDataWindow),
    &Verts[0], GL_STATIC_DRAW));

   glCall(glGenBuffers(1, &CompanionWindowIDIndexBuffer));
   glCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CompanionWindowIDIndexBuffer));
   glCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
    CompanionWindowIndexSize*sizeof(GLushort), 
    &vIndices[0], GL_STATIC_DRAW));

   glCall(glEnableVertexAttribArray(0));
   glCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow),
    (void *)offsetof(VertexDataWindow, position)));

   glCall(glEnableVertexAttribArray(1));
   glCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow),
    (void *)offsetof(VertexDataWindow, texCoord)));

   glCall(glBindVertexArray(0));

   glCall(glDisableVertexAttribArray(0));
   glCall(glDisableVertexAttribArray(1));

   glCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
   glCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::RenderStereoTargets()
{
   glCall(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
   glCall(glEnable(GL_MULTISAMPLE));

	// Left Eye
   glCall(glBindFramebuffer(GL_FRAMEBUFFER, leftEyeDesc.RenderFramebufferId));
   glCall(glViewport(0, 0, RenderWidth, RenderHeight));
 	RenderScene(Eye_Left);
   glCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	
   glCall(glDisable(GL_MULTISAMPLE));
	 	
   glCall(glBindFramebuffer(
    GL_READ_FRAMEBUFFER, leftEyeDesc.RenderFramebufferId));
   glCall(glBindFramebuffer(
    GL_DRAW_FRAMEBUFFER, leftEyeDesc.ResolveFramebufferId));

   glCall(glBlitFramebuffer(0, 0, RenderWidth, RenderHeight, 0, 0,
    RenderWidth, RenderHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR));

   glCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, 0));
   glCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));

   glCall(glEnable(GL_MULTISAMPLE));

	// Right Eye
   glCall(glBindFramebuffer(GL_FRAMEBUFFER, rightEyeDesc.RenderFramebufferId));
   glCall(glViewport(0, 0, RenderWidth, RenderHeight));
 	RenderScene(Eye_Right);
   glCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
 	
   glCall(glDisable(GL_MULTISAMPLE));

   glCall(glBindFramebuffer(
    GL_READ_FRAMEBUFFER, rightEyeDesc.RenderFramebufferId));
   glCall(glBindFramebuffer(
    GL_DRAW_FRAMEBUFFER, rightEyeDesc.ResolveFramebufferId));
	
   glCall(glBlitFramebuffer(0, 0, RenderWidth, RenderHeight, 0, 0,
     RenderWidth, RenderHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR));

   glCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, 0));
   glCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
}


//-----------------------------------------------------------------------------
// Purpose: Renders a scene with respect to nEye.
//-----------------------------------------------------------------------------
void CMainApplication::RenderScene(Hmd_Eye Eye) {
   glCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
   glCall(glEnable(GL_DEPTH_TEST));

	if(ShowCubes) {
      vector<TriangleMesh> Mesh = cyl.getMeshes();
      int indVal, i = 0;

      for (TriangleMesh t : Mesh) {
         GLuint elementbuffer;
         glGenBuffers(1, &elementbuffer);
         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
         glBufferData(GL_ELEMENT_ARRAY_BUFFER,
          Mesh[i].indices.size() * sizeof(uint), &(t.indices[0]), GL_STATIC_DRAW);

         glCall(glUseProgram(SceneProgramID));
         glCall(glUniformMatrix4fv(SceneMatrixLocation, 1, GL_FALSE,
            GetCurrentViewProjectionMatrix(Eye).get()));
         //glCall(glBindVertexArray(SceneVAO));
         glCall(glBindTexture(GL_TEXTURE_2D, Texture));
         glCall(glDrawElements(GL_TRIANGLE_STRIP, t.indices.size(), sizeof(uint), 0));
         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
         // glCall(glDrawArrays(GL_TRIANGLES, 0, Vertcount));
         //glCall(glBindVertexArray(0));
      }
	}

	bool IsInputAvailable = HMD->IsInputAvailable();

	if(IsInputAvailable) {
		// draw the controller axis lines
      glCall(glUseProgram(ControllerTransformProgramID));
      glCall(glUniformMatrix4fv(ControllerMatrixLocation, 1,
       GL_FALSE, GetCurrentViewProjectionMatrix(Eye).get()));
      glCall(glBindVertexArray(ControllerVAO));
      glCall(glDrawArrays(GL_LINES, 0, ControllerVertcount));
      glCall(glBindVertexArray(0));
	}

	// ----- Render Model rendering -----
   glCall(glUseProgram(RenderModelProgramID));

	for(uint32_t TrackedDevice = 0; 
    TrackedDevice <k_unMaxTrackedDeviceCount; TrackedDevice++) {
		if(!TrackedDeviceToRenderModel[TrackedDevice] 
       || !ShowTrackedDevice[TrackedDevice])
			continue;

		const TrackedDevicePose_t & pose = TrackedDevicePose[TrackedDevice];
		if(!pose.bPoseIsValid)
			continue;

		if(!IsInputAvailable && HMD->GetTrackedDeviceClass(TrackedDevice) 
       == TrackedDeviceClass_Controller)
			continue;

		const Matrix4 & DeviceToTracking = DevicePose[TrackedDevice];
		Matrix4 MVP = 
       GetCurrentViewProjectionMatrix(Eye) * DeviceToTracking;
      glCall(glUniformMatrix4fv(
       RenderModelMatrixLocation, 1, GL_FALSE, MVP.get()));

		TrackedDeviceToRenderModel[TrackedDevice]->Draw();
	}

   glCall(glUseProgram(0));
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::RenderCompanionWindow() {
   glCall(glDisable(GL_DEPTH_TEST));
   glCall(glViewport(0, 0, CompanionWindowWidth, CompanionWindowHeight));

   glCall(glBindVertexArray(CompanionWindowVAO));
   glCall(glUseProgram(CompanionWindowProgramID));

	// render left eye (first half of index array)
   glCall(glBindTexture(GL_TEXTURE_2D, leftEyeDesc.ResolveTextureId));
   glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
   glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
   glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
   glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
   glCall(glDrawElements(
    GL_TRIANGLES, CompanionWindowIndexSize/2, GL_UNSIGNED_SHORT, 0));

	// render right eye (second half of index array)
   glCall(glBindTexture(GL_TEXTURE_2D, rightEyeDesc.ResolveTextureId));
   glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
   glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
   glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
   glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
   glCall(glDrawElements(GL_TRIANGLES, CompanionWindowIndexSize/2,
    GL_UNSIGNED_SHORT, 
    (const void *)(uintptr_t)(CompanionWindowIndexSize)));

   glCall(glBindVertexArray(0));
   glCall(glUseProgram(0));
}


//-----------------------------------------------------------------------------
// Purpose: Gets a Matrix Projection Eye with respect to nEye.
//-----------------------------------------------------------------------------
Matrix4 CMainApplication::GetHMDMatrixProjectionEye(Hmd_Eye Eye) {
	if (!HMD)
		return Matrix4();

	HmdMatrix44_t mat = 
    HMD->GetProjectionMatrix(Eye, NearClip, FarClip);

	return Matrix4(
		mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
		mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1], 
		mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2], 
		mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
	);
}


//-----------------------------------------------------------------------------
// Purpose: Gets an HMDMatrixPoseEye with respect to nEye.
//-----------------------------------------------------------------------------
Matrix4 CMainApplication::GetHMDMatrixPoseEye(Hmd_Eye Eye) {
	if (!HMD)
		return Matrix4();

	HmdMatrix34_t matEyeRight = HMD->GetEyeToHeadTransform(Eye);
	Matrix4 matrixObj(
		matEyeRight.m[0][0], matEyeRight.m[1][0], matEyeRight.m[2][0], 0.0, 
		matEyeRight.m[0][1], matEyeRight.m[1][1], matEyeRight.m[2][1], 0.0,
		matEyeRight.m[0][2], matEyeRight.m[1][2], matEyeRight.m[2][2], 0.0,
		matEyeRight.m[0][3], matEyeRight.m[1][3], matEyeRight.m[2][3], 1.0f
		);

	return matrixObj.invert();
}


//-----------------------------------------------------------------------------
// Purpose: Gets a Current View Projection Matrix with respect to nEye,
//          which may be an Eye_Left or an Eye_Right.
//-----------------------------------------------------------------------------
Matrix4 CMainApplication::GetCurrentViewProjectionMatrix(Hmd_Eye Eye) {
	Matrix4 MVP;

	if(Eye == Eye_Left) 
		MVP = ProjectionLeft * eyePosLeft * HMDPose;
	else if(Eye == Eye_Right)
		MVP = ProjectionRight * eyePosRight *  HMDPose;

	return MVP;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::UpdateHMDMatrixPose() {
	if (!HMD)
		return;

	VRCompositor()->WaitGetPoses(
    TrackedDevicePose, k_unMaxTrackedDeviceCount, NULL, 0);

	ValidPoseCount = 0;
	PoseClasses = "";
	for (int nDevice = 0; nDevice <k_unMaxTrackedDeviceCount; ++nDevice) {
		if (TrackedDevicePose[nDevice].bPoseIsValid) {
			ValidPoseCount++;
			DevicePose[nDevice] = ConvertSteamVRMatrixToMatrix4(
          TrackedDevicePose[nDevice].mDeviceToAbsoluteTracking);

			if (DevClassChar[nDevice]==0) {
				switch (HMD->GetTrackedDeviceClass(nDevice)) {
				case TrackedDeviceClass_Controller:        
               DevClassChar[nDevice] = 'C'; 
               break;
				case TrackedDeviceClass_HMD:              
               DevClassChar[nDevice] = 'H'; 
               break;
				case TrackedDeviceClass_Invalid:          
               DevClassChar[nDevice] = 'I'; 
               break;
				case TrackedDeviceClass_GenericTracker:    
               DevClassChar[nDevice] = 'G'; 
               break;
				case TrackedDeviceClass_TrackingReference: 
               DevClassChar[nDevice] = 'T'; 
               break;
				default:                                       
               DevClassChar[nDevice] = '?'; 
               break;
				}
			}
			PoseClasses += DevClassChar[nDevice];
		}
	}

	if (TrackedDevicePose[k_unTrackedDeviceIndex_Hmd].bPoseIsValid) {
		HMDPose = DevicePose[k_unTrackedDeviceIndex_Hmd];
		HMDPose.invert();
	}
}


//-----------------------------------------------------------------------------
// Purpose: Finds a render model we've already loaded or loads a new one
//-----------------------------------------------------------------------------
CGLRenderModel *CMainApplication::FindOrLoadRenderModel(
 const char *RenderModelName) {
	CGLRenderModel *RenderModel = NULL;
	for(vector<CGLRenderModel * >::iterator i = RenderModelVectors.begin(); 
    i != RenderModelVectors.end(); i++) {
		if(!_stricmp((*i)->GetName().c_str(), RenderModelName)) {
			RenderModel = *i;
			break;
		}
	}

	// load the model if we didn't find one
	if(!RenderModel) {
		RenderModel_t *Model;
		EVRRenderModelError error;

		while (1) {
			error = VRRenderModels()->LoadRenderModel_Async(
          RenderModelName, &Model);
			if (error != VRRenderModelError_Loading)
				break;

			ThreadSleep(1);
		}

		if (error != VRRenderModelError_None) {
			dprintf("Unable to load render model %s - %s\n", RenderModelName, 
          VRRenderModels()->GetRenderModelErrorNameFromEnum(error));
			return NULL; // move on to the next tracked device
		}

		RenderModel_TextureMap_t *Texture;

		while (1) {
			error = VRRenderModels()->LoadTexture_Async(
          Model->diffuseTextureId, &Texture);

			if (error != VRRenderModelError_Loading)
				break;

			ThreadSleep(1);
		}

		if (error != VRRenderModelError_None) {
			dprintf("Unable to load render texture id:%d for render model %s\n",
          Model->diffuseTextureId, RenderModelName);
			VRRenderModels()->FreeRenderModel(Model);
			return NULL; // move on to the next tracked device
		}

		RenderModel = new CGLRenderModel(RenderModelName);

		if (!RenderModel->BInit(*Model, *Texture)) {
			dprintf("Unable to create GL model from render model %s\n",
          RenderModelName);
			delete RenderModel;
			RenderModel = NULL;
		}
		else
			RenderModelVectors.push_back(RenderModel);

		VRRenderModels()->FreeRenderModel(Model);
		VRRenderModels()->FreeTexture(Texture);
	}
	return RenderModel;
}


//-----------------------------------------------------------------------------
// Purpose: Create/destroy GL a Render Model for a single tracked device
//-----------------------------------------------------------------------------
void CMainApplication::SetupRenderModelForTrackedDevice(
 TrackedDeviceIndex_t TrackedDeviceIndex) {
	if(TrackedDeviceIndex >= k_unMaxTrackedDeviceCount)
		return;

	// try to find a model we've already set up
	string RenderModelName = GetTrackedDeviceString(
    HMD, TrackedDeviceIndex, Prop_RenderModelName_String);
	CGLRenderModel *RenderModel = FindOrLoadRenderModel(
    RenderModelName.c_str());
	if(!RenderModel) {
		string sTrackingSystemName = GetTrackedDeviceString(
       HMD, TrackedDeviceIndex, Prop_TrackingSystemName_String);

		dprintf("Unable to load render model for tracked device %d (%s.%s)", 
       TrackedDeviceIndex, sTrackingSystemName.c_str(), 
       RenderModelName.c_str());
	}
	else {
		TrackedDeviceToRenderModel[TrackedDeviceIndex] = RenderModel;
		ShowTrackedDevice[TrackedDeviceIndex] = true;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Create/destroy GL Render Models
//-----------------------------------------------------------------------------
void CMainApplication::SetupRenderModels()
{
	memset(
    TrackedDeviceToRenderModel, 0, sizeof(TrackedDeviceToRenderModel));

	if(!HMD)
		return;

	for(uint32_t TrackedDevice = k_unTrackedDeviceIndex_Hmd + 1; 
    TrackedDevice <k_unMaxTrackedDeviceCount; TrackedDevice++) {

		if(!HMD->IsTrackedDeviceConnected(TrackedDevice))
			continue;

		SetupRenderModelForTrackedDevice(TrackedDevice);
	}

}


//-----------------------------------------------------------------------------
// Purpose: Converts a SteamVR matrix to our local matrix class
//-----------------------------------------------------------------------------
Matrix4 CMainApplication::ConvertSteamVRMatrixToMatrix4(
 const HmdMatrix34_t &Pose) {

	Matrix4 matrixObj(
		Pose.m[0][0], Pose.m[1][0], Pose.m[2][0], 0.0,
		Pose.m[0][1], Pose.m[1][1], Pose.m[2][1], 0.0,
		Pose.m[0][2], Pose.m[1][2], Pose.m[2][2], 0.0,
		Pose.m[0][3], Pose.m[1][3], Pose.m[2][3], 1.0f
		);
	return matrixObj;
}


//-----------------------------------------------------------------------------
// Purpose: Create/destroy GL Render Models
//-----------------------------------------------------------------------------
CGLRenderModel::CGLRenderModel(const string & sRenderModelName)
	: ModelName(sRenderModelName) {

	IndexBuffer = 0;
	VertArray = 0;
	VertBuffer = 0;
	Texture = 0;
}


CGLRenderModel::~CGLRenderModel() {
	Cleanup();
}


//-----------------------------------------------------------------------------
// Purpose: Allocates and populates the GL resources for a render model
//-----------------------------------------------------------------------------
bool CGLRenderModel::BInit(const RenderModel_t & vrModel, 
 const RenderModel_TextureMap_t & vrDiffuseTexture) {
	// create and bind a VAO to hold state for this model
   glCall(glGenVertexArrays(1, &VertArray));
   glCall(glBindVertexArray(VertArray));

	// Populate a vertex buffer
   glCall(glGenBuffers(1, &VertBuffer));
   glCall(glBindBuffer(GL_ARRAY_BUFFER, VertBuffer));
   glCall(glBufferData(GL_ARRAY_BUFFER,
    sizeof(RenderModel_Vertex_t) * vrModel.unVertexCount, vrModel.rVertexData, 
    GL_STATIC_DRAW));

	// Identify the components in the vertex buffer
   glCall(glEnableVertexAttribArray(0));
   glCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
    sizeof(RenderModel_Vertex_t), 
    (void *)offsetof(RenderModel_Vertex_t, vPosition)));

   glCall(glEnableVertexAttribArray(1));
   glCall(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
    sizeof(RenderModel_Vertex_t), 
    (void *)offsetof(RenderModel_Vertex_t, vNormal)));

   glCall(glEnableVertexAttribArray(2));
   glCall(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
    sizeof(RenderModel_Vertex_t), 
    (void *)offsetof(RenderModel_Vertex_t, rfTextureCoord)));

	// Create and populate the index buffer
   glCall(glGenBuffers(1, &IndexBuffer));
   glCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer));
   glCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
    sizeof(uint16_t) * vrModel.unTriangleCount * 3, 
    vrModel.rIndexData, GL_STATIC_DRAW));

   glCall(glBindVertexArray(0));

	// create and populate the texture
   glCall(glGenTextures(1, &Texture));
   glCall(glBindTexture(GL_TEXTURE_2D, Texture));

   glCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, vrDiffuseTexture.unWidth,
    vrDiffuseTexture.unHeight,0, GL_RGBA, GL_UNSIGNED_BYTE, 
    vrDiffuseTexture.rubTextureMapData));

	// If this renders black ask McJohn what's wrong.
   glCall(glGenerateMipmap(GL_TEXTURE_2D));

   glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
   glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
   glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
   glCall(glTexParameteri(
    GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));

	GLfloat Largest;
   glCall(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &Largest));
   glCall(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, Largest));

   glCall(glBindTexture(GL_TEXTURE_2D, 0));

	VertexCount = vrModel.unTriangleCount * 3;

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Frees the GL resources for a render model
//-----------------------------------------------------------------------------
void CGLRenderModel::Cleanup() {
	if(VertBuffer)
	{
      glCall(glDeleteBuffers(1, &IndexBuffer));
      glCall(glDeleteVertexArrays(1, &VertArray));
      glCall(glDeleteBuffers(1, &VertBuffer));
		IndexBuffer = 0;
		VertArray = 0;
		VertBuffer = 0;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Draws the render model
//-----------------------------------------------------------------------------
void CGLRenderModel::Draw() {

   glBindVertexArray(VertArray);

   glCall(glActiveTexture(GL_TEXTURE0));
   glCall(glBindTexture(GL_TEXTURE_2D, Texture));

   glCall(glDrawElements(GL_TRIANGLES, VertexCount, GL_UNSIGNED_INT, 0));

   glBindVertexArray(0);
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int main(int argc, char *argv[]) {
	CMainApplication *MainApplication = new CMainApplication(argc, argv);

	if (!MainApplication->BInit()) {
		MainApplication->Shutdown();
		return 1;
	}

	MainApplication->RunMainLoop();

	MainApplication->Shutdown();

	return 0;
}
