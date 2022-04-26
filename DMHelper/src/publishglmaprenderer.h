#ifndef PUBLISHGLMAPRENDERER_H
#define PUBLISHGLMAPRENDERER_H

#include <QObject>
#include <QColor>
#include <QImage>
#include <QMatrix4x4>

class VideoPlayerGLPlayer;
class QOpenGLWidget;
class QOpenGLFunctions;

class PublishGLMapRenderer : public QObject
{
    Q_OBJECT
public:
    PublishGLMapRenderer(QObject *parent = nullptr);
    virtual ~PublishGLMapRenderer() override;

    virtual QColor getBackgroundColor();

    // DMH OpenGL renderer calls
    virtual void rendererActivated(QOpenGLWidget* glWidget);
    virtual void rendererDeactivated();
    virtual void cleanup();
    virtual bool deleteOnDeactivation();

    virtual void setBackgroundColor(const QColor& color);

    // Standard OpenGL calls
    virtual void initializeGL();
    virtual void resizeGL(int w, int h);
    virtual void paintGL();

signals:
    void initializationComplete();
    void updateWidget();
    void deactivated();

protected:
    // DMH OpenGL renderer calls
    virtual void updateProjectionMatrix();

    // Background overrides
    virtual void initializeBackground();
    virtual void resizeBackground(int w, int h);
    virtual void paintBackground(QOpenGLFunctions* functions);
    virtual QSizeF getBackgroundSize();

protected:
    QOpenGLWidget* _targetWidget;
    QSize _targetSize;
    QColor _color;
    QMatrix4x4 _projectionMatrix;
    QRectF _cameraRect;
    bool _initialized;
    unsigned int _shaderProgram;
    int _shaderModelMatrix;
    VideoPlayerGLPlayer* _videoPlayer;

};

#endif // PUBLISHGLMAPRENDERER_H
