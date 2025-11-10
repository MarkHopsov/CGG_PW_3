#include "OpenGLWindow.h"
#include <QDebug>
#include <cmath>
#include <QKeyEvent>

OpenGLWindow::OpenGLWindow()
    : m_vao_polygon(0), m_vbo_polygon(0)
    , m_ebo_triangles(0), m_ebo_all_lines(0), m_ebo_contour(0), m_ebo_even_points(0)
    , m_vao_triangle(0), m_vao_rectangle(0), m_vao_line(0)
    , m_vbo_triangle(0), m_vbo_rectangle(0), m_vbo_line(0)
    , m_vao_butterfly(0), m_vbo_butterfly(0)
    , m_shaderProgram(0)
    , m_n(6), m_radius(0.8f), m_polygonMode(1)
    , m_showTransformed(false)
    , m_currentTask(1)
{
}

OpenGLWindow::~OpenGLWindow()
{
    makeCurrent();
    // Освобождение буферов задания 1
    if (m_vao_polygon) glDeleteVertexArrays(1, &m_vao_polygon);
    if (m_vbo_polygon) glDeleteBuffers(1, &m_vbo_polygon);
    if (m_ebo_triangles) glDeleteBuffers(1, &m_ebo_triangles);
    if (m_ebo_all_lines) glDeleteBuffers(1, &m_ebo_all_lines);
    if (m_ebo_contour) glDeleteBuffers(1, &m_ebo_contour);
    if (m_ebo_even_points) glDeleteBuffers(1, &m_ebo_even_points);

    // Освобождение буферов задания 2
    if (m_vao_triangle) glDeleteVertexArrays(1, &m_vao_triangle);
    if (m_vao_rectangle) glDeleteVertexArrays(1, &m_vao_rectangle);
    if (m_vao_line) glDeleteVertexArrays(1, &m_vao_line);
    if (m_vbo_triangle) glDeleteBuffers(1, &m_vbo_triangle);
    if (m_vbo_rectangle) glDeleteBuffers(1, &m_vbo_rectangle);
    if (m_vbo_line) glDeleteBuffers(1, &m_vbo_line);

    // Освобождение буферов задания 3
    if (m_vao_butterfly) glDeleteVertexArrays(1, &m_vao_butterfly);
    if (m_vbo_butterfly) glDeleteBuffers(1, &m_vbo_butterfly);
    if (m_ebo_butterfly) glDeleteBuffers(1, &m_ebo_butterfly);

    if (m_shaderProgram) glDeleteProgram(m_shaderProgram);
}

void OpenGLWindow::updateWindowTitle()
{
    QString title = "Практическая работа №3 - ";

    switch (m_currentTask) {
    case 1:
        title += "Задание №1. ";
        switch (m_polygonMode) {
        case 1: title += "Пункт а"; break;
        case 2: title += "Пункт б"; break;
        case 3: title += "Пункт в"; break;
        case 4: title += "Пункт г"; break;
        }
        break;
    case 2:
        title += m_showTransformed ? "Задание №2. Преобразованные фигуры" : "Задание №2. Исходные фигуры";
        break;
    case 3:
        title += "Задание №3";
        break;
    }

    setTitle(title);
}

void OpenGLWindow::generatePolygonVertices()
{
    m_vertices.clear();
    m_triangleIndices.clear();
    m_allLinesIndices.clear();
    m_contourIndices.clear();
    m_evenPointsIndices.clear();

    // Генерация вершин n-угольника
    for (int i = 0; i < m_n; ++i) {
        float angle = 2.0f * M_PI * i / m_n;
        float x = m_radius * cos(angle);
        float y = m_radius * sin(angle);
        m_vertices.push_back(x);
        m_vertices.push_back(y);
        m_vertices.push_back(0.0f);
    }

    // а) Непересекающиеся треугольники (от центра к вершинам)
    m_vertices.push_back(0.0f);
    m_vertices.push_back(0.0f);
    m_vertices.push_back(0.0f);
    int centerIndex = m_n;

    for (int i = 0; i < m_n; ++i) {
        m_triangleIndices.push_back(centerIndex);
        m_triangleIndices.push_back(i);
        m_triangleIndices.push_back((i + 1) % m_n);
    }

    // б) Все возможные пересекающиеся линии
    for (int i = 0; i < m_n; ++i) {
        for (int j = i + 1; j < m_n; ++j) {
            m_allLinesIndices.push_back(i);
            m_allLinesIndices.push_back(j);
        }
    }

    // в) Контур n-угольника
    for (int i = 0; i < m_n; ++i) {
        m_contourIndices.push_back(i);
    }
    m_contourIndices.push_back(0);

    // г) Линия через чётные точки
    for (int i = 0; i < m_n; i += 2) {
        m_evenPointsIndices.push_back(i);
    }
}

void OpenGLWindow::setupShaders()
{
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        uniform mat4 transform;
        uniform vec3 objectColor;
        out vec3 ourColor;
        void main()
        {
            gl_Position = transform * vec4(aPos, 1.0);
            ourColor = objectColor;
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        in vec3 ourColor;
        void main()
        {
            FragColor = vec4(ourColor, 1.0);
        }
    )";

    // Компиляция шейдеров
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        qDebug() << "Vertex shader compilation failed:" << infoLog;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        qDebug() << "Fragment shader compilation failed:" << infoLog;
    }

    // Линковка шейдеров
    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vertexShader);
    glAttachShader(m_shaderProgram, fragmentShader);
    glLinkProgram(m_shaderProgram);

    glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(m_shaderProgram, 512, NULL, infoLog);
        qDebug() << "Shader program linking failed:" << infoLog;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void OpenGLWindow::setupPolygonBuffers()
{
    generatePolygonVertices();

    // Создание VAO и VBO
    glGenVertexArrays(1, &m_vao_polygon);
    glGenBuffers(1, &m_vbo_polygon);

    glBindVertexArray(m_vao_polygon);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_polygon);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(float), m_vertices.data(), GL_STATIC_DRAW);

    // Настройка атрибутов вершин
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Создание EBO для разных режимов
    glGenBuffers(1, &m_ebo_triangles);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo_triangles);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_triangleIndices.size() * sizeof(unsigned int),
                 m_triangleIndices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &m_ebo_all_lines);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo_all_lines);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_allLinesIndices.size() * sizeof(unsigned int),
                 m_allLinesIndices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &m_ebo_contour);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo_contour);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_contourIndices.size() * sizeof(unsigned int),
                 m_contourIndices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &m_ebo_even_points);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo_even_points);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_evenPointsIndices.size() * sizeof(unsigned int),
                 m_evenPointsIndices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void OpenGLWindow::setupGeometryTransformations()
{
    // Вершины треугольника
    float triangleVertices[] = {
        -0.1f, -0.1f, 0.0f,
        0.1f, -0.1f, 0.0f,
        0.0f,  0.1f, 0.0f
    };

    // Вершины прямоугольника
    float rectangleVertices[] = {
        -0.08f, -0.05f, 0.0f,
        0.08f, -0.05f, 0.0f,
        0.08f,  0.05f, 0.0f,
        -0.08f,  0.05f, 0.0f
    };
    unsigned int rectangleIndices[] = {
        0, 1, 2,
        2, 3, 0
    };

    // Вершины линии
    float lineVertices[] = {
        -0.15f, 0.0f, 0.0f,
        0.15f, 0.0f, 0.0f
    };

    // Создание VAO и VBO для треугольника
    glGenVertexArrays(1, &m_vao_triangle);
    glGenBuffers(1, &m_vbo_triangle);

    glBindVertexArray(m_vao_triangle);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_triangle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Создание VAO и VBO для прямоугольника
    glGenVertexArrays(1, &m_vao_rectangle);
    glGenBuffers(1, &m_vbo_rectangle);
    GLuint ebo_rectangle;
    glGenBuffers(1, &ebo_rectangle);

    glBindVertexArray(m_vao_rectangle);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_rectangle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), rectangleVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_rectangle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rectangleIndices), rectangleIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Создание VAO и VBO для линии
    glGenVertexArrays(1, &m_vao_line);
    glGenBuffers(1, &m_vbo_line);

    glBindVertexArray(m_vao_line);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_line);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void OpenGLWindow::setupTransformations()
{
    // Матрица преобразования для треугольника: масштабирование + перенос
    m_transform_triangle = glm::mat4(1.0f);
    m_transform_triangle = glm::translate(m_transform_triangle, glm::vec3(0.3f, -0.2f, 0.0f));
    m_transform_triangle = glm::scale(m_transform_triangle, glm::vec3(-0.5f, 1.5f, 1.0f));

    // Матрица преобразования для линии: поворот на 45°
    m_transform_line = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    // Матрица преобразования для прямоугольника: поворот относительно точки (0.2, 0.2) на -30°
    m_transform_rectangle = glm::mat4(1.0f);
    m_transform_rectangle = glm::translate(m_transform_rectangle, glm::vec3(0.2f, 0.2f, 0.0f));
    m_transform_rectangle = glm::rotate(m_transform_rectangle, glm::radians(-30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    m_transform_rectangle = glm::translate(m_transform_rectangle, glm::vec3(-0.2f, -0.2f, 0.0f));
}

void OpenGLWindow::generateButterflyWithTransformations()
{
    m_butterflyVertices.clear();
    m_butterflyIndices.clear();

    // Базовый треугольник - левый верхний
    std::vector<glm::vec3> baseTriangle = {
        {0.0f, 0.0f, 0.0f},
        {-0.8f, 0.0f, 0.0f},
        {-0.8f, 0.4f, 0.0f}
    };

    // 1. Исходный левый верхний треугольник
    for (const auto& point : baseTriangle) {
        m_butterflyVertices.push_back(point.x);
        m_butterflyVertices.push_back(point.y);
        m_butterflyVertices.push_back(point.z);
    }

    // 2. Правый верхний треугольник - отражение левого верхнего по оси Y
    glm::mat4 mirrorY = glm::scale(glm::mat4(1.0f), glm::vec3(-1.0f, 1.0f, 1.0f));
    for (const auto& point : baseTriangle) {
        glm::vec4 transformed = mirrorY * glm::vec4(point, 1.0f);
        m_butterflyVertices.push_back(transformed.x);
        m_butterflyVertices.push_back(transformed.y);
        m_butterflyVertices.push_back(transformed.z);
    }

    // 3. Левый нижний треугольник из левого верхнего
    glm::mat4 lowerTransform = glm::mat4(1.0f);
    lowerTransform = glm::scale(lowerTransform, glm::vec3(1.0f, -1.0f, 1.0f));
    lowerTransform = glm::scale(lowerTransform, glm::vec3(0.7f, 0.7f, 1.0f));
    lowerTransform = glm::rotate(lowerTransform, glm::radians(-40.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    for (const auto& point : baseTriangle) {
        glm::vec4 transformed = lowerTransform * glm::vec4(point, 1.0f);
        m_butterflyVertices.push_back(transformed.x);
        m_butterflyVertices.push_back(transformed.y);
        m_butterflyVertices.push_back(transformed.z);
    }

    // 4. Правый нижний треугольник - отражение левого нижнего по оси Y
    for (const auto& point : baseTriangle) {
        glm::vec4 transformed = mirrorY * lowerTransform * glm::vec4(point, 1.0f);
        m_butterflyVertices.push_back(transformed.x);
        m_butterflyVertices.push_back(transformed.y);
        m_butterflyVertices.push_back(transformed.z);
    }

    // Создаем индексы для рисования контуров треугольников
    for (int triangle = 0; triangle < 4; ++triangle) {
        int baseIndex = triangle * 3;
        m_butterflyIndices.push_back(baseIndex);
        m_butterflyIndices.push_back(baseIndex + 1);
        m_butterflyIndices.push_back(baseIndex + 2);
        m_butterflyIndices.push_back(baseIndex);
    }
}

void OpenGLWindow::setupButterflyBuffers()
{
    generateButterflyWithTransformations();

    glGenVertexArrays(1, &m_vao_butterfly);
    glGenBuffers(1, &m_vbo_butterfly);
    glGenBuffers(1, &m_ebo_butterfly);

    glBindVertexArray(m_vao_butterfly);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_butterfly);
    glBufferData(GL_ARRAY_BUFFER, m_butterflyVertices.size() * sizeof(float),
                 m_butterflyVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo_butterfly);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_butterflyIndices.size() * sizeof(unsigned int),
                 m_butterflyIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void OpenGLWindow::initializeGL()
{
    initializeOpenGLFunctions();

    setupShaders();
    setupPolygonBuffers();
    setupGeometryTransformations();
    setupTransformations();
    setupButterflyBuffers();

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glLineWidth(2.0f);

    updateWindowTitle();
}

void OpenGLWindow::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(m_shaderProgram);

    GLint transformLocation = glGetUniformLocation(m_shaderProgram, "transform");
    GLint colorLocation = glGetUniformLocation(m_shaderProgram, "objectColor");

    glm::mat4 identity = glm::mat4(1.0f);

    if (m_currentTask == 1) {
        // === ЗАДАНИЕ 1: n-угольник ===
        glBindVertexArray(m_vao_polygon);

        switch (m_polygonMode) {
        case 1: // Непересекающиеся треугольники - красный
            glUniform3f(colorLocation, 1.0f, 0.0f, 0.0f);
            glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(identity));
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo_triangles);
            glDrawElements(GL_LINES, m_triangleIndices.size(), GL_UNSIGNED_INT, 0);
            break;

        case 2: // Все возможные линии - зеленый
            glUniform3f(colorLocation, 0.0f, 1.0f, 0.0f);
            glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(identity));
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo_all_lines);
            glDrawElements(GL_LINES, m_allLinesIndices.size(), GL_UNSIGNED_INT, 0);
            break;

        case 3: // Контур - синий
            glUniform3f(colorLocation, 0.0f, 0.0f, 1.0f);
            glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(identity));
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo_contour);
            glDrawElements(GL_LINE_LOOP, m_contourIndices.size() - 1, GL_UNSIGNED_INT, 0);
            break;

        case 4: // Четные точки - желтый
            glUniform3f(colorLocation, 1.0f, 1.0f, 0.0f);
            glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(identity));
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo_even_points);
            glDrawElements(GL_LINE_STRIP, m_evenPointsIndices.size(), GL_UNSIGNED_INT, 0);
            break;
        }
    } else if (m_currentTask == 2) {
        // === ЗАДАНИЕ 2: преобразования ===

        // Рисуем треугольник
        glBindVertexArray(m_vao_triangle);
        if (m_showTransformed) {
            glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(m_transform_triangle));
            glUniform3f(colorLocation, 1.0f, 0.0f, 0.0f);
        } else {
            glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(identity));
            glUniform3f(colorLocation, 0.8f, 0.2f, 0.2f);
        }
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Рисуем прямоугольник
        glBindVertexArray(m_vao_rectangle);
        if (m_showTransformed) {
            glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(m_transform_rectangle));
            glUniform3f(colorLocation, 0.0f, 1.0f, 0.0f);
        } else {
            glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(identity));
            glUniform3f(colorLocation, 0.2f, 0.8f, 0.2f);
        }
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Рисуем линию
        glBindVertexArray(m_vao_line);
        if (m_showTransformed) {
            glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(m_transform_line));
            glUniform3f(colorLocation, 0.0f, 0.0f, 1.0f);
        } else {
            glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(identity));
            glUniform3f(colorLocation, 0.2f, 0.2f, 0.8f);
        }
        glDrawArrays(GL_LINES, 0, 2);
    } else if (m_currentTask == 3) {
        // === ЗАДАНИЕ 3: бабочка ===
        glBindVertexArray(m_vao_butterfly);
        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(identity));

        for (int triangle = 0; triangle < 4; ++triangle) {
            switch (triangle) {
            case 0: // Левый верхний - красный
                glUniform3f(colorLocation, 1.0f, 0.0f, 0.0f);
                break;
            case 1: // Правый верхний - зеленый
                glUniform3f(colorLocation, 0.0f, 1.0f, 0.0f);
                break;
            case 2: // Левый нижний - синий
                glUniform3f(colorLocation, 0.0f, 0.0f, 1.0f);
                break;
            case 3: // Правый нижний - желтый
                glUniform3f(colorLocation, 1.0f, 1.0f, 0.0f);
                break;
            }

            glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, (void*)(triangle * 4 * sizeof(unsigned int)));
        }
    }

    // Проверка ошибок
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        qDebug() << "OpenGL error:" << error;
    }
}

void OpenGLWindow::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void OpenGLWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
    } else if (event->key() == Qt::Key_Q) {
        m_currentTask = 1;
        updateWindowTitle();
        update();
    } else if (event->key() == Qt::Key_W) {
        m_currentTask = 2;
        updateWindowTitle();
        update();
    } else if (event->key() == Qt::Key_E) {
        m_currentTask = 3;
        updateWindowTitle();
        update();
    } else if (m_currentTask == 1) {
        // Управление для задания 1
        if (event->key() >= Qt::Key_1 && event->key() <= Qt::Key_4) {
            m_polygonMode = event->key() - Qt::Key_0;
            updateWindowTitle();
            update();
        }
    } else if (m_currentTask == 2) {
        // Управление для задания 2
        if (event->key() == Qt::Key_Space) {
            m_showTransformed = !m_showTransformed;
            updateWindowTitle();
            update();
        }
    }
}
