#define _USE_MATH_DEFINES
#include <cmath>
#include "Model.h"

using namespace std;
using namespace glm;

vector<uint> TriangleMesh::MakeRangeVec(int lo, int hi) {
   vector<uint> rtn(hi - lo);
   int idx = 0;

   for (int x = lo; x < hi; x++)
      rtn[idx++] = x;

   return rtn;
}

vector<Vertex> CompositeModel::getVertices(const glm::mat4x4 &tfm) const {
   vector<Vertex> rtn;

   for (auto &cr: children)
      for (auto &vtx : cr.mdl->getVertices(tfm * cr.xform))
         rtn.push_back(vtx);

   return rtn;
}

vector<TriangleMesh> CompositeModel::getMeshes() const {
   vector<TriangleMesh> rtn;
   int idxBase = 0;

   for (auto cr : children) {
      // Add all meshes from cr, and adapt offsets to fit into larger set
      for (auto &tm : cr.mdl->getMeshes()) {
         rtn.push_back(tm);
         for (auto &idx : rtn.back().indices)
            idx += idxBase;
      }
      idxBase += cr.mdl->getNumVertices();
   }

   return rtn;
}

int CompositeModel::getNumVertices() const {
   int rtn = 0;

   for (auto cr : children)
      rtn += cr.mdl->getNumVertices();

   return rtn;
}


vector<Vertex> CylinderModel::getVertices(const mat4x4 &tfm) const {
   uint nPts = (uint) mSamplePts.size();
   vector<Vertex> rtn(nPts * 4);
   vec4 loc;
   vec3 norm;
   vec2 texLoc;
   int idx = 0;
   float distance;

   for (float angle : mSamplePts) {
      distance = polarDist(angle);
      loc = tfm * vec4(distance*cos(angle), distance*sin(angle), 1.0f, 1.0f);
      texLoc = vec2(mUReps * angle / (2 * M_PI), mVReps);

      // Top
      rtn[idx] = Vertex(loc, vec3(0, 0, 1), texLoc);

      // Side Top
      norm = vec3(cos(angle), sin(angle), 0);
      rtn[nPts + 2*idx] = Vertex(loc, norm, texLoc);

      // Move to bottom
      texLoc[1] = 0.0;
      loc[2] = -1.0;
      rtn[nPts + 2*idx + 1] = Vertex(loc, norm, texLoc);

      // Bottom
      rtn[3*nPts + idx++] = Vertex(loc, vec3(0, 0, -1), texLoc);
   }

   return rtn;
}

vector<TriangleMesh> CylinderModel::getMeshes() const {
   uint numPts = (uint)mSamplePts.size();
   vector<uint> topIdxs, btmIdxs;

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

   TriangleMesh top(mName + "(top)", mTex, topIdxs);
   TriangleMesh sides(mName + "(sides)", mTex,
    TriangleMesh::MakeRangeVec(numPts, 3*numPts));
   TriangleMesh bottom(mName + "(bottom)", mTex, btmIdxs);

   return vector<TriangleMesh>{top, sides, bottom};
}

int CylinderModel::getNumVertices() const {
   return 4 * (int)mSamplePts.size();
} 