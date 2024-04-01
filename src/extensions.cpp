#include "extensions.h"
#include "real_entrypoints.h"

#define FUNCTION(name, capability)                                                     \
    name = reinterpret_cast<decltype(name)>(realEntryPoints.wglGetProcAddress(#name)); \
    if (name == nullptr)                                                               \
        capability = false;

void Extensions::load(RealEntryPoints &realEntryPoints) {
    HGLRC currentContext = realEntryPoints.wglGetCurrentContext();
    if (currentContext == lastLoadedContext) {
        return;
    }
    lastLoadedContext = currentContext;

    hasVbo = true;
    FUNCTION(glGenBuffers, hasVbo)
    FUNCTION(glDeleteBuffers, hasVbo)
    FUNCTION(glBindBuffer, hasVbo)
    FUNCTION(glBufferData, hasVbo)
    FUNCTION(glVertexAttribPointer, hasVbo)

    hasVao = true;
    FUNCTION(glGenVertexArrays, hasVao)
    FUNCTION(glDisableVertexAttribArray, hasVao)
    FUNCTION(glEnableVertexAttribArray, hasVao)
    FUNCTION(glDeleteVertexArrays, hasVao)
    FUNCTION(glBindVertexArray, hasVao)

    hasShaders = true;
    FUNCTION(glCreateShader, hasShaders)
    FUNCTION(glShaderSource, hasShaders)
    FUNCTION(glCompileShader, hasShaders)
    FUNCTION(glGetShaderiv, hasShaders)
    FUNCTION(glGetShaderInfoLog, hasShaders)
    FUNCTION(glDeleteShader, hasShaders)
    FUNCTION(glCreateProgram, hasShaders)
    FUNCTION(glDeleteProgram, hasShaders)
    FUNCTION(glAttachShader, hasShaders)
    FUNCTION(glLinkProgram, hasShaders)
    FUNCTION(glGetProgramiv, hasShaders)
    FUNCTION(glGetProgramInfoLog, hasShaders)
    FUNCTION(glUseProgram, hasShaders)
    FUNCTION(glGetUniformLocation, hasShaders)
    FUNCTION(glUniform1i, hasShaders)
    FUNCTION(glUniform2i, hasShaders)
    FUNCTION(glUniform4fv, hasShaders)
    FUNCTION(glUniformMatrix3fv, hasShaders)

    hasFbo = true;
    FUNCTION(glGenFramebuffers, hasFbo)
    FUNCTION(glBindFramebuffer, hasFbo)
    FUNCTION(glFramebufferTexture2D, hasFbo)
    FUNCTION(glDeleteFramebuffers, hasFbo)

    bool hasOther = true;
    FUNCTION(glActiveTexture, hasOther);
    FUNCTION(glClearTexImage, hasOther);
    FUNCTION(glCopyTexSubImage2D, hasOther);
}
