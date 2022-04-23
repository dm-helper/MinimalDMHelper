#include "dmh_vlc.h"

#define VIDEO_DEBUG_MESSAGES

DMH_VLC* DMH_VLC::_instance = nullptr;

DMH_VLC::DMH_VLC() :
    _vlcInstance(nullptr)
{
#ifdef VIDEO_DEBUG_MESSAGES
    /*
    const char *args[] = {
        "--no-hw-dec",
        "--verbose=3",
        ""
    };
    _vlcInstance = libvlc_new(sizeof(args) / sizeof(*args), args);
    */
    const char *verbose_args = "-vvv";
    _vlcInstance = libvlc_new(1, &verbose_args);
#else
    _vlcInstance = libvlc_new(0, nullptr);
#endif
}

DMH_VLC::~DMH_VLC()
{
    if(_vlcInstance)
    {
        libvlc_release(_vlcInstance);
        _vlcInstance = nullptr;
    }
}

libvlc_instance_t* DMH_VLC::Instance()
{
    if(!_instance)
    {
        Initialize();
        if(!_instance)
            return nullptr;
    }

    return _instance->_vlcInstance;
}

void DMH_VLC::Initialize()
{
    if(_instance)
        return;

    _instance = new DMH_VLC();
}

void DMH_VLC::Shutdown()
{
    if(_instance)
    {
        DMH_VLC* deleteInstance = _instance;
        _instance = nullptr;
        delete deleteInstance;
    }
}
