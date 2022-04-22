#include "publishglrenderer.h"
#include "dmconstants.h"
#include <QOpenGLWidget>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QDebug>

PublishGLRenderer::PublishGLRenderer(QObject *parent) :
    QObject(parent),
    _targetWidget(nullptr)
{
}

PublishGLRenderer::~PublishGLRenderer()
{
}

QColor PublishGLRenderer::getBackgroundColor()
{
    return QColor();
}

void PublishGLRenderer::rendererActivated(QOpenGLWidget* glWidget)
{
    _targetWidget = glWidget;

    if((_targetWidget) && (_targetWidget->context()))
        connect(_targetWidget->context(), &QOpenGLContext::aboutToBeDestroyed, this, &PublishGLRenderer::cleanup);
}

void PublishGLRenderer::rendererDeactivated()
{
    qDebug() << "[PublishGLRenderer] Renderer deactivated: " << this;
    if((_targetWidget) && (_targetWidget->context()))
        disconnect(_targetWidget->context(), &QOpenGLContext::aboutToBeDestroyed, this, &PublishGLRenderer::cleanup);

    cleanup();

    emit deactivated();
    _targetWidget = nullptr;
}

void PublishGLRenderer::cleanup()
{
}

bool PublishGLRenderer::deleteOnDeactivation()
{
    return false;
}

QRect PublishGLRenderer::getScissorRect()
{
    return QRect();
}

void PublishGLRenderer::updateRender()
{
    emit updateWidget();
}

void PublishGLRenderer::setBackgroundColor(const QColor& color)
{
    Q_UNUSED(color);
}
