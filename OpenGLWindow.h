#ifndef OPENGLWINDOW_H
#define OPENGLWINDOW_H

#include <QOpenGLWindow>
#include <QOpenGLFunctions_3_3_Core>
#include <vector>

// Подключаем GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
    void updateWindowTitle();

    // Методы для задания 1 (n-угольник)
    void generatePolygonVertices();
    void setupShaders();
    void setupPolygonBuffers();

    // Методы для задания 2 (преобразования)
    void setupGeometryTransformations();
    void setupTransformations();

    // Методы для задания 3 (бабочка)
    void generateButterflyWithTransformations();
    void setupButterflyBuffers();

    // Буферы для задания 1
    GLuint m_vao_polygon;
    GLuint m_vbo_polygon;
    GLuint m_ebo_triangles, m_ebo_all_lines, m_ebo_contour, m_ebo_even_points;

    // Буферы для задания 2
    GLuint m_vao_triangle, m_vao_rectangle, m_vao_line;
    GLuint m_vbo_triangle, m_vbo_rectangle, m_vbo_line;

    // Буферы для задания 3
    GLuint m_vao_butterfly;
    GLuint m_vbo_butterfly;
    GLuint m_ebo_butterfly;

    GLuint m_shaderProgram;

    // Данные для задания 1
    int m_n;
    float m_radius;
    int m_polygonMode;
    std::vector<float> m_vertices;
    std::vector<unsigned int> m_triangleIndices;
    std::vector<unsigned int> m_allLinesIndices;
    std::vector<unsigned int> m_contourIndices;
    std::vector<unsigned int> m_evenPointsIndices;

    // Данные для задания 2
    glm::mat4 m_transform_triangle;
    glm::mat4 m_transform_line;
    glm::mat4 m_transform_rectangle;
    bool m_showTransformed;

    // Данные для задания 3
    std::vector<float> m_butterflyVertices;
    std::vector<unsigned int> m_butterflyIndices;

    // Текущее задание (1, 2 или 3)
    int m_currentTask;
};

#endif // OPENGLWINDOW_H
