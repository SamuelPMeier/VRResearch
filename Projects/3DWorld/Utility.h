#pragma once

/// Includes ///
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/mat4x3.hpp>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <openvr.h>
#include <map>
#include <exception>
#include <string>
#include <vector>

/// Re-definitions /// 
typedef unsigned int uint;
typedef unsigned char uchar;

/// String Functions ///
std::string StringPrintf(const std::string fmt, ...);
std::vector<std::string> Split(const std::string& s, char delimiter);

/// Print Functions ///
void PrintMat(vr::HmdMatrix34_t mat);
void PrintMat(glm::mat4x4);
void PrintMat(glm::mat3x3);
void PrintMat(glm::mat4x3);

void PrintVec(glm::vec4);
void PrintVec(glm::vec3);
void PrintVec(glm::vec2);

void ClearConsole();

/// OpenGL error Checking ///
class WorldException : public std::exception {
   std::string mReason;

public:
   WorldException(std::string reason) : mReason(reason) {}
   const char *what() const noexcept override { return mReason.c_str(); }
};

/// redefines for inline error checking.
#define GLChkErr GLChkErrFtn(__LINE__, __FILE__)

void GLChkErrFtn(int, std::string);

/// Error map for easy access
const std::map<int, std::string> cError
{{GL_NO_ERROR, "GL_NO_ERROR"},
{GL_INVALID_ENUM, "GL_INVALID_ENUM"},
{GL_INVALID_VALUE, "GL_INVALID_VALUE"},
{GL_INVALID_OPERATION, "GL_INVALID_OPERATION"},
{GL_INVALID_FRAMEBUFFER_OPERATION, "GL_INVALID_FRAMEBUFFER_OPERATION"},
{GL_OUT_OF_MEMORY, "GL_OUT_OF_MEMORY"},
{GL_STACK_UNDERFLOW, "GL_STACK_UNDERFLOW"},
{GL_STACK_OVERFLOW, "GL_STACK_OVERFLOW"}};