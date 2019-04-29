#include <sstream>
#include <cstdarg>
#include "MyLib.h"

using namespace std;

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

vector<string> Split(const std::string& s, char delimiter) {
   std::vector<std::string> tokens;
   std::string token;
   std::istringstream tokenStream(s);

   while (std::getline(tokenStream, token, delimiter)) {
      tokens.push_back(token);
   }
   return tokens;
}