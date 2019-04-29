#include <glm/gtc/matrix_transform.hpp>

#include "Renderer.h"
#include "Model.h"
#include "Utility.h"
#include "HMDInput.h"

using namespace std;
using namespace glm;


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * /
Renderer Private Functions
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/// render single frame for each display, then output to whatever screen
void Renderer::RenderDisplay(shared_ptr<Display> dsp, 
 shared_ptr<HMDInput> inp) {
   dsp->PrepareWindow(mSdr, inp);
   if (mTexs.size() == mVAOs.size() && mTexs.size() == mElmBuffs.size()){
      for (int i = 0; i < mTexs.size(); i++) {

         // set texture and normal map, if exists
         mTexs[i]->UseTexture();
         if (mTexsNormal[i]) {
            mTexsNormal[i]->UseTexture();
            mSdr->SetNMap(true);
         }
         else
            mSdr->SetNMap(false);

         // render for specific display
         glBindVertexArray(mVAOs[i]); GLChkErr;
         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElmBuffs[i]); GLChkErr;
         dsp->Redraw(mSdr, mIndSizes[i]);
      }
   }
   else
      throw WorldException("Texture/VAO mismatch");

   // output to screen
   dsp->SwapWindows();
}

/// single pass render of shadows
void Renderer::RenderShadowMap() {
   glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   if (mTexs.size() == mVAOs.size() && mTexs.size() == mElmBuffs.size()) {
      for (int i = 0; i < mTexs.size(); i++) {
         glBindVertexArray(mVAOs[i]); GLChkErr;
         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElmBuffs[i]); GLChkErr;
         glDrawElements(GL_TRIANGLES, mIndSizes[i], GL_UNSIGNED_INT, (void*)0);
      }
   }
   else
      throw WorldException("Texture/VAO mismatch");
}

/// collect input from whatever is being used
int Renderer::HandleInput(shared_ptr<HMDInput> input, SDL_Event *event) {
   SDL_PollEvent(event);
   return input->FieldEvent(*event);
}

/// create usable buffers from models
void Renderer::CreateBuffers() {

   mat4 temp = translate(mat4(1.0f), vec3(1, 1, 1));
   VMap vertMap = mMdl->GetVertices(mat4(1.0f), temp);
   TMap trngMap = mMdl->GetTriangles();
   NMap normMap = mMdl->GetNormal();
   vector<vector<Vertex>> vertices;
   vector<vector<uint>> indices;
   int texIdx = 0;

   // build Vertex and Indice vectors from Vmap and TMap
   for (VMap::iterator it = vertMap.begin(); it != vertMap.end(); it++) {
      mTexs.push_back(it->first);
      mTexsNormal.push_back(normMap[it->first]);
      vector<Vertex> verts;
      vector<uint> inds;
      list<vector<Vertex>> v = vertMap.at(it->first);
      list<TriangleSet> t = trngMap.at(it->first);

      if (vertMap.at(it->first).size() == trngMap.at(it->first).size()) {
         while (v.size()) {
            inds.insert(inds.end(),
               t.front().mIndices.begin(), t.front().mIndices.end());
            verts.insert(verts.end(), v.front().begin(), v.front().end());
            v.pop_front();
            t.pop_front();
         }
         vertices.push_back(verts);
         indices.push_back(inds);
         mIndSizes.push_back(inds.size());
      }
      else
         throw WorldException("Triangle Mesh and Vertex count are different!");
   }

   //tangent/bitanget calculation
   for (int i = 0; i < mTexs.size(); i++) {
      for (int x = 0; x < indices[i].size(); x++) {
         Vertex v1 = vertices[i][indices[i][x]];
         Vertex v2 = vertices[i][indices[i][x+1]];
         Vertex v3 = vertices[i][indices[i][x+2]];

         vec3 DP1 = vec3(v2.loc - v1.loc);
         vec3 DP2 = vec3(v3.loc - v1.loc);
         vec2 DT1 = v2.texLoc - v1.texLoc;
         vec2 DT2 = v3.texLoc - v1.texLoc;

         float r = 1.0f / (DT1.x * DT2.y - DT1.y * DT2.x);
         vec3 tangent = (DP1 * DT2.y - DP2 * DT1.y)*r;
         vec3 biTangent = (DT1.x * DP2 - DP1 * DT2.x)*r;

         vertices[i][indices[i][x]].tangent = tangent;
         vertices[i][indices[i][x+1]].tangent = tangent;
         vertices[i][indices[i][x+2]].tangent = tangent;

         vertices[i][indices[i][x++]].biTangent = biTangent;
         vertices[i][indices[i][x++]].biTangent = biTangent;
         vertices[i][indices[i][x]].biTangent = biTangent;

      }
   }

   mVAOs = vector<GLuint>(indices.size());
   mElmBuffs = vector<GLuint>(indices.size());
   mVBOs = vector<GLuint>(indices.size());

   glGenVertexArrays(indices.size(), &mVAOs[0]); GLChkErr;
   glGenBuffers(indices.size(), &mElmBuffs[0]); GLChkErr;
   glGenBuffers(mVBOs.size(), &mVBOs[0]); GLChkErr;

   texIdx = 0; // Current texture, as we progress through the Vertex vectors
   for (vector<Vertex> vec : vertices) {

      // Bind the VAO
      glBindVertexArray(mVAOs[texIdx]); GLChkErr;

      glBindBuffer(GL_ARRAY_BUFFER, mVBOs[(texIdx)]); GLChkErr;
      glBufferData(GL_ARRAY_BUFFER, vec.size() * sizeof(Vertex),
         vec.data(), GL_STATIC_DRAW); GLChkErr;

      glVertexAttribPointer
      (0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0); GLChkErr;

      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
         (void*)(4 * sizeof(float))); GLChkErr;

      glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
         (void*)(7 * sizeof(float))); GLChkErr;

      glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
         (void*)(9 * sizeof(float))); GLChkErr;

      glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
         (void*)(12 * sizeof(float))); GLChkErr;

      // use the attributes for each array
      glEnableVertexAttribArray(0); GLChkErr;
      glEnableVertexAttribArray(1); GLChkErr;
      glEnableVertexAttribArray(2); GLChkErr;
      glEnableVertexAttribArray(3); GLChkErr;
      glEnableVertexAttribArray(4); GLChkErr;

      glBindVertexArray(mVAOs[texIdx++]); GLChkErr;
   }

   texIdx = 0;
   for (vector<uint> inds : indices) {

      // create indice array
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElmBuffs[texIdx]); GLChkErr;
      glBufferData(GL_ELEMENT_ARRAY_BUFFER,
         inds.size() * sizeof(uint), inds.data(), GL_STATIC_DRAW); GLChkErr;
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElmBuffs[texIdx]); GLChkErr;

      glBindVertexArray(mVAOs[texIdx++]); GLChkErr;
   }
}

/// create single instance of shader
void Renderer::CreateShader() {
   mSdr = shared_ptr<Shader>(new Shader());

   // create multiple light sources (up to five)
   vec4 pos(0, 1, 0, 1);
   vec3 clr(0.8f, 0.8f, 0.8f);
   LightSource light(pos, clr);

   mLightSources.push_back(light);
}

/// render single pass shadow map
void Renderer::CreateShadowMap() {

   glGenFramebuffers(1, &mShadowMap);

   uint shdSize = 1024;
   GLuint depthMap;

   // set parameters for output texture
   glGenTextures(1, &depthMap);
   glBindTexture(GL_TEXTURE_2D, depthMap);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
      shdSize, shdSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

   // create frame buffer 
   glBindFramebuffer(GL_FRAMEBUFFER, mShadowMap);
   glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
   glDrawBuffer(GL_NONE);
   glReadBuffer(GL_NONE);
   glBindFramebuffer(GL_FRAMEBUFFER, 0);

   // bind and set viewport
   glViewport(0, 0, shdSize, shdSize);
   glBindFramebuffer(GL_FRAMEBUFFER, mShadowMap);
   glClear(GL_DEPTH_BUFFER_BIT);

   // create camera transformation matrix
   float near_plane = 1.0f, far_plane = 10.0f;
   mat4 lightProjection = ortho(
    -10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);

   mat4 lightView = glm::lookAt(
    vec3(0 , 5, 2),
    vec3(0.0f, 0.0f, 0.0f),
    vec3(0.0f, 1.0f, 0.0f));

   mLSM = lightProjection * lightView;
   mSdr->SetShadowMap(mLSM);
   mTexs[0]->UseTexture();

   // create shadow map
   RenderShadowMap();

   glBindFramebuffer(GL_FRAMEBUFFER, 0);

   // create texture from framebuffer rendering
   mDepthTex = shared_ptr<Texture>(new TextureShadow(depthMap, "Shadows"));
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * /
Renderer Public Functions
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/// set up renderer (buffers, shadow map, shader)
Renderer::Renderer(shared_ptr<Model> mdl, vector<shared_ptr<Display>> displays,
 vector<shared_ptr<HMDInput>>input) : mDisplays(displays), mInputs(input),
 mMdl(mdl) {
   CreateShader();
   CreateBuffers();
   CreateShadowMap();
}

/// game loop, untied from input and FPS
void Renderer::Run() {
   SDL_Event *event = new SDL_Event();
   SDL_PollEvent(event);
   bool breakESC = true;

   mSdr->Configure(mLightSources, mLSM);
   mDepthTex->UseTexture();

   while (breakESC) {
      if (mDisplays.size() == mInputs.size()) {
         for (int i = 0; i < mDisplays.size(); i++) {
            while (SDL_PollEvent(event))
               breakESC = !mInputs[0]->FieldEvent(*event);
            RenderDisplay(mDisplays[i], mInputs[i]);
         }
      }
      else
         throw WorldException("Triangle Mesh and Vertex count are different!");
   }
}
