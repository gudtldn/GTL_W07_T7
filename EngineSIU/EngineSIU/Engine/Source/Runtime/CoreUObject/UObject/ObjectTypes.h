#pragma once


enum OBJECTS : uint8
{
    OBJ_SPHERE,
    OBJ_CUBE,
    OBJ_SPOTLIGHT,
    OBJ_POINTLIGHT,
    OBJ_DIRECTIONALLGIHT,
    OBJ_AMBIENTLIGHT,
    OBJ_PARTICLE,
    OBJ_TEXT,
    OBJ_FIREBALL,
    OBJ_TRIANGLE,
    OBJ_CAMERA,
    OBJ_PLAYER,
    OBJ_FOG,
    OBJ_PCM,
    OBJ_LUA_ACTOR,
    OBJ_LUA_COACH_ACTOR,
    OBJ_LUA_HEART_ACTOR,
    OBJ_LUA_GAMEMODE_ACTOR,
    OBJ_LUA_PLAYER,
    OBJ_END
};

enum ARROW_DIR : uint8
{
    AD_X,
    AD_Y,
    AD_Z,
    AD_END
};

enum EControlMode : uint8
{
    CM_TRANSLATION,
    CM_ROTATION,
    CM_SCALE,
    CM_END
};

enum ECoordMode : uint8
{
    CDM_WORLD,
    CDM_LOCAL,
    CDM_END
};

enum EPrimitiveColor : uint8
{
    RED_X,
    GREEN_Y,
    BLUE_Z,
    NONE,
    RED_X_ROT,
    GREEN_Y_ROT,
    BLUE_Z_ROT
};
