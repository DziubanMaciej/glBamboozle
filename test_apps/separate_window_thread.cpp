#include "test_app.h"

#include <atomic>
#include <thread>

std::atomic_int sharedCounter = 0;

TestApp *app = nullptr;
std::atomic<float> t = 0;
std::atomic<int> width = 0;
std::atomic<int> height = 0;

void renderBeginEnd(float t, int width, int height) {
    wglMakeCurrent(app->hdc, app->hglrc);
    auto a = glGetError();
    glViewport(0, 0, width, height);
    glClearColor(0.4, 0.1, 0.4, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    auto b = glGetError();
    glBegin(GL_TRIANGLES);


    const int triangleCount = 7;
    for (int i = 0; i < triangleCount; i++) {
        float angle = i / static_cast<float>(triangleCount) + t; // <0,1>
        angle *= 2 * 3.141592;                                   // <0, 2PI>
        angle += 3.141592 / 2;                                   // move by PI/2, so the first triangle is at the top

        float x = cosf(angle) / 3;
        float y = sinf(angle) / 3;

        if (i == 0) {
            glColor3f(1.0f, 0.0f, 0.0f);
        } else {
            glColor3f(1.0f, 1.0f, 0.0f);
        }
        glVertex2f(x + 0.1, y);
        glVertex2f(x - 0.1, y);
        glVertex2f(x, y + 0.2);
    }
    glEnd();

    glFlush();
}

void openglThreadFunction() {
    while (true) {
        while (sharedCounter.load() % 2 == 0)
            ;

        renderBeginEnd(t, width, height);

        sharedCounter++;
    }
}

void wakeUpOpenGlThread(float t, int width, int height) {
    while (sharedCounter.load() % 2 == 1)
        ;

    ::t = t;
    ::width = width;
    ::height = height;
    sharedCounter++;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    std::thread openglThread{openglThreadFunction};

    TestApp app = initializeTestApp(wakeUpOpenGlThread);
    ::app = &app;
    runTestApp(app);
    cleanupTestApp(app);
    return 0;
}
