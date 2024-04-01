#include "test_app.h"

void renderBeginEnd(float t, int width, int height) {
    glViewport(0, 0, width, height);
    glClearColor(0.4, 0.1, 0.4, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

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

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    TestApp app = initializeTestApp(renderBeginEnd);
    runTestApp(app);
    cleanupTestApp(app);
    return 0;
}
