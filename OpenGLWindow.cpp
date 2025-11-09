#include "OpenGLWindow.h"
#include <QDebug>
#include <QVector3D>
#include <QMatrix4x4>
#include <cmath>

OpenGLWindow::OpenGLWindow()
    : m_program(nullptr)
    , m_n(5) // Пятиугольник по умолчанию
    , m_radius(0.8f)
{
}

OpenGLWindow::~OpenGLWindow()
{
    makeCurrent();
    m_vbo.destroy();
    m_ebo.destroy();
    glDeleteVertexArrays(1, &m_vao);
    delete m_program;
}

void OpenGLWindow::initializeGL()
{
    initializeOpenGLFunctions();

    // Настройка OpenGL
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    // Генерация данных для многоугольника
    generatePolygonData(m_n, m_radius);

    // Настройка шейдеров и буферов
    setupShaders();
    setupBuffers();
}

void OpenGLWindow::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_program->bind();

    // Матрица проекции (ортографическая)
    QMatrix4x4 matrix;
    matrix.ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    m_program->setUniformValue(m_matrixUniform, matrix);

    // Рисуем треугольники (непересекающиеся)
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);

    m_program->release();
}

void OpenGLWindow::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void OpenGLWindow::keyPressEvent(QKeyEvent *event)
{
    // Изменение количества сторон по нажатию клавиш
    if (event->key() == Qt::Key_Plus && m_n < 20) {
        m_n++;
        generatePolygonData(m_n, m_radius);
        setupBuffers();
        update();
    } else if (event->key() == Qt::Key_Minus && m_n > 3) {
        m_n--;
        generatePolygonData(m_n, m_radius);
        setupBuffers();
        update();
    } else if (event->key() == Qt::Key_Escape) {
        close();
    }
}

void OpenGLWindow::generatePolygonData(int n, float radius)
{
    m_vertices.clear();
    m_indices.clear();

    // Центральная точка
    m_vertices << 0.0f << 0.0f << 0.0f; // позиция
    m_vertices << 1.0f << 1.0f << 1.0f; // цвет (белый)

    // Генерация вершин многоугольника
    for (int i = 0; i < n; ++i) {
        float angle = 2.0f * M_PI * i / n;
        float x = radius * cos(angle);
        float y = radius * sin(angle);

        // Позиция вершины
        m_vertices << x << y << 0.0f;

        // Цвет вершины (чередование цветов)
        float r = static_cast<float>(i % 3 == 0);
        float g = static_cast<float>(i % 3 == 1);
        float b = static_cast<float>(i % 3 == 2);
        m_vertices << r << g << b;
    }

    // Генерация индексов для треугольников (непересекающиеся)
    for (int i = 1; i <= n; ++i) {
        m_indices << 0; // центр
        m_indices << i;
        m_indices << (i % n) + 1;
    }
}

void OpenGLWindow::setupShaders()
{
    m_program = new QOpenGLShaderProgram();

    // Загрузка шейдеров из файлов
    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "shaders/vertexshader.glsl");
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "shaders/fragmentshader.glsl");

    if (!m_program->link()) {
        qDebug() << "Shader program linking error:" << m_program->log();
    }

    // Получение location атрибутов и uniform-переменных
    m_posAttr = m_program->attributeLocation("position");
    m_colAttr = m_program->attributeLocation("color");
    m_matrixUniform = m_program->uniformLocation("matrix");
}

void OpenGLWindow::setupBuffers()
{
    // Создание и настройка VAO
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    // Настройка VBO
    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(m_vertices.constData(), m_vertices.size() * sizeof(float));

    // Настройка EBO
    m_ebo.create();
    m_ebo.bind();
    m_ebo.allocate(m_indices.constData(), m_indices.size() * sizeof(unsigned int));

    // Настройка атрибутов вершин
    m_program->enableAttributeArray(m_posAttr);
    m_program->setAttributeBuffer(m_posAttr, GL_FLOAT, 0, 3, 6 * sizeof(float));

    m_program->enableAttributeArray(m_colAttr);
    m_program->setAttributeBuffer(m_colAttr, GL_FLOAT, 3 * sizeof(float), 3, 6 * sizeof(float));

    // Отвязывание буферов
    glBindVertexArray(0);
    m_vbo.release();
    m_ebo.release();
}
