#include "publishglmaprenderer.h"
#include "videoplayerglplayer.h"
#include <QOpenGLWidget>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QPainter>
#include <QPainterPath>
#include <QUndoStack>
#include <QDebug>

PublishGLMapRenderer::PublishGLMapRenderer(QObject *parent) :
    PublishGLRenderer(parent),
    _targetSize(),
    _color(),
    _projectionMatrix(),
    _cameraRect(),
    _initialized(false),
    _shaderProgram(0),
    _shaderModelMatrix(0)
{
}

PublishGLMapRenderer::~PublishGLMapRenderer()
{
}

QColor PublishGLMapRenderer::getBackgroundColor()
{
    return _color;
}

void PublishGLMapRenderer::cleanup()
{
    _initialized = false;

    _projectionMatrix.setToIdentity();

    if(_shaderProgram > 0)
    {
        if((_targetWidget) && (_targetWidget->context()))
        {
            QOpenGLFunctions *f = _targetWidget->context()->functions();
            if(f)
                f->glDeleteProgram(_shaderProgram);
        }
        _shaderProgram = 0;
    }
    _shaderModelMatrix = 0;

    PublishGLRenderer::cleanup();
}

bool PublishGLMapRenderer::deleteOnDeactivation()
{
    return true;
}

void PublishGLMapRenderer::setBackgroundColor(const QColor& color)
{
    _color = color;
    emit updateWidget();
}

void PublishGLMapRenderer::initializeGL()
{    
    if((_initialized) || (!_targetWidget))
        return;

    // Set up the rendering context, load shaders and other resources, etc.:
    QOpenGLFunctions *f = _targetWidget->context()->functions();
    if(!f)
        return;

    qDebug() << "[PublishGLMapRenderer] Initializing renderer";

    const char *vertexShaderSource = "#version 410 core\n"
        "layout (location = 0) in vec3 aPos;   // the position variable has attribute position 0\n"
        "layout (location = 1) in vec3 aColor; // the color variable has attribute position 1\n"
        "layout (location = 2) in vec2 aTexCoord;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "out vec3 ourColor; // output a color to the fragment shader\n"
        "out vec2 TexCoord;\n"
        "void main()\n"
        "{\n"
        "   // note that we read the multiplication from right to left\n"
        "   gl_Position = projection * view * model * vec4(aPos, 1.0); // gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "   ourColor = aColor; // set ourColor to the input color we got from the vertex data\n"
        "   TexCoord = aTexCoord;\n"
        "}\0";

    unsigned int vertexShader;
    vertexShader = f->glCreateShader(GL_VERTEX_SHADER);
    f->glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    f->glCompileShader(vertexShader);

    int  success;
    char infoLog[512];
    f->glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        qDebug() << "[PublishGLMapRenderer] ERROR::SHADER::VERTEX::COMPILATION_FAILED: " << infoLog;
        return;
    }

    const char *fragmentShaderSource = "#version 410 core\n"
        "out vec4 FragColor;\n"
        "in vec3 ourColor;\n"
        "in vec2 TexCoord;\n"
        "uniform sampler2D texture1;\n"
        "void main()\n"
        "{\n"
        "    FragColor = texture(texture1, TexCoord); // FragColor = vec4(ourColor, 1.0f);\n"
        "}\0";

    unsigned int fragmentShader;
    fragmentShader = f->glCreateShader(GL_FRAGMENT_SHADER);
    f->glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    f->glCompileShader(fragmentShader);

    f->glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        qDebug() << "[PublishGLMapRenderer] ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: " << infoLog;
        return;
    }

    _shaderProgram = f->glCreateProgram();

    f->glAttachShader(_shaderProgram, vertexShader);
    f->glAttachShader(_shaderProgram, fragmentShader);
    f->glLinkProgram(_shaderProgram);

    f->glGetProgramiv(_shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        f->glGetProgramInfoLog(_shaderProgram, 512, NULL, infoLog);
        qDebug() << "[PublishGLMapRenderer] ERROR::SHADER::PROGRAM::COMPILATION_FAILED: " << infoLog;
        return;
    }

    f->glUseProgram(_shaderProgram);
    f->glDeleteShader(vertexShader);
    f->glDeleteShader(fragmentShader);
    _shaderModelMatrix = f->glGetUniformLocation(_shaderProgram, "model");

    f->glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture

    // Create the objects
    initializeBackground();

    // Matrices
    // Model
    QMatrix4x4 modelMatrix;
    f->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, modelMatrix.constData());
    // View
    QMatrix4x4 viewMatrix;
    viewMatrix.lookAt(QVector3D(0.f, 0.f, 500.f), QVector3D(0.f, 0.f, 0.f), QVector3D(0.f, 1.f, 0.f));
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "view"), 1, GL_FALSE, viewMatrix.constData());
    // Projection - note, this is set later when resizing the window
    updateProjectionMatrix();

    _initialized = true;
}

void PublishGLMapRenderer::resizeGL(int w, int h)
{
    _targetSize = QSize(w, h);
    qDebug() << "[PublishGLMapRenderer] Resize w: " << w << ", h: " << h;
    resizeBackground(w, h);

    emit updateWidget();
}

void PublishGLMapRenderer::paintGL()
{
    if((!_initialized) || (!_targetSize.isValid()) || (!_targetWidget) || (!_targetWidget->context()))
        return;

    if(!isBackgroundReady())
    {
        updateBackground();
        if(!isBackgroundReady())
            return;
    }

    updateProjectionMatrix();

    QOpenGLFunctions *f = _targetWidget->context()->functions();
    QOpenGLExtraFunctions *e = _targetWidget->context()->extraFunctions();
    if((!f) || (!e))
        return;

    // Draw the scene
    f->glClearColor(_color.redF(), _color.greenF(), _color.blueF(), 1.0f);
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    f->glUseProgram(_shaderProgram);
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "projection"), 1, GL_FALSE, _projectionMatrix.constData());

    paintBackground(f);
}

void PublishGLMapRenderer::updateProjectionMatrix()
{
    if((_shaderProgram == 0) || (!_targetSize.isValid()) || (!_targetWidget) || (!_targetWidget->context()))
        return;

    QOpenGLFunctions *f = _targetWidget->context()->functions();
    if(!f)
        return;

    // Hack the camera
    _cameraRect.setSize(getBackgroundSize());

    // Update projection matrix and other size related settings:
    QSizeF rectSize = QSizeF(_targetSize).scaled(_cameraRect.size(), Qt::KeepAspectRatioByExpanding);
    QSizeF halfRect = rectSize / 2.0;
    QPointF cameraTopLeft((rectSize.width() - _cameraRect.width()) / 2.0, (rectSize.height() - _cameraRect.height()) / 2);
    QPointF cameraMiddle(_cameraRect.x() + (_cameraRect.width() / 2.0), _cameraRect.y() + (_cameraRect.height() / 2.0));
    QSizeF backgroundMiddle = getBackgroundSize() / 2.0;

    _projectionMatrix.setToIdentity();
    _projectionMatrix.rotate(0.0, 0.0, 0.0, -1.0);
    _projectionMatrix.ortho(cameraMiddle.x() - backgroundMiddle.width() - halfRect.width(), cameraMiddle.x() - backgroundMiddle.width() + halfRect.width(),
                            backgroundMiddle.height() - cameraMiddle.y() - halfRect.height(), backgroundMiddle.height() - cameraMiddle.y() + halfRect.height(),
                            -1000.f, 1000.f);
}

void PublishGLMapRenderer::updateBackground()
{
}

void PublishGLMapRenderer::updateContents()
{

}
