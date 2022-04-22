#ifndef PUBLISHGLMAPVIDEORENDERER_H
#define PUBLISHGLMAPVIDEORENDERER_H

#include "publishglmaprenderer.h"

class VideoPlayerGLPlayer;

class PublishGLMapVideoRenderer : public PublishGLMapRenderer
{
    Q_OBJECT
public:
    PublishGLMapVideoRenderer(QObject *parent = nullptr);
    virtual ~PublishGLMapVideoRenderer() override;

    // DMH OpenGL renderer calls
    virtual void cleanup() override;

    QImage getLastScreenshot();

protected:
    // Background overrides
    virtual void initializeBackground() override;
    virtual bool isBackgroundReady() override;
    virtual void resizeBackground(int w, int h) override;
    virtual void paintBackground(QOpenGLFunctions* functions) override;
    virtual QSizeF getBackgroundSize() override;
    virtual void updateBackground() override;

    VideoPlayerGLPlayer* _videoPlayer;
};

#endif // PUBLISHGLMAPVIDEORENDERER_H
