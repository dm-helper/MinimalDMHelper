#include "videoplayerglplayer.h"
#include "videoplayerglvideo.h"
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <QTimerEvent>
#include <QDebug>

#define VIDEO_DEBUG_MESSAGES

const int stopCallComplete = 0x01;
const int stopConfirmed = 0x02;
const int stopComplete = stopCallComplete | stopConfirmed;
const int INVALID_TRACK_ID = -99999;

VideoPlayerGLPlayer::VideoPlayerGLPlayer(const QString& videoFile, QOpenGLContext* context, QSurfaceFormat format, bool playVideo, bool playAudio, QObject *parent) :
    QObject(parent),
    _videoFile(videoFile),
    _context(context),
    _format(format),
    _videoSize(),
    _playVideo(playVideo),
    _playAudio(playAudio),
    _video(nullptr),
    _fboTexture(0),
    _vlcError(false),
    _vlcPlayer(nullptr),
    _vlcMedia(nullptr),
    _status(-1),
    _selfRestart(false),
    _deleteOnStop(false),
    _stopStatus(0),
    _firstImage(false),
    _originalTrack(INVALID_TRACK_ID),
    _modelMatrix(),
    _VAO(0),
    _VBO(0),
    _EBO(0),

    _tempTexture(0),
    _frameCount(0)
{
    if(_context)
    {
        _videoFile.replace("/","\\\\");
        _vlcError = !VideoPlayerGLPlayer::initializeVLC();
#ifdef VIDEO_DEBUG_MESSAGES
        qDebug() << "[VideoPlayerGLPlayer] Player object initialized: " << this;
#endif
    }
}

VideoPlayerGLPlayer::~VideoPlayerGLPlayer()
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Destroying player object: " << this;
#endif

    _selfRestart = false;
    cleanupPlayer();
    cleanupVBObjects();

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Player object destroyed: " << this;
#endif

}

const QString& VideoPlayerGLPlayer::getFileName() const
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Getting file name: " << _videoFile;
#endif

    return _videoFile;
}

void VideoPlayerGLPlayer::paintGL()
{
    if((!_context) || (!_video))
    {
        qDebug() << "[VideoPlayerGLPlayer] Background paint ABORTED!!!!!!!!!!";
        return;
    }

    QOpenGLFunctions *f = _context->functions();
    QOpenGLExtraFunctions *e = _context->extraFunctions();
    if((!f) || (!e))
    {
        qDebug() << "[VideoPlayerGLPlayer] Background paint ABORTED!!!!!!!!!!";
        return;
    }

    bool newFrame = _video->isNewFrameAvailable();
    if((newFrame) && (++_frameCount > 0))
    //if(newFrame)
    {
#ifdef VIDEO_DEBUG_MESSAGES
        qDebug() << "[VideoPlayerGLPlayer] New frame";
#endif
        QOpenGLFramebufferObject *fbo = _video->getVideoFrame();
        if(fbo)
        {
#ifdef VIDEO_DEBUG_MESSAGES
            if(!fbo->isValid())
                qDebug() << "[VideoPlayerGLPlayer] ERROR: invalid frame buffer object: " << fbo;
#endif

            if(_fboTexture > 0)
            {
#ifdef VIDEO_DEBUG_MESSAGES
            qDebug() << "[VideoPlayerGLPlayer] Deleting old texture: " << _fboTexture;
#endif
                f->glDeleteTextures(1, &_fboTexture);
                _fboTexture = -1;
            }
            _fboTexture = fbo->takeTexture();
            _frameCount = 0;
#ifdef VIDEO_DEBUG_MESSAGES
            qDebug() << "[VideoPlayerGLPlayer] New Texture: " << _fboTexture;
#endif
        }
    }
    else
    {
#ifdef VIDEO_DEBUG_MESSAGES
        qDebug() << "[VideoPlayerGLPlayer] Reusing Texture: " << _fboTexture;
#endif
    }

    if(_fboTexture <= 0)
    {
        qDebug() << "[VideoPlayerGLPlayer] Background paint ABORTED!!!!!!!!!!";
        return;
    }

    e->glBindVertexArray(_VAO);

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Painting texture: " << _fboTexture << ", new frame: " << newFrame << " from video " << _video << " with size " << _video->getVideoSize();
#endif

    f->glBindTexture(GL_TEXTURE_2D, _fboTexture);
    //f->glBindTexture(GL_TEXTURE_2D, _tempTexture);
    f->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

bool VideoPlayerGLPlayer::isPlayingVideo() const
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Getting playing video state: " << _playVideo;
#endif

    return _playVideo;
}

void VideoPlayerGLPlayer::setPlayingVideo(bool playVideo)
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Setting playing video state: " << playVideo;
#endif

    _playVideo = playVideo;
}

bool VideoPlayerGLPlayer::isPlayingAudio() const
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Getting playing audio state: " << _playAudio;
#endif

    return _playAudio;
}

void VideoPlayerGLPlayer::setPlayingAudio(bool playAudio)
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Setting playing audio state: " << playAudio;
#endif
    _playAudio = playAudio;

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Playing audio state set";
#endif

}

bool VideoPlayerGLPlayer::isError() const
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Getting error state: " << _vlcError;
#endif

    return _vlcError;
}

QSize VideoPlayerGLPlayer::getOriginalSize() const
{
    QSize originalSize = _video ? _video->getVideoSize() : QSize();

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Getting original size: " << originalSize;
#endif

    return originalSize;
}

void VideoPlayerGLPlayer::registerNewFrame()
{
    qDebug() << "[VideoPlayerGLPlayer] Confirming frame available";
    emit frameAvailable();
}

QSurfaceFormat VideoPlayerGLPlayer::getFormat() const
{
    return _format;
}

QSize VideoPlayerGLPlayer::getSize() const
{
    return _videoSize;
}

QImage VideoPlayerGLPlayer::getLastScreenshot()
{
    if(!_video)
        return QImage();

    QOpenGLFramebufferObject* fbo = _video->getVideoFrame();
    if(!fbo)
        return QImage();

    return fbo->toImage();
}

void VideoPlayerGLPlayer::playerEventCallback( const struct libvlc_event_t *p_event, void *p_data )
{
    if((!p_event) || (!p_data))
        return;

    VideoPlayerGLPlayer* that = static_cast<VideoPlayerGLPlayer*>(p_data);
    if(!that)
        return;

    switch(p_event->type)
    {
        case libvlc_MediaPlayerOpening:
            qDebug() << "[VideoPlayerGLPlayer] Video event received: OPENING = " << p_event->type;
            break;
        case libvlc_MediaPlayerBuffering:
            qDebug() << "[VideoPlayerGLPlayer] Video event received: BUFFERING = " << p_event->type;
            break;
        case libvlc_MediaPlayerPlaying:
            qDebug() << "[VideoPlayerGLPlayer] Video event received: PLAYING = " << p_event->type;
            break;
        case libvlc_MediaPlayerPaused:
            qDebug() << "[VideoPlayerGLPlayer] Video event received: PAUSED = " << p_event->type;
            break;
        case libvlc_MediaPlayerStopped:
            qDebug() << "[VideoPlayerGLPlayer] Video event received: STOPPED = " << p_event->type;
            break;
        default:
            qDebug() << "[VideoPlayerGLPlayer] UNEXPECTED Video event received:  " << p_event->type;
            break;
    };

    that->_status = p_event->type;
}

void VideoPlayerGLPlayer::stopThenDelete()
{
    qDebug() << "[VideoPlayerGLPlayer] Stop Then Delete triggered, stop called...";
    _deleteOnStop = true;
    stopPlayer();

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] stopThenDelete completed";
#endif

}

bool VideoPlayerGLPlayer::restartPlayer()
{
    if(_vlcPlayer)
    {
        qDebug() << "[VideoPlayerGLPlayer] Restart Player called, stop called...";
        _selfRestart = true;
        return stopPlayer();
    }
    else
    {
        qDebug() << "[VideoPlayerGLPlayer] Restart Player called, but no player running - starting player!";
        return startPlayer();
    }
}

void VideoPlayerGLPlayer::videoResized()
{
    qDebug() << "[VideoPlayerGLPlayer] Video being resized, recreating vertex arrays";
    cleanupVBObjects();
    createVBObjects();
}

void VideoPlayerGLPlayer::initializationComplete()
{
    if((!_context) || (!_video))
        return;

    qDebug() << "[VideoPlayerGLPlayer] Confirming initialization complete";
    emit contextReady(_context);
}

void VideoPlayerGLPlayer::timerEvent(QTimerEvent *event)
{
    if((_status == libvlc_MediaPlayerOpening) || (_status == libvlc_MediaPlayerBuffering) || (_status == libvlc_MediaPlayerPlaying))
        return;

    cleanupPlayer();
    killTimer(event->timerId());

    if(_selfRestart)
    {
        _selfRestart = false;
        startPlayer();
        qDebug() << "[VideoPlayerGLPlayer] Internal Stop Check: player restarted.";
    }
    else
    {
        qDebug() << "[VideoPlayerGLPlayer] Internal Stop Check: video player being destroyed.";
        deleteLater();
    }
}

bool VideoPlayerGLPlayer::initializeVLC()
{
    qDebug() << "[VideoPlayerGLPlayer] Initializing VLC!";

    if(!_context)
    {
        qDebug() << "[VideoPlayerGLPlayer] ERROR: No context provided, not initializing VLC!";
        return false;
    }

    if(_videoFile.isEmpty())
    {
        qDebug() << "[VideoPlayerGLPlayer] ERROR: Playback file empty - not initializing VLC!";
        return false;
    }

    if(!DMH_VLC::Instance())
        return false;

    _video = new VideoPlayerGLVideo(this);

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Initializing VLC completed";
#endif

    return true;
}

bool VideoPlayerGLPlayer::startPlayer()
{
    if(!DMH_VLC::Instance())
    {
        qDebug() << "[VideoPlayerGLPlayer] VLC not instantiated - not able to start player!";
        return false;
    }

    if(_vlcPlayer)
    {
        qDebug() << "[VideoPlayerGLPlayer] Player already running - not able to start player!";
        return false;
    }

    if(_videoFile.isEmpty())
    {
        qDebug() << "[VideoPlayerGLPlayer] Playback file empty - not able to start player!";
        return false;
    }

    qDebug() << "[VideoPlayerGLPlayer] Starting video player with " << _videoFile.toUtf8().constData();

    // Create a new Media
    _vlcMedia = libvlc_media_new_path(DMH_VLC::Instance(), _videoFile.toUtf8().constData());
    if (!_vlcMedia)
        return false;

    _vlcPlayer = libvlc_media_player_new_from_media(_vlcMedia);
    if(!_vlcPlayer)
        return false;

    libvlc_audio_set_volume(_vlcPlayer, 0);

    // Set up event callbacks
    libvlc_event_manager_t* eventManager = libvlc_media_player_event_manager(_vlcPlayer);
    if(eventManager)
    {
        libvlc_event_attach(eventManager, libvlc_MediaPlayerOpening, playerEventCallback, static_cast<void*>(this));
        libvlc_event_attach(eventManager, libvlc_MediaPlayerBuffering, playerEventCallback, static_cast<void*>(this));
        libvlc_event_attach(eventManager, libvlc_MediaPlayerPlaying, playerEventCallback, static_cast<void*>(this));
        libvlc_event_attach(eventManager, libvlc_MediaPlayerPaused, playerEventCallback, static_cast<void*>(this));
        libvlc_event_attach(eventManager, libvlc_MediaPlayerStopped, playerEventCallback, static_cast<void*>(this));
    }

    bool callbackResult = libvlc_video_set_output_callbacks(_vlcPlayer,
                                                            libvlc_video_engine_opengl,
                                                            VideoPlayerGLVideo::setup,
                                                            VideoPlayerGLVideo::cleanup,
                                                            nullptr,
                                                            VideoPlayerGLVideo::resizeRenderTextures,
                                                            VideoPlayerGLVideo::swap,
                                                            VideoPlayerGLVideo::makeCurrent,
                                                            VideoPlayerGLVideo::getProcAddress,
                                                            nullptr,
                                                            nullptr,
                                                            _video);

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Player callback result: " << callbackResult;
#endif

    // And start playback
    libvlc_media_player_play(_vlcPlayer);

    createVBObjects();

    qDebug() << "[VideoPlayerGLPlayer] Player started";

    return true;
}

bool VideoPlayerGLPlayer::stopPlayer()
{
    qDebug() << "[VideoPlayerGLPlayer] Stop Player called";

    if(_vlcPlayer)
        libvlc_media_player_stop_async(_vlcPlayer);

    startTimer(500);

    return true;
}

void VideoPlayerGLPlayer::cleanupPlayer()
{
    qDebug() << "[VideoPlayerGLPlayer] Player being cleaned up";

    if(_vlcPlayer)
    {
        libvlc_media_player_release(_vlcPlayer);
        _vlcPlayer = nullptr;
    }

    if(_vlcMedia)
    {
        libvlc_media_release(_vlcMedia);
        _vlcMedia = nullptr;
    }

    if(_video)
    {
        delete _video;
        _video = nullptr;
    }
}

void VideoPlayerGLPlayer::createVBObjects()
{
    if((!_context) || (!_video))
        return;

    // Set up the rendering context, load shaders and other resources, etc.:
    QOpenGLFunctions *f = _context->functions();
    QOpenGLExtraFunctions *e = _context->extraFunctions();
    if((!f) || (!e))
        return;


    // Texture
    f->glGenTextures(1, &_tempTexture);
    f->glBindTexture(GL_TEXTURE_2D, _tempTexture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


    // load and generate the background texture
    QImage image(QString("mountainruins.png"));

    QImage glBackgroundImage = image.convertToFormat(QImage::Format_RGBA8888).mirrored();
    f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glBackgroundImage.width(), glBackgroundImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, glBackgroundImage.bits());
    f->glGenerateMipmap(GL_TEXTURE_2D);


//    _videoSize = _video->getVideoSize();
    _videoSize = image.size();
    if((_videoSize.width() <= 0) || (_videoSize.height() <= 0))
        return;

    float vertices[] = {
        // positions    // colors           // texture coords
         (float)_videoSize.width() / 2,  (float)_videoSize.height() / 2, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
         (float)_videoSize.width() / 2, -(float)_videoSize.height() / 2, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
        -(float)_videoSize.width() / 2, -(float)_videoSize.height() / 2, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -(float)_videoSize.width() / 2,  (float)_videoSize.height() / 2, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left
    };

    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Creating video vertex buffers with size: " << _videoSize;
#endif

    e->glGenVertexArrays(1, &_VAO);
    f->glGenBuffers(1, &_VBO);
    f->glGenBuffers(1, &_EBO);

    e->glBindVertexArray(_VAO);

    f->glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    f->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
    f->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    f->glEnableVertexAttribArray(0);
    // color attribute
    f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
    f->glEnableVertexAttribArray(1);
    // texture attribute
    f->glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    f->glEnableVertexAttribArray(2);

    emit vbObjectsCreated();
}

void VideoPlayerGLPlayer::cleanupVBObjects()
{
    if(!_context)
        return;

    QOpenGLFunctions *f = _context->functions();
    QOpenGLExtraFunctions *e = _context->extraFunctions();
    if((!f) || (!e))
        return;

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Cleaning up video vertex buffers";
#endif

    _videoSize = QSize();

    if(_VAO > 0)
    {
        e->glDeleteVertexArrays(1, &_VAO);
        _VAO = 0;
    }

    if(_VBO > 0)
    {
        f->glDeleteBuffers(1, &_VBO);
        _VBO = 0;
    }

    if(_EBO > 0)
    {
        f->glDeleteBuffers(1, &_EBO);
        _EBO = 0;
    }
}

void VideoPlayerGLPlayer::internalAudioCheck(int newStatus)
{
    if((_playAudio) ||
       (_originalTrack != INVALID_TRACK_ID) ||
       (newStatus != libvlc_MediaPlayerPlaying) ||
       (!_vlcPlayer))
        return;

    qDebug() << "[VideoPlayerGLPlayer] Internal Audio Check identified audio, shall be turned off";

    _originalTrack = libvlc_audio_get_track(_vlcPlayer);
    if(_originalTrack != -1)
        libvlc_audio_set_track(_vlcPlayer, -1);

    qDebug() << "[VideoPlayerGLPlayer] Audio turning off completed";
}

GLuint VideoPlayerGLPlayer::getfboTexture() const
{
    return _fboTexture;
}

bool VideoPlayerGLPlayer::isPlaying() const
{
    bool result = ((_status == libvlc_MediaPlayerBuffering) ||
                   (_status == libvlc_MediaPlayerPlaying));

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Getting is playing status: " << result;
#endif

    return result;
}

bool VideoPlayerGLPlayer::isPaused() const
{
    bool result = (_status == libvlc_MediaPlayerPaused);

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Getting is paused status: " << result;
#endif

    return result;
}

bool VideoPlayerGLPlayer::isProcessing() const
{
    bool result = ((_status == libvlc_MediaPlayerOpening) ||
                   (_status == libvlc_MediaPlayerBuffering) ||
                   (_status == libvlc_MediaPlayerPlaying) ||
                   (_status == libvlc_MediaPlayerPaused));

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Getting is processing status: " << result;
#endif

    return result;
}

bool VideoPlayerGLPlayer::isStatusValid() const
{
    bool result = ((_status == libvlc_MediaPlayerOpening) ||
                   (_status == libvlc_MediaPlayerBuffering) ||
                   (_status == libvlc_MediaPlayerPlaying) ||
                   (_status == libvlc_MediaPlayerPaused) ||
                   (_status == libvlc_MediaPlayerStopped));

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Getting is status valid: " << result;
#endif

    return result;
}





