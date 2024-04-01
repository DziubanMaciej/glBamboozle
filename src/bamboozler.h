#pragma once

#include <Windows.h>
#include <gl/GL.h>

struct RealEntryPoints;
struct Extensions;

#define SUB_OBJECT(Name)              \
    RealEntryPoints &realEntryPoints; \
    Extensions &extensions;           \
    Name(RealEntryPoints &r, Extensions &e) : extensions(e), realEntryPoints(r) {}

class Bamboozler {
public:
    Bamboozler(RealEntryPoints &realEntryPoints, Extensions &extensions);
    bool init();
    void deinit();

    BOOL wglSwapBuffers(HDC hdc);
    BOOL wglSwapLayerBuffers(HDC hdc, UINT flags);

private:
    Extensions &extensions;
    RealEntryPoints &realEntryPoints;

    bool applyPostProcesses(HDC hdc);
    static bool getDefaultFramebufferSize(HDC hdc, GLsizei *outW, GLsizei *outH);

    struct BamboozleSettings {
        float multiplyColor[4] = {1, 1, 1, 1};
        bool invertColor = false;
        bool flipX = false;
        bool flipY = false;
        bool highPassFilter = false;
        bool bounce = false;
        float bounceSpeed = 0.01f;
        float sizeMultiplier = 0.5f;
        int pixelation = 1;
    } bamboozleSettings;

    struct BounceState {
        GLsizei fbWidth;
        GLsizei fbHeight;

        float velocityX = 0;
        float velocityY = 0;
        float positionX = 0;
        float positionY = 0;

        bool update(float speed, float sizeMultiplier, float dt, GLsizei fbWidth, GLsizei fbHeight, GLint *outPosX, GLint *outPosY, GLsizei *outWidth, GLsizei *outHeight);
    } bounceState;

    struct GuiState {
        bool initialized = false;
        bool initializedSuccess = false;
        HHOOK wndProcHook;
        HHOOK getMsgHook;

        bool update(HINSTANCE module, HDC hdc, BamboozleSettings &bamboozleSettings);

        bool initialize(HINSTANCE module, HDC hdc);
        bool beginFrame();
        bool endFrame(BamboozleSettings &bamboozleSettings);

        static LRESULT CALLBACK imguiWndProcHook(int code, WPARAM wParam, LPARAM lParam);
        static LRESULT CALLBACK imguiGetMsgHook(int code, WPARAM wParam, LPARAM lParam);
    } guiState;

    struct ScratchTextures {
        SUB_OBJECT(ScratchTextures)

        constexpr static inline size_t numTextures = 2;
        GLuint textures[numTextures] = {};
        GLsizei width = {};
        GLsizei height = {};

        bool setup(GLsizei fbWidth, GLsizei fbHeight);
        void free();
        bool copyScreenToTexture(size_t textureIndex = 0, bool keepBinding = false);
        bool bindTexture(size_t textureIndex);
    } scratchTextures;

    struct FullscreenVertexBuffer {
        SUB_OBJECT(FullscreenVertexBuffer)

        bool initialized = false;
        GLuint vbo = {};
        GLuint vao = {};

        bool setup();
        void free();
    } fullscreenVertexBuffer;

    struct Program {
        SUB_OBJECT(Program)

        bool initialized = false;
        bool initializedSuccess = false;

        struct MultiplyColor {
            GLuint program = {};
            GLint uniformColor = {};
        } multiplyColor;

        struct InvertColor {
            GLuint program = {};
        } invertColor;

        struct Flip {
            GLuint program = {};
            GLint uniformFlipDimensions = {};
        } flip;

        struct KernelFilter {
            GLuint program = {};
            GLint uniformKernel = {};
            GLint uniformSampleStride = {};
        } kernelFilter;

        struct Passthrough {
            GLuint program = {};
        } passthrough;

        struct Pixelation {
            GLuint program = {};
            GLint uniformSize = {};
        } pixelation;

        bool setup();
        void free();
        GLuint compileShader(const char *source, GLenum shaderType);
        GLuint createProgram(const char *vertexShaderSource, const char *fragmentShaderSource);
        GLint getUniformLocation(GLuint program, const char *name);

        bool runMultiplyColor(float *color);
        bool runInvertColor();
        bool runFlip(bool flipX, bool flipY);
        bool runHighPassFilter();
        bool runBounce(ScratchTextures &scratchTextures, GLint posX, GLint posY, GLsizei width, GLsizei height);
        bool runPixelation(int size);
    } program;
};
