#pragma once
#include <exception>
#include <string>
#include <vector>

typedef unsigned int uint;
typedef unsigned char uchar;

class WorldException : public std::exception {
   std::string mReason;

public:
   WorldException(std::string reason) : mReason(reason) {}

   const char *what() const noexcept override {return mReason.c_str();}
};

std::string StringPrintf(const std::string fmt, ...);

template <class Ptr>
struct LessPtr {
   int operator()(const Ptr &a, const Ptr &b) const { return *a < *b;}
};

std::vector<std::string> Split(const std::string& s, char delimiter);