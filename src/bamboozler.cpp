#include "bamboozler.h"
#include "extensions.h"
#include "log.h"
#include "real_entrypoints.h"

#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_win32.h>
#include <commctrl.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <imgui.h>
#include <memory>

#define CHECK_GL_ERROR(statement)                  \
    {                                              \
        statement;                                 \
        GLenum err = realEntryPoints.glGetError(); \
        if (err != GL_NO_ERROR) {                  \
            printf("GL ERROR: 0x%x\n", err);       \
            return {};                             \
        }                                          \
    }

#define CHECK_BOOLEAN_ERROR(statement) \
    {                                  \
        const bool result = statement; \
        if (!result) {                 \
            printf("Boolean error");   \
            return {};                 \
        }                              \
    }

extern HINSTANCE glBamboozleInstance;


Bamboozler::Bamboozler(RealEntryPoints &realEntryPoints, Extensions &extensions)
    : realEntryPoints(realEntryPoints),
      extensions(extensions),
      scratchTextures(realEntryPoints, extensions),
      fullscreenVertexBuffer(realEntryPoints, extensions),
      program(realEntryPoints, extensions) {}

bool Bamboozler::init() {
    srand(time(NULL));
    return true;
}

void Bamboozler::deinit() {
    program.free();
    fullscreenVertexBuffer.free();
    scratchTextures.free();
}

struct StateSave {
    StateSave(RealEntryPoints &realEntryPoints, Extensions &extensions) : realEntryPoints(realEntryPoints), extensions(extensions) {
        realEntryPoints.glGetIntegerv(GL_TEXTURE_BINDING_2D, reinterpret_cast<GLint *>(&texture2D));
        realEntryPoints.glGetIntegerv(GL_VERTEX_ARRAY_BINDING, reinterpret_cast<GLint *>(&vao));
        if (extensions.hasFbo) {
            realEntryPoints.glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, reinterpret_cast<GLint *>(&drawFbo));
            realEntryPoints.glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, reinterpret_cast<GLint *>(&readFbo));
        }
        if (extensions.hasShaders) {
            realEntryPoints.glGetIntegerv(GL_CURRENT_PROGRAM, reinterpret_cast<GLint *>(&program));
        }
    }
    ~StateSave() {
        realEntryPoints.glBindTexture(GL_TEXTURE_2D, texture2D);
        extensions.glBindVertexArray(vao);
        if (extensions.hasFbo) {
            extensions.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFbo);
            extensions.glBindFramebuffer(GL_READ_FRAMEBUFFER, readFbo);
        }
        if (extensions.hasShaders) {
            extensions.glUseProgram(program);
        }
    }

    RealEntryPoints &realEntryPoints;
    Extensions &extensions;
    GLuint texture2D = {};
    GLuint vao = {};
    GLuint drawFbo = {};
    GLuint readFbo = {};
    GLuint program = {};
};

BOOL Bamboozler::wglSwapBuffers(HDC hdc) {
    applyPostProcesses(hdc);
    guiState.update(glBamboozleInstance, hdc, bamboozleSettings);
    BOOL result = realEntryPoints.wglSwapBuffers(hdc);

    // Restore the framebuffer
    // TODO: This is only needed for apps, which read the framebuffer after swapping. Most apps
    // don't do it. Hence, this is left unimplemented for now.

    return result;
}

BOOL Bamboozler::wglSwapLayerBuffers(HDC hdc, UINT flags) {
    return wglSwapBuffers(hdc);
}

bool Bamboozler::applyPostProcesses(HDC hdc) {
    StateSave stateSave{realEntryPoints, extensions};

    // Get framebuffer size
    GLsizei fbWidth{};
    GLsizei fbHeight{};
    if (!getDefaultFramebufferSize(hdc, &fbWidth, &fbHeight)) {
        return false;
    }

    // Setup sub-objects states
    CHECK_BOOLEAN_ERROR(scratchTextures.setup(fbWidth, fbHeight));
    CHECK_BOOLEAN_ERROR(fullscreenVertexBuffer.setup());
    CHECK_BOOLEAN_ERROR(program.setup());

    // Prepare state for post-processes
    CHECK_GL_ERROR(extensions.glBindBuffer(GL_ARRAY_BUFFER, 0));
    CHECK_GL_ERROR(extensions.glBindVertexArray(fullscreenVertexBuffer.vao));
    CHECK_GL_ERROR(extensions.glEnableVertexAttribArray(0));
    CHECK_GL_ERROR(realEntryPoints.glDisable(GL_BLEND));
    CHECK_GL_ERROR(realEntryPoints.glDisable(GL_DEPTH_TEST));

    // Execute post-processes
    if (bamboozleSettings.multiplyColor[0] != 1 || bamboozleSettings.multiplyColor[1] != 1 ||
        bamboozleSettings.multiplyColor[2] != 1 || bamboozleSettings.multiplyColor[3] != 1) {
        CHECK_BOOLEAN_ERROR(scratchTextures.copyScreenToTexture());
        CHECK_BOOLEAN_ERROR(program.runMultiplyColor(bamboozleSettings.multiplyColor));
    }
    if (bamboozleSettings.invertColor) {
        CHECK_BOOLEAN_ERROR(scratchTextures.copyScreenToTexture());
        CHECK_BOOLEAN_ERROR(program.runInvertColor());
    }
    if (bamboozleSettings.flipX || bamboozleSettings.flipY) {
        CHECK_BOOLEAN_ERROR(scratchTextures.copyScreenToTexture());
        CHECK_BOOLEAN_ERROR(program.runFlip(bamboozleSettings.flipX, bamboozleSettings.flipY));
    }
    if (bamboozleSettings.highPassFilter) {
        CHECK_BOOLEAN_ERROR(scratchTextures.copyScreenToTexture());
        CHECK_BOOLEAN_ERROR(program.runHighPassFilter());
    }
    if (bamboozleSettings.bounce) {
        CHECK_BOOLEAN_ERROR(scratchTextures.copyScreenToTexture());
        GLint posX{};
        GLint posY{};
        GLsizei width{};
        GLsizei height{};
        CHECK_BOOLEAN_ERROR(bounceState.update(bamboozleSettings.bounceSpeed, bamboozleSettings.sizeMultiplier, 0.1f, fbWidth, fbHeight, &posX, &posY, &width, &height));
        CHECK_BOOLEAN_ERROR(program.runBounce(scratchTextures, posX, posY, width, height));
    } else {
        bounceState = {};
    }
    if (bamboozleSettings.pixelation > 1) {
        CHECK_BOOLEAN_ERROR(scratchTextures.copyScreenToTexture());
        CHECK_BOOLEAN_ERROR(program.runPixelation(bamboozleSettings.pixelation));
    }

    return true;
}

bool Bamboozler::getDefaultFramebufferSize(HDC hdc, GLsizei *outW, GLsizei *outH) {
    HWND hwnd = WindowFromDC(hdc);
    if (hwnd == nullptr) {
        return false;
    }

    RECT clientRect{};
    if (GetClientRect(hwnd, &clientRect) == 0) {
        return false;
    }

    *outW = static_cast<GLsizei>(clientRect.right - clientRect.left);
    *outH = static_cast<GLsizei>(clientRect.bottom - clientRect.top);
    return true;
}

// ---------------------------------------------------------- BounceState subobject

bool Bamboozler::BounceState::update(float speed, float sizeMultiplier, float dt, GLsizei fbWidth, GLsizei fbHeight, GLint *outPosX, GLint *outPosY, GLsizei *outWidth, GLsizei *outHeight) {
    const float maxPositionX = fbWidth * (1 - sizeMultiplier);
    const float maxPositionY = fbHeight * (1 - sizeMultiplier);

    if (this->fbWidth != fbWidth || this->fbHeight != fbHeight) {
        this->fbWidth = fbWidth;
        this->fbHeight = fbHeight;
        this->positionX = maxPositionX / 2;
        this->positionY = maxPositionY / 2;
        const float velocityAngle = static_cast<float>(rand()) / (2 * 3.141592);
        this->velocityX = speed * cos(velocityAngle);
        this->velocityY = speed * sin(velocityAngle);
    } else {
        // Scale velocity to new value
        float currentVelocity2 = velocityX * velocityX + velocityY * velocityY;
        float multiplier2 = speed * speed / currentVelocity2;
        float multiplier = sqrtf(multiplier2);
        velocityX *= multiplier;
        velocityY *= multiplier;

        // Move by velocity
        positionX += velocityX * dt * fbWidth;
        positionY += velocityY * dt * fbHeight;

        // If hit edge, clamp the position to valid coordinates and flip the velocity. Not
        // completely physically correct, but it doesn't matter here.
        if (positionX < 0) {
            positionX = 0;
            velocityX = -velocityX;
        }
        if (positionX > maxPositionX) {
            positionX = maxPositionX;
            velocityX = -velocityX;
        }
        if (positionY < 0) {
            positionY = 0;
            velocityY = -velocityY;
        }
        if (positionY > maxPositionY) {
            positionY = maxPositionY;
            velocityY = -velocityY;
        }
    }

    *outPosX = this->positionX;
    *outPosY = this->positionY;
    *outWidth = this->fbWidth * sizeMultiplier;
    *outHeight = this->fbHeight * sizeMultiplier;

    return true;
}

// ---------------------------------------------------------- GuiState subobject

bool Bamboozler::GuiState::update(HINSTANCE module, HDC hdc, BamboozleSettings &bamboozleSettings) {
    bool firstFrame = !initialized;

    if (firstFrame) {
        initialized = true;
        CHECK_BOOLEAN_ERROR(initialize(module, hdc));
        initializedSuccess = true;
    }
    CHECK_BOOLEAN_ERROR(initializedSuccess);

    if (!firstFrame) {
        CHECK_BOOLEAN_ERROR(endFrame(bamboozleSettings));
    }
    CHECK_BOOLEAN_ERROR(beginFrame());
    return true;
}

bool Bamboozler::GuiState::initialize(HINSTANCE module, HDC hdc) {
    HWND hwnd = WindowFromDC(hdc);
    if (hwnd == nullptr) {
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    CHECK_BOOLEAN_ERROR(ImGui_ImplWin32_InitForOpenGL(hwnd));
    CHECK_BOOLEAN_ERROR(ImGui_ImplOpenGL3_Init());

    wndProcHook = SetWindowsHookExA(
        WH_CALLWNDPROC,
        imguiWndProcHook,
        module,
        0);
    CHECK_BOOLEAN_ERROR(wndProcHook != nullptr);

    getMsgHook = SetWindowsHookExA(
        WH_GETMESSAGE,
        imguiGetMsgHook,
        module,
        0);
    CHECK_BOOLEAN_ERROR(getMsgHook != nullptr);

    return true;
}

bool Bamboozler::GuiState::beginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    return true;
}

bool Bamboozler::GuiState::endFrame(BamboozleSettings &bamboozleSettings) {
    ImGui::Begin("glBamboozle");
    ImGui::SetNextItemWidth(100);
    ImGui::ColorPicker3("Multiply color", bamboozleSettings.multiplyColor, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoInputs);
    ImGui::Checkbox("Invert colors", &bamboozleSettings.invertColor);
    ImGui::Checkbox("Flip horizontally", &bamboozleSettings.flipX);
    ImGui::Checkbox("Flip vertically", &bamboozleSettings.flipY);
    ImGui::Checkbox("High pass filter", &bamboozleSettings.highPassFilter);
    ImGui::Checkbox("Bounce", &bamboozleSettings.bounce);
    if (!bamboozleSettings.bounce) {
        ImGui::BeginDisabled();
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    ImGui::SliderFloat("Speed", &bamboozleSettings.bounceSpeed, 0.001f, 0.5f);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    ImGui::SliderFloat("Size", &bamboozleSettings.sizeMultiplier, 0.01f, 0.99f);
    if (!bamboozleSettings.bounce) {
        ImGui::EndDisabled();
    }
    ImGui::SliderInt("Pixelate", &bamboozleSettings.pixelation, 1, 10);
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    return true;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK Bamboozler::GuiState::imguiWndProcHook(int code, WPARAM wParam, LPARAM lParam) {
    auto data = reinterpret_cast<const CWPSTRUCT *>(lParam);
    ImGui_ImplWin32_WndProcHandler(data->hwnd, data->message, data->wParam, data->lParam);
    return CallNextHookEx(NULL, code, wParam, lParam);
}

LRESULT CALLBACK Bamboozler::GuiState::imguiGetMsgHook(int code, WPARAM wParam, LPARAM lParam) {
    auto data = reinterpret_cast<const MSG *>(lParam);
    ImGui_ImplWin32_WndProcHandler(data->hwnd, data->message, data->wParam, data->lParam);
    return CallNextHookEx(NULL, code, wParam, lParam);
}

// ---------------------------------------------------------- ScratchTextures subobject

bool Bamboozler::ScratchTextures::setup(GLsizei fbWidth, GLsizei fbHeight) {
    // Create texture name
    if (textures[0] == 0) {
        CHECK_GL_ERROR(realEntryPoints.glGenTextures(numTextures, textures));
    }

    // Allocate texture with the same size as framebuffer
    if (fbWidth != this->width || fbHeight != this->height) {
        // Allocate
        for (size_t i = 0; i < numTextures; i++) {
            realEntryPoints.glBindTexture(GL_TEXTURE_2D, textures[i]);
            CHECK_GL_ERROR(realEntryPoints.glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RGBA8, // TODO can we query this?
                fbWidth,
                fbHeight,
                0,
                GL_RGBA,
                GL_FLOAT,
                nullptr));
            CHECK_GL_ERROR(realEntryPoints.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
            CHECK_GL_ERROR(realEntryPoints.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
            this->width = fbWidth;
            this->height = fbHeight;
        }
    }

    return true;
}

void Bamboozler::ScratchTextures::free() {
    realEntryPoints.glDeleteTextures(numTextures, textures);
    memset(textures, 0, sizeof(textures));
}

bool Bamboozler::ScratchTextures::copyScreenToTexture(size_t textureIndex, bool keepBinding) {
    GLuint savedTextureBinding = {};
    if (keepBinding) {
        realEntryPoints.glGetIntegerv(GL_TEXTURE_BINDING_2D, reinterpret_cast<GLint *>(&savedTextureBinding));
    }

    CHECK_GL_ERROR(realEntryPoints.glBindTexture(GL_TEXTURE_2D, textures[textureIndex]));
    CHECK_GL_ERROR(extensions.glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height));
    CHECK_GL_ERROR(extensions.glActiveTexture(GL_TEXTURE0));

    if (keepBinding) {
        CHECK_GL_ERROR(realEntryPoints.glBindTexture(GL_TEXTURE_2D, savedTextureBinding));
    }

    return true;
}

bool Bamboozler::ScratchTextures::bindTexture(size_t textureIndex) {
    CHECK_GL_ERROR(realEntryPoints.glBindTexture(GL_TEXTURE_2D, textures[textureIndex]));
    return true;
}

// ---------------------------------------------------------- FullscreenVertexBuffer subobject

bool Bamboozler::FullscreenVertexBuffer::setup() {
    if (!initialized) {
        initialized = true;

        CHECK_GL_ERROR(extensions.glGenVertexArrays(1, &vao));
        CHECK_GL_ERROR(extensions.glBindVertexArray(vao));

        float data[] = {
            0, 0, //
            0, 1, //
            1, 1, //

            1, 1, //
            1, 0, //
            0, 0, //
        };
        const GLsizeiptr dataSize = sizeof(data);

        CHECK_GL_ERROR(extensions.glGenBuffers(1, &vbo));
        CHECK_GL_ERROR(extensions.glBindBuffer(GL_ARRAY_BUFFER, vbo));
        CHECK_GL_ERROR(extensions.glBufferData(GL_ARRAY_BUFFER, dataSize, data, GL_STATIC_DRAW));

        GLsizei components = 2;
        CHECK_GL_ERROR(extensions.glVertexAttribPointer(0, components, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * components, nullptr));
    }

    return vbo != 0 && vao != 0;
}

void Bamboozler::FullscreenVertexBuffer::free() {
    extensions.glDeleteBuffers(1, &vbo);
    vbo = {};

    extensions.glDeleteVertexArrays(1, &vao);
    vao = {};

    initialized = {};
}

// ---------------------------------------------------------- Program subobject

bool Bamboozler::Program::setup() {
    if (initialized) {
        return initializedSuccess;
    }

    initialized = true;
    initializedSuccess = true;

    if (!extensions.hasShaders) {
        initializedSuccess = false;
        return false;
    }

    const char *vsSource = R"SOURCE_CODE(
#version 330 core
in vec2 inPosition;
out vec2 texCoord;
void main() {
    gl_Position = vec4(2 * inPosition - 1, 0.0, 1.0);
    texCoord = inPosition;
}
)SOURCE_CODE";

    const char *fsMultiplyColor = R"SOURCE_CODE(
#version 330 core
uniform sampler2D realFrame;
uniform vec4 color;
in vec2 texCoord;
out vec4 outFragColor;
void main() {
    vec4 realColor = texture2D(realFrame, texCoord);
    outFragColor = color * realColor;
}
)SOURCE_CODE";
    multiplyColor.program = createProgram(vsSource, fsMultiplyColor);
    multiplyColor.uniformColor = getUniformLocation(multiplyColor.program, "color");
    initializedSuccess = initializedSuccess && (multiplyColor.program != 0);

    const char *fsSourceInvertColor = R"SOURCE_CODE(
#version 330 core
uniform sampler2D realFrame;
in vec2 texCoord;
out vec4 outFragColor;
void main() {
    vec4 realColor = texture2D(realFrame, texCoord);
    outFragColor.rgb = 1 - realColor.rgb;
}
)SOURCE_CODE";
    invertColor.program = createProgram(vsSource, fsSourceInvertColor);
    initializedSuccess = initializedSuccess && (invertColor.program != 0);

    const char *fsSourceFlip = R"SOURCE_CODE(
#version 330 core
uniform sampler2D realFrame;
uniform bvec2 flipDimensions;
in vec2 texCoord;
out vec4 outFragColor;
void main() {
    vec2 flippedTexCoord = texCoord.xy;
    if (flipDimensions[0]) flippedTexCoord.x = 1 - flippedTexCoord.x;
    if (flipDimensions[1]) flippedTexCoord.y = 1 - flippedTexCoord.y;

    vec4 realColor = texture2D(realFrame, flippedTexCoord);
    outFragColor = realColor;
}
)SOURCE_CODE";
    flip.program = createProgram(vsSource, fsSourceFlip);
    flip.uniformFlipDimensions = getUniformLocation(flip.program, "flipDimensions");
    initializedSuccess = initializedSuccess && (flip.program != 0);

    const char *fsSourceKernelFilter = R"SOURCE_CODE(
#version 330 core
uniform sampler2D realFrame;
uniform int sampleStride;
uniform mat3 kernel;
in vec2 texCoord;
out vec4 outFragColor;

void main()
{
    int s = sampleStride;
    vec3 realColor = texture(realFrame, texCoord).rgb;
    vec3 highPassColor 
                   = kernel[0][0] * textureOffset(realFrame, texCoord, ivec2(-s, -s)).rgb;
    highPassColor += kernel[0][1] * textureOffset(realFrame, texCoord, ivec2( 0, -s)).rgb;
    highPassColor += kernel[0][2] * textureOffset(realFrame, texCoord, ivec2( s, -s)).rgb;
    highPassColor += kernel[1][0] * textureOffset(realFrame, texCoord, ivec2(-s,  0)).rgb;
    highPassColor += kernel[1][1] * realColor;
    highPassColor += kernel[1][2] * textureOffset(realFrame, texCoord, ivec2( s,  0)).rgb;
    highPassColor += kernel[2][0] * textureOffset(realFrame, texCoord, ivec2(-s,  s)).rgb;
    highPassColor += kernel[2][1] * textureOffset(realFrame, texCoord, ivec2( 0,  s)).rgb;
    highPassColor += kernel[2][2] * textureOffset(realFrame, texCoord, ivec2( s,  s)).rgb;

    outFragColor = vec4(highPassColor, 1.0);
}
)SOURCE_CODE";
    kernelFilter.program = createProgram(vsSource, fsSourceKernelFilter);
    kernelFilter.uniformKernel = getUniformLocation(kernelFilter.program, "kernel");
    kernelFilter.uniformSampleStride = getUniformLocation(kernelFilter.program, "sampleStride");
    initializedSuccess = initializedSuccess && (kernelFilter.program != 0);

    const char *fsSourcePassthrough = R"SOURCE_CODE(
#version 330 core
uniform sampler2D realFrame;
in vec2 texCoord;
out vec4 outFragColor;
void main() {
    vec4 realColor = texture2D(realFrame, texCoord);
    outFragColor = realColor;
}
)SOURCE_CODE";
    passthrough.program = createProgram(vsSource, fsSourcePassthrough);
    initializedSuccess = initializedSuccess && (passthrough.program != 0);

    const char *fsSourcePixelation = R"SOURCE_CODE(
#version 330 core
uniform sampler2D realFrame;
uniform int pixelationSize;
in vec2 texCoord;
out vec4 outFragColor;
void main() {
    vec2 realFrameSize = textureSize(realFrame, 0);

    vec2 texCoordPixelated = texCoord * realFrameSize;
    texCoordPixelated = floor(texCoordPixelated / pixelationSize) * pixelationSize;
    texCoordPixelated = texCoordPixelated / realFrameSize;

    //ivec2 a = ivec2(texCoord * realFrameSize + 0.5);
    //a = a / pixelationSize * pixelationSize;
    //texCoordPixelated = (vec2(a) - 0.5) / realFrameSize;

    //vec2 texCoordPixelated = texCoord;
    //texCoordPixelated = texCoordPixelated * realFrameSize;
    //texCoordPixelated = texCoordPixelated + 0.5;
    //texCoordPixelated = texCoordPixelated / pixelationSize;
    //texCoordPixelated = floor(texCoordPixelated);
    //texCoordPixelated = texCoordPixelated * pixelationSize;
    //texCoordPixelated = texCoordPixelated / realFrameSize;

    //vec2 texelSize = 1 / realFrameSize;
   // vec2 texCoordPixelated = texCoord;
    //texCoordPixelated = (texCoordPixelated / texelSize) * texelSize;

    vec4 realColor = texture2D(realFrame, texCoordPixelated);
    outFragColor = realColor;
}
)SOURCE_CODE";
    pixelation.program = createProgram(vsSource, fsSourcePixelation);
    pixelation.uniformSize = getUniformLocation(pixelation.program, "pixelationSize");
    initializedSuccess = initializedSuccess && (pixelation.program != 0);

    return initializedSuccess;
}

void Bamboozler::Program::free() {
    extensions.glDeleteProgram(invertColor.program);
    invertColor = {};

    extensions.glDeleteProgram(flip.program);
    flip = {};

    initialized = {};
}

GLuint Bamboozler::Program::compileShader(const char *source, GLenum shaderType) {
    GLuint shader = extensions.glCreateShader(shaderType);
    CHECK_GL_ERROR("glCreateShader");

    CHECK_GL_ERROR(extensions.glShaderSource(shader, 1, &source, NULL));
    extensions.glCompileShader(shader);

    GLint success{};
    extensions.glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (success == GL_FALSE) {
        GLint logSize{};
        CHECK_GL_ERROR(extensions.glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize));
        auto log = std::make_unique<GLchar[]>(logSize);
        CHECK_GL_ERROR(extensions.glGetShaderInfoLog(shader, logSize, NULL, log.get()));
        printf("Shader compilation error: %s\n", log.get());
        CHECK_GL_ERROR(extensions.glDeleteShader(shader));
        return 0;
    }

    return shader;
}

GLuint Bamboozler::Program::createProgram(const char *vertexShaderSource, const char *fragmentShaderSource) {

    GLuint vs = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    if (vs == 0) {
        return 0;
    }
    GLuint fs = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    if (fs == 0) {
        CHECK_GL_ERROR(extensions.glDeleteShader(vs));
        return 0;
    }

    GLuint program = extensions.glCreateProgram();
    CHECK_GL_ERROR("glCreateProgram");

    CHECK_GL_ERROR(extensions.glAttachShader(program, vs));
    CHECK_GL_ERROR(extensions.glAttachShader(program, fs));
    CHECK_GL_ERROR(extensions.glLinkProgram(program));
    CHECK_GL_ERROR(extensions.glDeleteShader(vs));
    CHECK_GL_ERROR(extensions.glDeleteShader(fs));

    GLint success;
    extensions.glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (success == GL_FALSE) {

        GLint logSize{};
        CHECK_GL_ERROR(extensions.glGetProgramInfoLog(program, 0, &logSize, nullptr));
        auto log = std::make_unique<GLchar[]>(logSize + 1);
        CHECK_GL_ERROR(extensions.glGetProgramInfoLog(program, logSize + 1, nullptr, log.get()));
        printf("Shader compilation error: %s\n", log.get());
    }

    return program;
}

GLint Bamboozler::Program::getUniformLocation(GLuint program, const char *name) {
    GLint location = extensions.glGetUniformLocation(program, name);
    CHECK_GL_ERROR("glGetUniformLocation");
    return location;
}

bool Bamboozler::Program::runMultiplyColor(float *color) {
    CHECK_GL_ERROR(extensions.glUseProgram(multiplyColor.program));
    CHECK_GL_ERROR(extensions.glUniform4fv(multiplyColor.uniformColor, 1, color));
    CHECK_GL_ERROR(realEntryPoints.glDrawArrays(GL_TRIANGLES, 0, 6));
    return true;
}

bool Bamboozler::Program::runInvertColor() {
    CHECK_GL_ERROR(extensions.glUseProgram(invertColor.program));
    CHECK_GL_ERROR(realEntryPoints.glDrawArrays(GL_TRIANGLES, 0, 6));
    return true;
}

bool Bamboozler::Program::runFlip(bool flipX, bool flipY) {
    CHECK_GL_ERROR(extensions.glUseProgram(flip.program));
    CHECK_GL_ERROR(extensions.glUniform2i(flip.uniformFlipDimensions, flipX, flipY));
    CHECK_GL_ERROR(realEntryPoints.glDrawArrays(GL_TRIANGLES, 0, 6));
    return true;
}

bool Bamboozler::Program::runHighPassFilter() {
    CHECK_GL_ERROR(extensions.glUseProgram(kernelFilter.program));
    GLfloat kernelValues[] = {
        -1, -1, -1, //
        -1, 8, -1,  //
        -1, -1, -1, //
    };
    //GLfloat kernelValues[] = {
    //    0, -1, 0, //
    //    -1, 5, -1,  //
    //    0, -1, 0, //
    //};
    CHECK_GL_ERROR(extensions.glUniformMatrix3fv(kernelFilter.uniformKernel, 1, GL_FALSE, kernelValues));
    CHECK_GL_ERROR(extensions.glUniform1i(kernelFilter.uniformSampleStride, 1));
    CHECK_GL_ERROR(realEntryPoints.glDrawArrays(GL_TRIANGLES, 0, 6));
    return true;
}

bool Bamboozler::Program::runBounce(ScratchTextures &scratchTextures, GLint posX, GLint posY, GLsizei width, GLsizei height) {
    // Save framebuffer to a separate texture
    CHECK_BOOLEAN_ERROR(scratchTextures.copyScreenToTexture(1, true));

    // Blur the framebuffer to use as a background
    const int sampleStride = 5; // this should probably be dependent on fb size, but let's just eyeball it.
    const int blurIterations = 3;
    CHECK_GL_ERROR(extensions.glUseProgram(kernelFilter.program));
    GLfloat v = 1.f / 9.f;
    GLfloat kernelValues[] = {
        v, v, v, //
        v, v, v, //
        v, v, v, //
    };
    CHECK_GL_ERROR(extensions.glUniform1i(kernelFilter.uniformSampleStride, sampleStride));
    CHECK_GL_ERROR(extensions.glUniformMatrix3fv(kernelFilter.uniformKernel, 1, GL_FALSE, kernelValues));
    for (int i = 0; i < blurIterations; i++) {
        CHECK_BOOLEAN_ERROR(scratchTextures.copyScreenToTexture());
        CHECK_GL_ERROR(realEntryPoints.glDrawArrays(GL_TRIANGLES, 0, 6));
    }

    // Draw smaller, unblurred framebuffer
    CHECK_BOOLEAN_ERROR(scratchTextures.bindTexture(1)); // bind texture that we previously saved
    CHECK_GL_ERROR(realEntryPoints.glViewport(posX, posY, width, height));
    CHECK_GL_ERROR(extensions.glUseProgram(passthrough.program));
    CHECK_GL_ERROR(realEntryPoints.glDrawArrays(GL_TRIANGLES, 0, 6));
    return true;
}

bool Bamboozler::Program::runPixelation(int size) {
    CHECK_GL_ERROR(extensions.glUseProgram(pixelation.program));
    CHECK_GL_ERROR(extensions.glUniform1i(pixelation.uniformSize, size));
    CHECK_GL_ERROR(realEntryPoints.glDrawArrays(GL_TRIANGLES, 0, 6));
    return true;
}
