#pragma once

#include "glad/glad.h"

namespace Utils
{

void CheckGLError();

}

#ifndef RELEASE_CFG
    #define CHECKGLERROR Utils::CheckGLError()
#else
    #define CHECKGLERROR (void)0
#endif