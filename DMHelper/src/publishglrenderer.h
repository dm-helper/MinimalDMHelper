#ifndef PUBLISHGLRENDERER_H
#define PUBLISHGLRENDERER_H

#include <QObject>
#include <QPointF>

class CampaignObjectBase;
class PublishGLImage;
class QOpenGLWidget;
class QOpenGLFunctions;

class PublishGLRenderer : public QObject
{
    Q_OBJECT

public:
    PublishGLRenderer(QObject *parent = nullptr);
    virtual ~PublishGLRenderer();

    virtual QColor getBackgroundColor();

    // DMH OpenGL renderer calls
    virtual void rendererActivated(QOpenGLWidget* glWidget);
    virtual void rendererDeactivated();
    virtual void cleanup();
    virtual bool deleteOnDeactivation();

    virtual QRect getScissorRect();

    virtual void updateRender();

    // Standard OpenGL calls
    virtual void initializeGL() = 0;
    virtual void resizeGL(int w, int h) = 0;
    virtual void paintGL() = 0;

signals:
    void initializationComplete();
    void updateWidget();
    void deactivated();

public slots:
    virtual void setBackgroundColor(const QColor& color);

protected:
    virtual void updateProjectionMatrix() = 0;

    QOpenGLWidget* _targetWidget;
};

#endif // PUBLISHGLRENDERER_H
