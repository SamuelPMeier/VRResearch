//#include "Model.h"
//
//using namespace std;
//using namespace glm;
//
//int main() {
//   vector<float> angles = { 1.5708f, 3.14159f, 4.71239f, 0.0f };
//   vector<Vertex> Verts;
//   vector<TriangleMesh> mesh;
//   vector<uint> indices;
//   int i = 0;
//   mat4x4 identity = mat4x4(1.0f);
//
//   CircleCylinderModel cyl = CircleCylinderModel("Test", NULL, 1, 1, angles);
//
//   Verts = cyl.getVertices(identity);
//   mesh = cyl.getMeshes();
//
//  /* for (Vertex v : Verts) {
//      printf("%d\nverts:%f, %f, %f, %f\n", i++, v.loc.x, v.loc.y, v.loc.z, v.loc.w);
//      printf("Normals: %f, %f, %f\n", v.normal.x, v.normal.y, v.normal.z);
//      printf("texLoc: %f, %f\n\n", v.texLoc.x, v.texLoc.y);
//   }*/
//
//   for (TriangleMesh m : mesh) {
//      printf("Mesh with %d indexes: ", m.indices.size());
//      for (uint i : m.indices) {
//         printf(" %d", i);
//      }
//      printf("\n");
//   }
//   /*
//   for (int x = 0; x < indices.size(); x++) {
//      printf("%d", indices[x]);
//      if (!((x + 1) % 3))
//         printf("\n");
//      else
//         printf(", ");
//   }
//   */
//
//}