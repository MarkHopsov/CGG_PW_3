#include <QApplication>
#include <QOpenGLContext>
#include <QSurfaceFormat>

#include "OpenGLWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Настройка формата OpenGL для macOS
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(3, 3); // macOS поддерживает OpenGL 3.3+
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSamples(4); // Включение сглаживания

    QSurfaceFormat::setDefaultFormat(format);

    OpenGLWindow window;
    window.resize(800, 600);
    window.setTitle("Практическая работа №3 - Правильный n-угольник");
    window.show();

    return app.exec();
}
