#ifndef OPENGLWINDOW_H
#define OPENGLWINDOW_H

#include <QOpenGLWindow>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QKeyEvent>

class OpenGLWindow : public QOpenGLWindow, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    OpenGLWindow();
    ~OpenGLWindow();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void generatePolygonData(int n, float radius);
    void setupShaders();
    void setupBuffers();

    QOpenGLShaderProgram *m_program;
    QOpenGLBuffer m_vbo;
    QOpenGLBuffer m_ebo;
    GLuint m_vao;

    // Данные вершин и индексов
    QVector<float> m_vertices;
    QVector<unsigned int> m_indices;

    int m_n; // Количество сторон многоугольника
    float m_radius; // Радиус многоугольника

    // Атрибуты шейдера
    GLint m_posAttr;
    GLint m_colAttr;
    GLint m_matrixUniform;
};

#endif // OPENGLWINDOW_H
