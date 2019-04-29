#pragma once
#include <memory>
#include "Model.h"

// Subclasses of ModelMaker make a scenegraph (Model) and return it via
// MakeModel.  

class ModelMaker {
public:
   ModelMaker() {};
   virtual ~ModelMaker() {};

   virtual std::shared_ptr<Model> MakeModel() = 0;
};

class CubeMaker : public ModelMaker {
   // member data
   float mSize;

   // texture location constants
   const char *mTexPath = "Resource/cube_texture.png";

public:
   CubeMaker(float size) : mSize(size) {}
   std::shared_ptr<Model> MakeModel() override;
};

class MultiCubeMaker : public ModelMaker {
   // member data
   float mSize;

   // texture location constants
   const char *mTexPath1 = "Resource/cube_texture.png";
   const char *mTexPath2 = "Resource/white_texture.png";

public:
   MultiCubeMaker(float size) : mSize(size) {}
   std::shared_ptr<Model> MakeModel() override;
};

class RoomMaker : public ModelMaker {
   // member data
   int mRoomSize;

   // texture location constants
   const char *mFloor = "Resource/Blue_texture.png";
   const char *mWall = "Resource/DirtyBrick_S.png";
   const char *mCeiling = "Resource/ceiling_texture.png";
   const char *mFloor_N = "Resource/Blue_texture_N.png";
   const char *mCeiling_N = "Resource/ceiling_texture_N.png";
   const char *mWallNormal = "Resource/DirtyBrick_N.png";

public:
   RoomMaker(int r) : mRoomSize(r) {};
   std::shared_ptr<Model> MakeModel() override;
};

class TableMaker : public ModelMaker {
   // member data
   float mHeight;
   float mWidth;
   float mLength;

   // texture location constants
   const char *mTop = "Resource/top2_texture.png";
   const char *mLegs = "Resource/leg_texture.png";
   const char *mTopN = "Resource/top2_texture_N.png";
   const char *mLegsN = "Resource/leg_texture_N.png";

public:
   TableMaker(float x, float y, float z) 
    : mWidth(x), mHeight(z), mLength(y) {};
   std::shared_ptr<Model> MakeModel() override;
};