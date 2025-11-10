#include <QApplication>
#include <QSurfaceFormat>

#include "OpenGLWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(format);

    OpenGLWindow window;
    window.resize(800, 600);
    window.show();

    return app.exec();
}
