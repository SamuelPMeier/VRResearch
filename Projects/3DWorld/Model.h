#pragma once

#include <vector>
#include <GL/glew.h>
#include "Textures.h"
#include "Utility.h"
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <memory>
#include <list>
#include <map>

// all information for individual vertex6
struct Vertex {
   glm::vec4 loc;    // Location vector in NDC
   glm::vec3 normal; // Vertex unit normal in NDC
   glm::vec2 texLoc; // Texture location for 2-D mapping
   glm::vec3 tangent;
   glm::vec3 biTangent;

   Vertex() {}
   Vertex(glm::vec4 lc, glm::vec3 n, glm::vec2 t, 
    glm::vec4 nT = glm::vec4(0, 0, 0,0), glm::vec4 nBT = glm::vec4(0, 0, 0,0))
    : loc(lc), normal(n), texLoc(t), tangent(nT), biTangent(nBT) {}

   // Transform loc by lXfm and normal by nXfm
   void ApplyXForm(const glm::mat4 &, const glm::mat3 &, const glm::mat4 &);
};

// A set of triangles, described by a series of vertex indices, interpreted in
// triangle-strip fashion to avoid needless repeats, but ultimately rendered as
// individual triangles, not a strip
struct TriangleSet {
   std::string mDescription;      // Description e.g. "table leg 4" for debug
   std::shared_ptr<Texture> mTex; // Texture to apply to this triangle set
   std::vector<uint> mIndices;     // Indices of vertices forming the set

   TriangleSet(std::string d, std::shared_ptr<Texture> t,
    const std::vector<uint> &i) : mDescription(d), mTex(t), mIndices(i) {}

   // Fill in a uint vector with values x in range [lo, hi). (Useful for many
   // index-vector creation cases.)
   static std::vector<uint> MakeRangeVec(int lo, int hi);
};

// Map from Texture pointers to lists of vectors of Vertexes.  
typedef std::map<std::shared_ptr<Texture>,
   std::list<std::vector<Vertex>>> VMap;

// Map from Texture pointers to lists of TriangleSets
typedef std::map<std::shared_ptr<Texture>,
   std::list<TriangleSet>> TMap;

typedef std::map<std::shared_ptr<Texture>, int> IMap;

typedef std::map<std::shared_ptr<Texture>, std::shared_ptr<Texture>> NMap;

// Abstract interface for all model scenetree nodes
class Model {
protected:
   // member data
   std::string mName;

public:
   Model(std::string n) : mName(n) {}
   virtual std::string getName() const {return mName;}

   virtual IMap GetNumVertices() const = 0;    // Count vertices per tex

   // Return all vertices in model, per texture, transformed by tfm
   virtual VMap GetVertices(const glm::mat4x4 &, const glm::mat4x4 &) const = 0;

   // Return all TriangleSets in model, per texture, index-referencing
   // the vertices.
   virtual TMap GetTriangles() const = 0;

   // return the normal for the texture
   virtual NMap GetNormal() const = 0;
};

// holds pointers to multiple models.
class CmpModel : public Model {
public:
   // member data
   struct Child {
      glm::mat4x4 xform;
      glm::mat4x4 texXfm;
      std::shared_ptr<Model> mdl;

      Child(const glm::mat4x4 &xf, 
       const glm::mat4x4 &texXfm, std::shared_ptr<Model> m)
       : xform(xf),texXfm(texXfm), mdl(std::move(m)) {}
   };

   CmpModel(std::string n, const std::vector<Child> &refs)
    : Model(n), mChildren(refs) {}

   void addChild(const glm::mat4x4 &xf, 
    const glm::mat4x4 &texXfm, std::shared_ptr<Model> m) {
      mChildren.push_back(Child(xf, texXfm, m));
   };

   IMap GetNumVertices() const override;
   VMap GetVertices(const glm::mat4x4 &, const glm::mat4x4 &) const override;
   TMap GetTriangles() const override;
   NMap GetNormal() const override;

protected:
   std::vector<Child> mChildren = std::vector<Child>();
};

// Model initialized via constructor-passed information
class DirectModel : public Model {
public:
   DirectModel(std::string n, std::vector<Vertex> vertices,
    std::vector<TriangleSet> meshes);
};

// Model initialized by file contents
class FileModel : public Model {
public:
   FileModel(std::string n, std::string fN);
};

// Model formed by an x/y figure defined by a parametric polar distance function, 
// sampled at points specified in the constructor, with domain 0.0 to 2pi
// Resultant cylinder is centered at NDC origin, stretching from -1 to 1 in Z, 
// and is closed at top and bottom.  X/Y extent is determined by the parametric 
// function, but it's expected to stay within -1 to 1 range in X and Y
class CylinderModel : public Model {
protected:
   // member data
   std::vector<float> mSamplePts; // Vals in [0, 2pi) on which to call polarDist
   std::shared_ptr<Texture> mTex;
   std::shared_ptr<Texture> mTexNormal;
   int mUReps;      // Number of tex repeats around cylinder (int to avoid seam)
   float mVReps;    // Number of tex repeats down cylinder

   virtual float polarDist(float x) const = 0;
public:
   CylinderModel(std::string n, std::shared_ptr<Texture> t, int uReps,
    float vReps, const std::vector<float> &pts,
    std::shared_ptr<Texture> nT) : Model(n), mTex(t),
    mUReps(uReps), mVReps(vReps), mSamplePts(pts), mTexNormal(nT) {}

   IMap GetNumVertices() const override;
   VMap GetVertices(const glm::mat4x4 &, const glm::mat4x4 &) const override;
   TMap GetTriangles() const override;
   NMap GetNormal() const override;
};

class CircleCylinderModel : public CylinderModel {
public: 
   CircleCylinderModel(std::string n, std::shared_ptr<Texture> t, int uReps,
    float vReps, const std::vector<float> &pts, std::shared_ptr<Texture> nT)
    : CylinderModel(n, t, uReps, vReps, pts, nT) {}

   float polarDist(float x) const override {return 1.0;}
};

// model formed by six verts and transformed into the 
// positions and sizes needed
class PlaneModel : public Model {
   // member data
   std::shared_ptr<Texture> mTex;
   std::shared_ptr<Texture> mTexNormal;
   std::vector<Vertex> mVerts;

public:
   PlaneModel(std::string n, std::shared_ptr<Texture> t,
    std::shared_ptr<Texture> nT);
   IMap GetNumVertices() const override;
   VMap GetVertices(const glm::mat4x4 &, const glm::mat4x4 &) const override;
   TMap GetTriangles() const override;
   NMap GetNormal() const override;
};

// perfect cube model, all sides have individual vertices for correct normals
class CubeModel : public Model {
   // member data
   std::shared_ptr<Texture> mTex;
   std::shared_ptr<Texture> mTexNormal;
   std::vector<Vertex> mVerts;

public:
   CubeModel(std::string n, std::shared_ptr<Texture> t,
    std::shared_ptr<Texture> nT);
   IMap GetNumVertices() const override;
   VMap GetVertices(const glm::mat4x4 &, const glm::mat4x4 &) const override;
   TMap GetTriangles() const override;
   NMap GetNormal() const override;
};