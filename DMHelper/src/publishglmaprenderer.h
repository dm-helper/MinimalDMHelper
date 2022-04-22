#ifndef PUBLISHGLMAPRENDERER_H
#define PUBLISHGLMAPRENDERER_H

#include "publishglrenderer.h"
#include <QColor>
#include <QImage>
#include <QMatrix4x4>

class QOpenGLFunctions;

class PublishGLMapRenderer : public PublishGLRenderer
{
    Q_OBJECT
public:
    PublishGLMapRenderer(QObject *parent = nullptr);
    virtual ~PublishGLMapRenderer() override;

    virtual QColor getBackgroundColor() override;

    // DMH OpenGL renderer calls
    virtual void cleanup() override;
    virtual bool deleteOnDeactivation() override;

    virtual void setBackgroundColor(const QColor& color) override;

    // Standard OpenGL calls
    virtual void initializeGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;

protected:
    // DMH OpenGL renderer calls
    virtual void updateProjectionMatrix() override;

    // Background overrides
    virtual void initializeBackground() = 0;
    virtual bool isBackgroundReady() = 0;
    virtual void resizeBackground(int w, int h) = 0;
    virtual void paintBackground(QOpenGLFunctions* functions) = 0;
    virtual QSizeF getBackgroundSize() = 0;
    virtual void updateBackground();

    virtual void updateContents();

protected:
    QSize _targetSize;
    QColor _color;
    QMatrix4x4 _projectionMatrix;
    QRectF _cameraRect;
    bool _initialized;
    unsigned int _shaderProgram;
    int _shaderModelMatrix;
};

#endif // PUBLISHGLMAPRENDERER_H
