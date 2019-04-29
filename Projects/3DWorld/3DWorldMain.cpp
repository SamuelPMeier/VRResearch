//#include "3DWorldMain.h"
//#include "Utility.h"
//#include <windows.h>
//#include <conio.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <glm/gtx/transform.hpp>
//#include <GL/glew.h>
//#include <glm/glm.hpp>
//#include <openvr.h>
//#include <iostream>
//#include <sstream>
//#include <fstream>
//#include <memory>
//#include <SDL.h>
//
//#include "Model.h"
//#include "lodepng.h"
//#include "compat.h"
//#include "strtools.h"
//#include "Textures.h"
//#include "ModelMaker.h"
//#include "HMDInput.h" 
//
//#define chkErr(x);\
//   x;\
//   ThrowException(__LINE__, __FILE__)
//
//using namespace glm;
//using namespace vr;
//using namespace std;
//
//bool TryCompileShader(int shaderId);
//int CreateShader(const std::string &fileName, GLenum shaderType);
//string ReadFile(const char* file);
//vector<GLfloat> ReadFileModel(const char* file);
//
//int main(int argc, char *argv[]) {
//   vec3 loc, rot;
//   loc = rot = {0, 0, 0};
//   vector<vec3> movement{loc, rot};
//   const vector<float> angles{0.0f, 1.5708f, 3.14159f, 4.71239f};
//
//   int Width = 1000, Height = 1000;
//   SDL_Window * window = NULL;
//
//
//   SDL_GLContext mainContext;
//   vector<GLfloat> positions;
//   vector<GLfloat> colors;
//   uint32_t positionAttributeIndex = 0, colorAttributeIndex = 1;
//   GLuint shaderProgram;
//   GLuint mvpID, nvpID, colorID, lPosID, lColID;
//   bool matrixIDSet;
//   std::vector<int32_t> shaderIds;
//   GLuint vbo[3], vao[1];
//   GLuint vertexshader, fragmentShader;
//   GLuint elementbuffer;
//   shared_ptr<Texture> t;  // INITIALIZE ME!!!
//   string s = "Cube";
//
//   //CircleCylinderModel cyl(s, t, 4, 1.0f, angles);
//   //vector<Vertex> vec = cyl.getVertices(mat4x4(1.0f));
//   //vector<uint> indices = cyl.getMesh().indices;
//
//// above works, below does not, and should
//
//   CubeMaker *maker =  new CubeMaker(1.0f, t);
//   unique_ptr<Model> cubeMdl = maker->MakeModel();
//   vector<Vertex> vec = cubeMdl->getVertices(mat4x4(1.0f));
//   vector<uint> indices = cubeMdl->getMesh().indices;
//
//   GLuint Texture;
//   int i = 0;
//
//   printf("\n\n\n\n\n\n");
//
//   for (Vertex v : vec) {
//      cout << i++ <<endl;
//      printVec(v.loc);
//      printVec(v.normal);
//      printVec(v.texLoc);
//   }
//
//   for (int i = 1; i < indices.size(); i += 3) {
//      printf("%d %d %d\n", indices[i-1], indices[i], indices[i+1]);
//   }
//
//   try {
//      mat4 projection = glm::perspective
//      (
//         0.4f,       // Field of view ( how far out to the sides we can see )
//         1.0f,       // Aspect ratio ( stretch image in widh or height )
//         0.5f,       // Near plane ( anything close than this will be cut off )
//         100.0f      // Far plane ( anything further away than this will be cut off )
//      );
//
//      // Set up view matric
//      mat4 view = glm::lookAt
//      (
//         vec3(0, 0, -10), // Camera is at (0,0,-4), in World Space
//         vec3(0, 0, 0),  // And looks at the origin
//         vec3(0, -1, 0)  // Head is up ( set to 0,-1,0 to look upside-down )
//      );
//
//
//
//      if (SDL_Init(SDL_INIT_VIDEO) < 0)
//      {
//         printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
//      }
//      else
//      {
//         window = SDL_CreateWindow(
//            "SDL Tutorial",
//            SDL_WINDOWPOS_UNDEFINED,
//            SDL_WINDOWPOS_UNDEFINED,
//            Width,
//            Height,
//            SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
//
//         if (window == NULL) {
//            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
//         }
//         else {
//            mainContext = SDL_GL_CreateContext(window);
//
//            glEnable(GL_BLEND);
//            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//            glEnable(GL_DEPTH_TEST);
//            glDepthFunc(GL_LEQUAL);
//
//            SDL_GL_SetSwapInterval(1);
//            glewExperimental = GL_TRUE;
//            glewInit();
//		
//            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
//            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
//            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
//            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
//
//
//            shaderProgram = glCreateProgram();
//
//            int shaderId = CreateShader("Resource/frag.glsl", GL_FRAGMENT_SHADER);
//
//            if (TryCompileShader(shaderId)) {
//               glAttachShader(shaderProgram, shaderId);
//               shaderIds.push_back(shaderId);
//            }
//
//            shaderId = CreateShader("Resource/vert.glsl", GL_VERTEX_SHADER);
//
//            if (TryCompileShader(shaderId)) {
//               glAttachShader(shaderProgram, shaderId);
//               shaderIds.push_back(shaderId);
//            }
//
//            glLinkProgram(shaderProgram);
//
//            glUseProgram(shaderProgram);
//
//            positions = ReadFileModel("Resource/positions.txt");
//            colors = ReadFileModel("Resource/colors.txt");
//
//            string strFullPath = "Resource/white_texture.png";
//
//            vector<unsigned char> imageRGBA;
//            unsigned ImageWidth, ImageHeight;
//            unsigned Error = lodepng::decode(imageRGBA, ImageWidth, ImageHeight,
//               strFullPath.c_str());
//
//            if (Error != 0)
//               return false;
//
//            chkErr(glGenTextures(1, &Texture));
//            chkErr(glBindTexture(GL_TEXTURE_2D, Texture));
//
//            chkErr(glTexParameteri(
//               GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
//            chkErr(glTexParameteri(
//               GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT)); //GL_MIRRORED_REPEAT
//            chkErr(glTexParameteri(
//               GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
//            chkErr(glTexParameteri
//            (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
//
//            chkErr(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ImageWidth, ImageHeight,
//               0, GL_RGBA, GL_UNSIGNED_BYTE, &imageRGBA[0]));
//
//            chkErr(glGenerateMipmap(GL_TEXTURE_2D));
//
//            chkErr(glBindTexture(GL_TEXTURE_2D, Texture));
//
//            chkErr(glGenVertexArrays(1, vao));
//            chkErr(glBindVertexArray(vao[0]));
//
//            chkErr(glGenBuffers(3, vbo));
//            chkErr(glBindBuffer(GL_ARRAY_BUFFER, vbo[0]));
//            chkErr(glBufferData(GL_ARRAY_BUFFER, vec.size() * sizeof(Vertex),
//               &vec[0], GL_STATIC_DRAW));
//            chkErr(glVertexAttribPointer
//            (0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0));
//
//            chkErr(glBindBuffer(GL_ARRAY_BUFFER, vbo[1]));
//            chkErr(glBufferData(GL_ARRAY_BUFFER, vec.size() * sizeof(Vertex),
//               &vec[0], GL_STATIC_DRAW));
//            chkErr(glVertexAttribPointer
//            (1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(4 * sizeof(float))));
//
//            chkErr(glBindBuffer(GL_ARRAY_BUFFER, vbo[2]));
//            chkErr(glBufferData(GL_ARRAY_BUFFER, vec.size() * sizeof(Vertex),
//               &vec[0], GL_STATIC_DRAW));
//            chkErr(glVertexAttribPointer
//            (2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(7 * sizeof(float))));
//
//            chkErr(glEnableVertexAttribArray(0));
//            chkErr(glEnableVertexAttribArray(1));
//            chkErr(glEnableVertexAttribArray(2));
//
//            chkErr(glGenBuffers(1, &elementbuffer));
//            chkErr(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer));
//            chkErr(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
//               indices.size() * sizeof(uint), &indices[0], GL_STATIC_DRAW));
//            chkErr(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer));
//
//            chkErr(glBindVertexArray(vao[0]));
//         }
//      }
//
//      KeyboardHMDInput keys(window, 0.005f, 100);
//
//      while (true) {
//         SDL_Event *event = new SDL_Event;
//         SDL_PollEvent(event);
//         keys.FieldEvent(*event);
//
//         mat4x4 mat = keys.GetViewTransform(HMDInput::cLeft);
//
//         printMat(mat);
//
//         COORD c;
//         c.X = 0;
//         c.Y = 0;
//
//         SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
//
//         glm::mat4 mvp = projection * view * mat;
//         glm::mat3 nvp = transpose(inverse(mat3(mvp)));
//         printMat(mvp);
//         printMat(nvp);
//
//         mvpID = glGetUniformLocation(shaderProgram, "mvp");
//         nvpID = glGetUniformLocation(shaderProgram, "nvp");
//         lPosID = glGetUniformLocation(shaderProgram, "lPos");
//         lColID = glGetUniformLocation(shaderProgram, "lColor");
//         GLuint ambColID = glGetUniformLocation(shaderProgram, "ambColor");
//
//         try {
//            chkErr(glUniformMatrix4fv(mvpID, 1, GL_FALSE, &mvp[0][0]));
//            chkErr(glUniformMatrix3fv(nvpID, 1, GL_FALSE, &nvp[0][0]));
//         }
//         catch (WorldException err) {
//            printf("Error: %s\n", err.what());
//         }
//
//         float pos[]{-10, -10, -10}, 
//          lCol[]{0.5f, 0.5f, .75}, 
//          ambCol[]{.25, .25, .25};
//         chkErr(glUniform3fv(lPosID, 1, pos));
//         chkErr(glUniform3fv(lColID, 1, lCol));
//         chkErr(glUniform3fv(ambColID, 1, ambCol));
//
//         chkErr(glUseProgram(shaderProgram));
//         chkErr(glClearColor(0.0, 0.0, 0.0, 1.0));
//         chkErr(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
//         chkErr(glDrawElements(
//            GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void*)0));
//         SDL_GL_SwapWindow(window);
//      }
//   }
//   catch (WorldException e) {
//      printf("%s", e.what());
//   }
//   return 0;
//}
//
//vector<GLfloat> ReadFileModel(const char* file) {
//   // Open file
//   ifstream t(file);
//
//   vector<GLfloat> result;
//
//   while (t.good()) {
//      string str;
//      t >> str;
//
//      GLfloat f = atof(str.c_str());
//
//      result.push_back(f);
//   }
//
//   return result;
//}
//
//string ReadFile(const char* file) {
//   // Open file
//   std::ifstream t(file);
//
//   // Read file into buffer
//   std::stringstream buffer;
//   buffer << t.rdbuf();
//
//   // Make a std::string and fill it with the contents of buffer
//   std::string fileContent = buffer.str();
//
//   return fileContent;
//}
//
//int CreateShader(const std::string &fileName, GLenum shaderType) {
//   // Read file as std::string 
//   std::string str = ReadFile(fileName.c_str());
//
//   // c_str() gives us a const char*, but we need a non-const one
//   char* src = const_cast<char*>(str.c_str());
//   int32_t size = str.length();
//
//   // Create an empty vertex shader handle
//   int shaderId = glCreateShader(shaderType);
//
//   // Send the vertex shader source code to OpenGL
//   glShaderSource(shaderId, 1, &src, &size);
//
//   return shaderId;
//}
//
//bool TryCompileShader(int shaderId) {
//   // Compile the vertex shader
//   glCompileShader(shaderId);
//
//   // Ask OpenGL if the shaders was compiled
//   int wasCompiled = 0;
//   glGetShaderiv(shaderId, GL_COMPILE_STATUS, &wasCompiled);
//
//   // Return false if compilation failed
//   return (wasCompiled != 0);
//}