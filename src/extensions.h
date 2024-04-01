#pragma once

#include <Windows.h>
#include <GL/gl.h>
#include <GL/glext.h>

struct RealEntryPoints;

struct Extensions {
    void load(RealEntryPoints &realEntryPoints);

    HGLRC lastLoadedContext = nullptr;
    bool hasVbo = false;
    bool hasVao = false;
    bool hasShaders = false;
    bool hasFbo = false;

    PFNGLGENBUFFERSPROC glGenBuffers;
    PFNGLDELETEBUFFERSPROC glDeleteBuffers;
    PFNGLBINDBUFFERPROC glBindBuffer;
    PFNGLBUFFERDATAPROC glBufferData;
    PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;

    PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
    PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
    PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
    PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
    PFNGLBINDVERTEXARRAYPROC glBindVertexArray;

    PFNGLCREATESHADERPROC glCreateShader;
    PFNGLSHADERSOURCEPROC glShaderSource;
    PFNGLCOMPILESHADERPROC glCompileShader;
    PFNGLGETSHADERIVPROC glGetShaderiv;
    PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
    PFNGLDELETESHADERPROC glDeleteShader;
    PFNGLCREATEPROGRAMPROC glCreateProgram;
    PFNGLDELETEPROGRAMPROC glDeleteProgram;
    PFNGLATTACHSHADERPROC glAttachShader;
    PFNGLLINKPROGRAMPROC glLinkProgram;
    PFNGLGETPROGRAMIVPROC glGetProgramiv;
    PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
    PFNGLUSEPROGRAMPROC glUseProgram;
    PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
    PFNGLUNIFORM1IPROC glUniform1i;
    PFNGLUNIFORM2IPROC glUniform2i;
    PFNGLUNIFORM4FVPROC glUniform4fv;
    PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv;

    PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
    PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebuffer;
    PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
    PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;

    PFNGLACTIVETEXTUREPROC glActiveTexture;
    PFNGLCLEARTEXIMAGEPROC glClearTexImage;
    PFNGLCOPYTEXSUBIMAGE2DEXTPROC glCopyTexSubImage2D;
};
