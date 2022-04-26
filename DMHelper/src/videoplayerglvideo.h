#ifndef VIDEOPLAYERGLVIDEO_H
#define VIDEOPLAYERGLVIDEO_H

#include "dmh_vlc.h"
#include <QObject>
#include <QSemaphore>
#include <QMutex>
#include <QSize>

class VideoPlayerGLPlayer;
class QOpenGLContext;
class QOpenGLFramebufferObject;
class QOffscreenSurface;

class VideoPlayerGLVideo : public QObject
{
    Q_OBJECT
public:
    VideoPlayerGLVideo(VideoPlayerGLPlayer* player);
    virtual ~VideoPlayerGLVideo();

    bool isNewFrameAvailable();
    QOpenGLFramebufferObject *getVideoFrame();
    QSize getVideoSize() const;

    static bool resizeRenderTextures(void* data, const libvlc_video_render_cfg_t *cfg,
                                     libvlc_video_output_cfg_t *render_cfg);
    static bool setup(void** data, const libvlc_video_setup_device_cfg_t *cfg,
                      libvlc_video_setup_device_info_t *out);
    static void cleanup(void* data);
    static void swap(void* data);
    static bool makeCurrent(void* data, bool current);
    static void* getProcAddress(void* data, const char* current);

protected slots:
    void configureContext(QOpenGLContext *renderContext);

protected:
    VideoPlayerGLPlayer *_player;
    QOpenGLContext *_context;
    QOffscreenSurface *_surface;
    QSemaphore _videoReady;

    //FBO data
    unsigned _width;
    unsigned _height;
    QMutex _textLock;
    //QOpenGLFramebufferObject *_buffers[3];
    QOpenGLFramebufferObject *_buffers[5];
    size_t _idxRender;
    //size_t _idxSwapRender;
    size_t _idxSwapA;
    size_t _idxSwapB;
    size_t _idxSwapC;
    size_t _idxDisplay;
    bool _updated;
    bool _initialized;
    int _frameCount;
};

#endif // VIDEOPLAYERGLVIDEO_H
