#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include "Model.h"
#include <glm/gtx/matrix_transform_2d.hpp>

using namespace std;
using namespace glm;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * /
Vertex Functions
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/// applies passed-in transformation to single vertex
void Vertex::ApplyXForm(
 const mat4 &locXfm, const mat3 &normXFm, const mat4 &texXfm) {
   loc = locXfm * loc;
   normal = normXFm * normal;
   texLoc = vec2(texXfm * vec4(texLoc.x, texLoc.y, 0, 1));
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * /
TriangleSet Functions
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/// returns vector of all points betweeen lo and hi
vector<uint> TriangleSet::MakeRangeVec(int lo, int hi) {
   vector<uint> rtn(hi - lo);
   int idx = 0;

   for (int x = lo; x < hi; x++)
      rtn[idx++] = x;

   return rtn;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * /
CpmModel Functions
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// returns vertices for all child models attached to current CMPmodel,
// organized by bound texture.
VMap CmpModel::GetVertices(const mat4x4 &tfm, const mat4x4 &texXfm) const {
   VMap rtn;

   for (auto &cr: mChildren)
      for (
       auto &pair : cr.mdl->GetVertices(tfm * cr.xform, texXfm * cr.texXfm)) {
         rtn[pair.first].insert(
            rtn[pair.first].end(), pair.second.begin(), pair.second.end());
      }

   return rtn;
}

// returns indices for all child models attached to current CMPmodel
// organized by bound texture
TMap CmpModel::GetTriangles() const {
   TMap rtn;
   IMap basePerTex, offsPerTex;
   int offset;
   shared_ptr<Texture> tex;

   for (auto cr : mChildren) {
      // Add all sets per tex from cr, and adapt offsets to fit into larger set
      offsPerTex = cr.mdl->GetNumVertices();
      for (auto &pair : cr.mdl->GetTriangles()) {
         tex = pair.first;
         // Traverse all sets for |tex| and offset them
         offset = basePerTex[tex];
         for (auto &tSet: pair.second)
            for (auto &idx : tSet.mIndices)
               idx += offset;
         basePerTex[tex] = basePerTex[tex] + offsPerTex[tex];

         // Add adjusted list of triangls to end of returned set for this tex
         rtn[tex].splice(rtn[tex].end(), pair.second);
      }
   }

   return rtn;
}

// returns the number of vertexes that each child model has, broken up by tex.
IMap CmpModel::GetNumVertices() const {
   IMap rtn;

   for (auto cr : mChildren)
      for (auto pair: cr.mdl->GetNumVertices())
         rtn[pair.first] = rtn[pair.first] + pair.second;
   
   return rtn;
}

// get the normal map for each child model, organized by texture
NMap CmpModel::GetNormal() const {
   NMap rtn;

   for (auto cr : mChildren)
      for (auto pair : cr.mdl->GetNormal())
         rtn[pair.first] = pair.second;

   return rtn;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * /
CylinderModel Functions
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// returns the transformed vertices for the model
VMap CylinderModel::GetVertices(
 const mat4x4 &xfm, const mat4x4 &texXfm) const {
   uint nPts = (uint) mSamplePts.size();
   vector<Vertex> vtxs(nPts * 4);
   vec4 loc;
   vec3 norm;
   vec2 sideTexLoc;
   int idx = 0;
   float distance;
   VMap rtn;
   mat3 normXfm = inverse(transpose(mat3(xfm)));

   for (float angle : mSamplePts) {
      distance = polarDist(angle);
      loc = vec4(distance*cos(angle), distance*sin(angle), 1.0f, 1.0f);
      sideTexLoc = vec2(mUReps * std::min<float>(angle, M_PI-angle)
       / (2*M_PI), 0.0f);

      // Top
      vtxs[idx] = Vertex(loc, vec3(0, 0, 1),
       vec2((loc.x + 1.0)/2.0, (loc.y + 1.0)/2.0));

      // Side Top
      norm = vec3(cos(angle), sin(angle), 0);
      vtxs[nPts + 2*idx] = Vertex(loc, norm, sideTexLoc);

      // Move to bottom
      sideTexLoc[1] = mVReps;
      loc[2] = -1.0;
      vtxs[nPts + 2*idx + 1] = Vertex(loc, norm, sideTexLoc);

      // Bottom
      vtxs[3*nPts + idx++] = Vertex(loc, vec3(0, 0, -1),
       vec2((loc.x + 1.0)/2.0, (loc.y + 1.0)/2.0));
   }

   for (Vertex &v: vtxs)
      v.ApplyXForm(xfm, normXfm, texXfm);

   rtn[mTex].push_back(vtxs);

   return rtn;
}

// returns the indices for model
TMap CylinderModel::GetTriangles() const {
   uint numPts = (uint)mSamplePts.size();
   vector<uint> topIdxs, btmIdxs, sideIdxs, fullIdxs;
   TMap rtn;

   // Top
   topIdxs.push_back(0);
   for (uint x = 1; x <= numPts / 2; x++) {

      topIdxs.push_back(x);
      if (numPts - x > x)
         topIdxs.push_back(numPts - x);
   }

   // Bottom
   for (auto idx : topIdxs)
      btmIdxs.push_back(3*numPts + idx);

   sideIdxs = TriangleSet::MakeRangeVec(numPts, 3*numPts);

   for (int i = 0; i < numPts; i++) {
      fullIdxs.push_back(topIdxs[i]);
      if (i+1 < numPts)
         fullIdxs.push_back(topIdxs[i+1]);
      else
         fullIdxs.push_back(topIdxs[(i+1)-(numPts)]);
      if (i+2 < numPts)
         fullIdxs.push_back(topIdxs[i+2]);
      else
         fullIdxs.push_back(topIdxs[(i+2)-(numPts)]);
   }

   for (int i = 0; i < numPts*2; i++) {
      fullIdxs.push_back(sideIdxs[i]);
      if (i+1 < numPts*2)
         fullIdxs.push_back(sideIdxs[i+1]);
      else
         fullIdxs.push_back(sideIdxs[(i+1)-(numPts*2)]);
      if (i+2 < numPts*2)
         fullIdxs.push_back(sideIdxs[i+2]);
      else
         fullIdxs.push_back(sideIdxs[(i+2)-(numPts*2)]);
   }

   for (int i = 0; i < numPts; i++) {
      fullIdxs.push_back(btmIdxs[i]);
      if (i+1 < numPts)
         fullIdxs.push_back(btmIdxs[i+1]);
      else
         fullIdxs.push_back(btmIdxs[(i+1)-(numPts)]);
      if (i+2 < numPts)
         fullIdxs.push_back(btmIdxs[i+2]);
      else
         fullIdxs.push_back(btmIdxs[(i+2)-(numPts)]);
   }

   rtn[mTex].push_back(TriangleSet("triangles", mTex, fullIdxs));

   return rtn;
}
 
// returns the number of vertices used in model
IMap CylinderModel::GetNumVertices() const {
   IMap rtn;

   rtn[mTex] = 4 * (int)mSamplePts.size();

   return rtn;
} 

// returns the normal map for model
NMap CylinderModel::GetNormal() const {
   NMap rtn;

   rtn[mTex] = mTexNormal;

   return rtn;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * /
PlaneModel Functions
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// creates 1x1 vertical model
PlaneModel::PlaneModel(string n, shared_ptr<Texture> t,
   shared_ptr<Texture> nT) : Model(n), mTex(t), mTexNormal(nT) {
   vector<vec4> locs = {{.5,.5,0,1},{.5,-.5,0,1},{-.5,.5,0,1},{-.5,-.5,0,1}};
   vector<vec3> norms = {{0,0,1},{0,0,1},{0,0,1},{0,0,1}};
   vector<vec2> texLocs = {{1,0},{1,1},{0,0},{0,1}};

   for (int i = 0; i < 4; i++)
      mVerts.push_back(Vertex(locs[i], norms[i], texLocs[i]));
}

// returns indices for plane
TMap PlaneModel::GetTriangles() const {
   TMap rtn;
   vector<uint> inds = {0,1,2,1,2,3};
   rtn[mTex].push_back(TriangleSet("trinagles", mTex, inds));

   return rtn;
}

// returns vertices for plane
VMap PlaneModel::GetVertices(const mat4x4 &xfm, const mat4x4 &texXfm) const {
   vector<Vertex> verts;
   mat3 normXfm = inverse(transpose(mat3(xfm)));
   VMap rtn;

   for (Vertex v : mVerts) {
      v.ApplyXForm(xfm, normXfm, texXfm);
      verts.push_back(v);
   }

   rtn[mTex].push_back(verts);
   
   return rtn;
}

// returns the number ov vertices in plane
IMap PlaneModel::GetNumVertices() const {
   IMap rtn;

   rtn[mTex] = 4;

   return rtn;
}

// returns the normal map for plane
NMap PlaneModel::GetNormal() const {
   NMap rtn;

   rtn[mTex] = mTexNormal;

   return rtn;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * /
CubeModel Functions
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// creates a six sided perfect normal cube.
CubeModel::CubeModel(string n, shared_ptr<Texture> t,
   shared_ptr<Texture> nT) : Model(n), mTex(t), mTexNormal(nT) {
   vector<vec4> locs = {
      // z
    {-0.5,-0.5,0.5,0.5},{0.5,-0.5,0.5,0.5},
    {-0.5,0.5,0.5,0.5},{0.5,0.5,0.5,0.5},
      // -x
    {-0.5,-0.5,0.5,0.5},{-0.5,-0.5,-0.5,0.5},
    {-0.5,0.5,0.5,0.5},{-0.5,0.5,-0.5,0.5}, 
      // y
    {-0.5,0.5,0.5,0.5},{-0.5,0.5,-0.5,0.5},
    {0.5,0.5,0.5,0.5},{0.5,0.5,-0.5,0.5}, 
      // x
    {0.5,0.5,0.5,0.5},{0.5,0.5,-0.5,0.5},
    {0.5,-0.5,0.5,0.5},{0.5,-0.5,-0.5,0.5},
      // -y
    {0.5,-0.5,0.5,0.5},{0.5,-0.5,-0.5,0.5},
    {-0.5,-0.5,0.5,0.5},{-0.5,-0.5,-0.5,0.5}, 
      // -z
    {-0.5,-0.5,-0.5,0.5},{0.5,-0.5,-0.5,0.5},
    {-0.5,0.5,-0.5,0.5},{0.5,0.5,-0.5,0.5}};

   vector<vec3> norms = {
      // z
    {0,0,1},{0,0,1},{0,0,1},{0,0,1},
      // -x
    {-1,0,0},{-1,0,0},{-1,0,0},{-1,0,0},
      // y
    {0,1,0},{0,1,0},{0,1,0},{0,1,0},
      // x
    {1,0,0},{1,0,0},{1,0,0},{1,0,0},
      // -y
    {0,-1,0},{0,-1,0},{0,-1,0},{0,-1,0},
      // -z
    {0,0,-1},{0,0,-1},{0,0,-1},{0,0,-1}};

   vector<vec2> texLocs = {
      // z
    {1,0},{1,1},{0,0},{0,1},
      // -x
    {1,0},{1,1},{0,0},{0,1},
      // y
    {1,0},{1,1},{0,0},{0,1},
      // x
    {1,0},{1,1},{0,0},{0,1},
      // -y
    {1,0},{1,1},{0,0},{0,1},
      // -z
    {1,0},{1,1},{0,0},{0,1},};

   for (int i = 0; i < 24; i++)
      mVerts.push_back(Vertex(locs[i], norms[i], texLocs[i]));
}

// returns indices for cube model
TMap CubeModel::GetTriangles() const {
   TMap rtn;
   vector<uint> inds = {
      // z
    0,1,2, 1,2,3,
      // -x
    4,5,6, 5,6,7,
      // y
    8,9,10, 9,10,11,
      // x
    12,13,14, 13,14,15,
      // -y
    16,17,18, 17,18,19,
      // -z
    20,21,22, 21,22,23};

   rtn[mTex].push_back(TriangleSet("trinagles", mTex, inds));

   return rtn;
}

// returns vertices for cube model
VMap CubeModel::GetVertices(const mat4x4 &xfm, const mat4x4 &texXfm) const {
   vector<Vertex> verts;
   mat3 normXfm = inverse(transpose(mat3(xfm)));
   VMap rtn;

   for (Vertex v : mVerts) {
      v.ApplyXForm(xfm, normXfm, texXfm);
      verts.push_back(v);
   }

   rtn[mTex].push_back(verts);

   return rtn;
}

// returns number of vertices for cube model (always 24)
IMap CubeModel::GetNumVertices() const {
   IMap rtn;

   rtn[mTex] = 24;

   return rtn;
}

// returns normal map for model
NMap CubeModel::GetNormal() const {
   NMap rtn;

   rtn[mTex] = mTexNormal;

   return rtn;
}
