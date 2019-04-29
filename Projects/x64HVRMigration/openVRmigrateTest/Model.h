#pragma once

#include <vector>
#include <GL/glew.h>
#include "Matrices.h"
#include "Textures.h"
#include "MyLib.h"
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>

// Sam, please:
// 1. Change function names to CapCamelCase (?)
// 2. Write DirectModel.

struct Vertex {
   glm::vec4 loc;    // Location vector in NDC
   glm::vec3 normal; // Vertex unit normal in NDC
   glm::vec2 texLoc; // Texture location for 2-D mapping

   Vertex() {}
   Vertex(glm::vec4 lc, glm::vec3 n, glm::vec2 t)
    : loc(lc), normal(n), texLoc(t) {}
};

struct TriangleMesh {
   std::string mDescription;   // Description e.g. "table leg 4" for debug
   Texture *mTex;             // Texture to apply to this mesh
   std::vector<uint> indices; // Indices of vertices forming the mesh 

   TriangleMesh(std::string d, Texture *t,
      const std::vector<uint> &i) : mDescription(d), mTex(t), indices(i) {}

   // Fill in a uint vector with values x in range [lo, hi). (Useful for many
   // index-vector creation cases.)
   static std::vector<uint> MakeRangeVec(int lo, int hi);
};

// Abstract interface for all model scenetree nodes
class Model {
protected:
   std::string mName;

public:
   Model(std::string n) : mName(n) {}
   virtual std::string getName() const {return mName;}

   // Return all vertices in model, transformed by tfm
   virtual std::vector<Vertex> getVertices(const glm::mat4x4 &tfm) const = 0;
   virtual int getNumVertices() const = 0; // Count vertices w/o generating them

   // Return all TriangleMeshes in model, index-referencing the vertices.
   virtual std::vector<TriangleMesh> getMeshes() const = 0;
};

class CompositeModel : public Model {
protected:
   struct ChildRef {
      glm::mat4x4 xform;
      Model *mdl;

      ChildRef(const glm::mat4x4 &xf, Model *m) : xform(xf), mdl(m) {}
   };

   std::vector<ChildRef> children = std::vector<ChildRef>();

public:
   void addChild(const glm::mat4x4 &xf, Model *m) {
      children.push_back(ChildRef(xf, m));
   };

   std::vector<Vertex> getVertices(const glm::mat4x4 &tfm) const override;
   std::vector<TriangleMesh> getMeshes() const override;
   int getNumVertices() const override;
}
;
// Model initialized via constructor-passed information
class DirectModel : public Model {
public:
   DirectModel(std::string n, std::vector<Vertex> vertices,
    std::vector<TriangleMesh> meshes);
};

// Model initialized by file contents
class FileModel : public Model {
public:
   FileModel(std::string n, std::string fN);
};

// Model formed by an x/y figure defined by a parametric polar distance function, 
// sampled at points specified in the constructor, with domain 0.0 to 1..
// Resultant cylinder
// is centered at NDC origin, stretching from -1 to 1 in Z, and is closed
// at top and bottom.  X/Y extent is determined by the parametric function, 
// but it's expected to stay within -1 to 1 range in X and Y
class CylinderModel : public Model {
protected:
   std::vector<float> mSamplePts; // Vals in [0, 2pi) on which to call polarDist
   Texture *mTex;
   int mUReps;      // Number of tex repeats around cylinder (int to avoid seam)
   float mVReps;    // Number of tex repeats down cylinder

   virtual float polarDist(float x) const = 0;
public:
   CylinderModel(std::string n, Texture *t, int uReps, float vReps, 
    const std::vector<float> &s) : Model(n), mTex(t), mUReps(uReps),
    mVReps(vReps), mSamplePts(s) {}

   std::vector<Vertex> getVertices(const glm::mat4x4 &tfm) const override;
   std::vector<TriangleMesh> getMeshes() const override;
   int getNumVertices() const override;
};

class CircleCylinderModel : public CylinderModel {
public: 
   CircleCylinderModel(std::string n, Texture *t, int uReps, float vReps,
    const std::vector<float> &s) : CylinderModel(n, t, uReps, vReps, s) {}

   float polarDist(float x) const override {return 1.0;}
};

