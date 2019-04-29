#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdarg>

#include "Windows.h"
#include "Utility.h"

using namespace glm;
using namespace std;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * /
 String Functions
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/// Returns inline formated string. (like printf)
string StringPrintf(const string fmt, ...) {
   int size = ((int)fmt.size()) * 2 + 500;   // Best guess
   string str;
   va_list ap;

   while (1) {     // Maximum two passes on a POSIX system...
      str.resize(size);
      va_start(ap, fmt);
      int n = vsnprintf((char *)str.data(), size, fmt.c_str(), ap);
      va_end(ap);

      if (n > -1 && n < size) {  // Everything worked
         str.resize(n);
         return str;
      }

      if (n > -1)       // Needed size was returned; resize and retry
         size = n + 1;
      else
         size *= 2;     // Guess at a larger size (OS specific)
   }
   return str;
}

/// Returns vector of strings split by delimiter char
vector<string> Split(const std::string& s, char delimiter) {
   std::vector<std::string> tokens;
   std::string token;
   std::istringstream tokenStream(s);

   while (std::getline(tokenStream, token, delimiter)) {
      tokens.push_back(token);
   }
   return tokens;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * /
Print Functions
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/// prints various mats. (4x4, 3x3, 4x3, VR::4x3)
void PrintMat(vr::HmdMatrix34_t mat) {
   printf(
      "[%-10f, %-10f, %-10f, %-10f\n"
      " %-10f, %-10f, %-10f, %-10f\n"
      " %-10f, %-10f, %-10f, %-10f\n",
      mat.m[0][0], mat.m[0][1], mat.m[0][2], mat.m[0][3],
      mat.m[1][0], mat.m[1][1], mat.m[1][2], mat.m[1][3],
      mat.m[2][0], mat.m[2][1], mat.m[2][2], mat.m[2][3]);
}

/// prints various mats. (4x4, 3x3, 4x3, VR::4x3)
void PrintMat(mat4x4 mat) {
   printf(
    "[%-10f, %-10f, %-10f, %-10f\n"
    " %-10f, %-10f, %-10f, %-10f\n"
    " %-10f, %-10f, %-10f, %-10f\n"
    " %-10f, %-10f, %-10f, %-10f]\n",
    mat[0][0], mat[0][1], mat[0][2], mat[0][3],
    mat[1][0], mat[1][1], mat[1][2], mat[1][3],
    mat[2][0], mat[2][1], mat[2][2], mat[2][3],
    mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
}

/// prints various mats. (4x4, 3x3, 4x3, VR::4x3)
void PrintMat(mat3x3 mat) {
   printf(
      "[%-10f, %-10f, %-10f\n"
      " %-10f, %-10f, %-10f\n"
      " %-10f, %-10f, %-10f]\n",
      mat[0][0], mat[0][1], mat[0][2],
      mat[1][0], mat[1][1], mat[1][2],
      mat[2][0], mat[2][1], mat[2][2]);
}

/// prints various mats. (4x4, 3x3, 4x3, VR::4x3)
void PrintMat(mat4x3 mat) {
   printf(
    "[%-10f, %-10f, %-10f\n"
    " %-10f, %-10f, %-10f\n"
    " %-10f, %-10f, %-10f\n"
    " %-10f, %-10f, %-10f]\n",
    mat[0][0], mat[0][1], mat[0][2],
    mat[1][0], mat[1][1], mat[1][2],
    mat[2][0], mat[2][1], mat[2][2],
    mat[3][0], mat[3][1], mat[3][2]);
}

/// prints various Vectors. (4, 3, 2)
void PrintVec(vec4 vec) {
   printf(
      "%-10f, %-10f, %-10f, %-10f\n",
      vec[0], vec[1], vec[2], vec[3]);
}

void PrintVec(vec3 vec) {
   printf(
      "%-10f, %-10f, %-10f\n",
      vec[0], vec[1], vec[2]);
}

void PrintVec(vec2 vec) {
   printf(
      "%-10f, %-10f\n",
      vec[0], vec[1]);
}

/// Resets the Command Line Console cursor to (0,0)
void ClearConsole() {
   COORD bs;
   bs.X = bs.Y = 0;
   SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), bs);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * /
Error Functions
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/// Checks the returned error code and prints out file, line of error + msg.
void GLChkErrFtn(int line, string file) {
   string errString;
   GLenum errCode;

   while (GL_NO_ERROR != (errCode = glGetError())) {
      printf("%d\n", errCode);
      if (errString.size() > 0)
         errString += ", ";
      errString += cError.at(errCode);
      if (errCode == GL_INVALID_OPERATION)
         break; // Special case to avoid endless loop
   }

   if (errString.size() > 0) {
      throw WorldException(
         StringPrintf("%s at line:%d:\n\t%s", file.c_str(), line,
          errString.c_str()));
   }
}

