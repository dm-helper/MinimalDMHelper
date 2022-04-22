#include "publishglmapvideorenderer.h"
#include "videoplayerglplayer.h"
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QDebug>

PublishGLMapVideoRenderer::PublishGLMapVideoRenderer(QObject *parent) :
    PublishGLMapRenderer(parent),
    _videoPlayer(nullptr)
{
}

PublishGLMapVideoRenderer::~PublishGLMapVideoRenderer()
{
    PublishGLMapVideoRenderer::cleanup();
}

void PublishGLMapVideoRenderer::cleanup()
{
    if(_videoPlayer)
    {
        VideoPlayerGLPlayer* deletePlayer = _videoPlayer;
        _videoPlayer = nullptr;
        deletePlayer->stopThenDelete();
    }

    PublishGLMapRenderer::cleanup();
}

void PublishGLMapVideoRenderer::initializeBackground()
{
    // Create the objects
    _videoPlayer = new VideoPlayerGLPlayer(QString("./Airship_gridLN.m4v"),
                                           _targetWidget->context(),
                                           _targetWidget->format(),
                                           _targetSize,
                                           true,
                                           false);
    connect(_videoPlayer, &VideoPlayerGLPlayer::frameAvailable, this, &PublishGLMapVideoRenderer::updateWidget);
    connect(_videoPlayer, &VideoPlayerGLPlayer::vbObjectsCreated, this, &PublishGLMapVideoRenderer::updateProjectionMatrix);
    _videoPlayer->restartPlayer();
}

bool PublishGLMapVideoRenderer::isBackgroundReady()
{
    return _videoPlayer != nullptr;
}

void PublishGLMapVideoRenderer::resizeBackground(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);

    if(!_videoPlayer)
        return;

    _videoPlayer->targetResized(_targetSize);
    _videoPlayer->initializationComplete();
    updateProjectionMatrix();
}

void PublishGLMapVideoRenderer::paintBackground(QOpenGLFunctions* functions)
{
    if(!_videoPlayer)
        return;

    QMatrix4x4 modelMatrix;
    functions->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, modelMatrix.constData());
    _videoPlayer->paintGL();
}

QSizeF PublishGLMapVideoRenderer::getBackgroundSize()
{
    return _videoPlayer ? _videoPlayer->getSize() : QSizeF();
}

void PublishGLMapVideoRenderer::updateBackground()
{
}

QImage PublishGLMapVideoRenderer::getLastScreenshot()
{
    return _videoPlayer ? _videoPlayer->getLastScreenshot() : QImage();
}
