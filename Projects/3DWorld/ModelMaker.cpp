#include <memory>
#include <glm/gtc/matrix_transform.hpp>
#include "ModelMaker.h"

#define M_PI 3.1415926535897

using namespace glm;
using namespace std;

using Chd = CmpModel::Child;

// single use transformation matricies
const mat4 moveBack1 = mat4(
 1, 0, 0, 0,
 0, 1, 0, 0,
 0, 0, 1, 0,
 -1, -1, 5, 1);
const mat4 moveBack2 = mat4(
 1, 0, 0, 0,
 0, 1, 0, 0,
 0, 0, 1, 0,
 1, 1, 5, 1);
const mat4 moveBack3 = mat4(
 1, 0, 0, 0,
 0, 1, 0, 0,
 0, 0, 1, 0,
 -1, 1, 5, 1);
const mat4 moveBack4 = mat4(
 1, 0, 0, 0,
 0, 1, 0, 0,
 0, 0, 1, 0,
 1, -1, 5, 1);
const mat4 moveBack5 = mat4(
 1, 0, 0, 0,
 0, 1, 0, 0,
 0, 0, 1, 0,
 0, 0, 5, 1);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * /
Cube Model
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

shared_ptr<Model> CubeMaker::MakeModel() {
   vector<float> angles{0.0f, 1.5708f, 3.14159f, 4.71239f};

   // textures/normal Maps
   shared_ptr<Texture> tex(new TexturePng("cubeTex", mTexPath, true));

   // individual transformations
   mat4 squash = scale(mat4(1.0f), vec3(1.0, 1.0, .707)) * moveBack1;

   // model hierarchy
   return shared_ptr<Model>(new CmpModel(string("cube"), vector<Chd>{
    Chd(squash, mat4(1.0f),
    shared_ptr<Model>(
     new CircleCylinderModel("cyl", tex, 4, 1.0f, angles, nullptr)))
   }));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * /
Multi-Cube Model
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

shared_ptr<Model> MultiCubeMaker::MakeModel() {
   vector<float> angles{0.0f, 1.5708f, 3.14159f, 4.71239f};
   mat4 squash = scale(mat4(1.0f), vec3(1.0, 1.0, .707));

   // textures/normal Maps
   shared_ptr<Texture> tex1(new TexturePng("cubeTex", mTexPath1, true));
   shared_ptr<Texture> tex2(new TexturePng("WhiteCube", mTexPath2, true));

   // model hierarchy
   shared_ptr<Model> parent = shared_ptr<Model>(new 
    CmpModel(string("parent"), vector<Chd>{
    Chd(moveBack1 * squash, mat4(1.0f),
     shared_ptr<Model>(
     new CircleCylinderModel("cyl", tex1, 4, 1.0f, angles, nullptr))),
    Chd(moveBack2 * squash, mat4(1.0f),
    shared_ptr<Model>(
     new CircleCylinderModel("cyl", tex1, 4, 1.0f, angles, nullptr))),
    Chd(moveBack3 * squash, mat4(1.0f),
    shared_ptr<Model>(
     new CircleCylinderModel("cyl", tex2, 4, 1.0f, angles, nullptr))),
    Chd(moveBack4 * squash, mat4(1.0f),
    shared_ptr<Model>(
     new CircleCylinderModel("cyl", tex2, 4, 1.0f, angles, nullptr))),
    Chd(moveBack5 * squash, mat4(1.0f),
    shared_ptr<Model>(
     new CircleCylinderModel("cyl", tex2, 4, 1.0f, angles, nullptr)))})
    );

   return parent;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * /
Room Model
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

shared_ptr<Model> RoomMaker::MakeModel() {
   int wallMove = mRoomSize / 2;

   // textures/normal Maps
   shared_ptr<Texture> texFloor(new TexturePng("FloorTex", mFloor, true));
   shared_ptr<Texture> texWall(new TexturePng("wallTex", mWall, true));
   shared_ptr<Texture> texCeiling(
      new TexturePng("ceilingTex", mCeiling, true));

   shared_ptr<Texture> texFloor_N(
    new TextureNormal("FloorTex_N", mFloor_N, true));
   shared_ptr<Texture> texWNrml(
    new TextureNormal("wallTex_N", mWallNormal, true));
   shared_ptr<Texture> texCeilingNorm(
    new TextureNormal("ceilingTex_N", mCeiling_N, true));

   // individual transformations
   mat4 floorScale = scale(mat4(1.0f), vec3(mRoomSize, mRoomSize, 1));
   mat4 wallScale = scale(mat4(1.0f), vec3(mRoomSize, 2.5, 1));

   mat4 floorRot = rotate(mat4(1.0f), (float)-(M_PI / 2), vec3(1, 0, 0));
   mat4 ceilingRot = rotate(mat4(1.0f), (float)(M_PI / 2), vec3(1, 0, 0)) * 
      rotate(mat4(1.0f), (float)(M_PI), vec3(0, 0, 1));;

   mat4 northRotation = rotate(mat4(1.0f),(float)-(M_PI/2),vec3(0,1,0));
   mat4 southRotation = rotate(mat4(1.0f),(float)(M_PI/2),vec3(0,1,0));
   mat4 eastRotation = rotate(mat4(1.0f), (float)(M_PI), vec3(0, 1, 0));

   mat4 ceilingTranslate = translate(mat4(1.0f), vec3(0,2.25,0));
   mat4 northTranslateWall = translate(mat4(1.0f), vec3(wallMove, 1, 0));
   mat4 southTranslateWall = translate(mat4(1.0f), vec3(-wallMove, 1, 0));
   mat4 eastTranslateWall = translate(mat4(1.0f), vec3(0, 1, wallMove));
   mat4 westTranslateWall = translate(mat4(1.0f), vec3(0, 1, -wallMove));

   // model hierarchy
   shared_ptr<Model> room = shared_ptr<Model>(new
    CmpModel(string("floor"), vector<Chd>{
      Chd(floorRot * floorScale, floorScale,
         shared_ptr<Model>(new PlaneModel("plane", texFloor, texFloor_N))),
      Chd(northTranslateWall * northRotation * wallScale, wallScale,
         shared_ptr<Model>(new PlaneModel("plane", texWall, texWNrml))),
      Chd(southTranslateWall *southRotation * wallScale, wallScale,
         shared_ptr<Model>(new PlaneModel("plane", texWall, texWNrml))),
      Chd(eastTranslateWall * eastRotation * wallScale, wallScale,
         shared_ptr<Model>(new PlaneModel("plane", texWall, texWNrml))),
      Chd(westTranslateWall * wallScale, wallScale,
         shared_ptr<Model>(new PlaneModel("plane", texWall, texWNrml))),
      Chd(ceilingTranslate * ceilingRot * floorScale, floorScale,
         shared_ptr<Model>(
         new PlaneModel("plane", texCeiling, texCeilingNorm))),
      Chd(translate(mat4(1.0f), vec3(1.3, 0, 1.3)), mat4(1.0), TableMaker(0.5f, 0.5f, 0.4f).MakeModel())
    }));

   return room;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * /
Table Model
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

shared_ptr<Model> TableMaker::MakeModel() {
   float legXmove = mWidth-0.1f;
   float legYmove = mLength-0.1f;

   // textures/normal Maps
   shared_ptr<Texture> texTop(new TexturePng("TTopTex", mTop, true));
   shared_ptr<Texture> texLeg(new TexturePng("TLegTex", mLegs, true));
   shared_ptr<Texture> texTop_N(new TextureNormal("TTopTex_N", mTopN, true));
   shared_ptr<Texture> texLeg_N(new TextureNormal("TLegTex_N", mLegsN, true));

   // individual transformations
   mat4 topScale = scale(mat4(1.0f), vec3(mWidth, mLength, 0.02f));
   mat4 topTrans = translate(mat4(1.0f), vec3(0,mHeight*2,0));
   mat4 topRot = rotate(mat4(1.0f), (float)(M_PI / 2), vec3(1, 0, 0));

   mat4 legScale = scale(mat4(1.0f), vec3(0.05f, 0.05f, mHeight));
   mat4 legRot = rotate(mat4(1.0f), (float)-(M_PI / 2), vec3(1, 0, 0));
   mat4 leg1Trans = translate(mat4(1.0f), vec3(legXmove,mHeight,legYmove));
   mat4 leg2Trans = translate(mat4(1.0f), vec3(legXmove,mHeight,-legYmove));
   mat4 leg3Trans = translate(mat4(1.0f), vec3(-legXmove,mHeight,legYmove));
   mat4 leg4Trans = translate(mat4(1.0f), vec3(-legXmove,mHeight,-legYmove));

   // model hierarchy
   shared_ptr<Model> table = shared_ptr<Model>(new
    CmpModel(string("floor"), vector<Chd>{
     Chd(leg1Trans * legRot * legScale, mat4(1.0f),
      shared_ptr<Model>(new CubeModel("Leg1", texLeg, texLeg_N))),
     Chd(leg2Trans * legRot * legScale, mat4(1.0f),
      shared_ptr<Model>(new CubeModel("Leg2", texLeg, texLeg_N))),
     Chd(leg3Trans * legRot * legScale, mat4(1.0f),
      shared_ptr<Model>(new CubeModel("Leg3", texLeg, texLeg_N))),
     Chd(leg4Trans * legRot * legScale, mat4(1.0f),
      shared_ptr<Model>(new CubeModel("Leg4", texLeg, texLeg_N))),
     Chd(topTrans * topRot * topScale, mat4(1.0f),
      shared_ptr<Model>(new CubeModel("TTop", texTop, texTop_N)))
   }));

   return table;
}