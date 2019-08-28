#ifndef AUDIOPLUGIN_H
#define AUDIOPLUGIN_H

#include <QObject>
#include <QWidget>
#include <external/vst/aeffect.h>
#include <external/vst/aeffectx.h>
#include <wpn114audio/graph.hpp>

#ifdef __APPLE__
#include <QMacNativeWidget>
#include <QMacCocoaViewContainer>
#endif

#ifdef __linux__
#include <dlfcn.h>
#endif

//-------------------------------------------------------------------------------------------------
using vstint32_t        = VstInt32;
using vstptr_t          = VstIntPtr;
using aeffect           = AEffect;
using master_callback   = audioMasterCallback;

typedef aeffect* (*vstentry_t)(master_callback);

//-------------------------------------------------------------------------------------------------
enum class typeversion
//-------------------------------------------------------------------------------------------------
{
    AU          = 1,
    VST2X       = 2,
    VST3X       = 3,
    LV2         = 4,
    AAX         = 5
};

//-------------------------------------------------------------------------------------------------
enum class MIDI
//-------------------------------------------------------------------------------------------------
{
    SYSEX                   = 0xf0,
    EOX                     = 0xf7,
    NOTE_OFF                = 0x80,
    NOTE_ON                 = 0x90,
    AFTERTOUCH              = 0xa0,
    CONTINUOUS_CONTROL      = 0xb0,
    PATCH_CHANGE            = 0xc0,
    CHANNEL_PRESSURE        = 0xd0,
    PITCH_BEND              = 0xe0
};

//=================================================================================================
class Vst2x : public Node
//=================================================================================================
{
    Q_OBJECT

    WPN_DECLARE_DEFAULT_AUDIO_INPUT  (audio_in, 0)
    WPN_DECLARE_DEFAULT_AUDIO_OUTPUT (audio_out, 0)
    WPN_DECLARE_DEFAULT_MIDI_INPUT   (midi_in, 0)
    WPN_DECLARE_DEFAULT_MIDI_OUTPUT  (midi_out, 0)

    Q_PROPERTY (QString path READ path WRITE set_path)

    QString
    m_path;

    void*
    m_dl = nullptr;

    aeffect*
    m_aeffect = nullptr;

    QWidget*
    m_view = nullptr;

public:

    //---------------------------------------------------------------------------------------------
    Q_SIGNAL void
    loaded();

    //---------------------------------------------------------------------------------------------
    static vstptr_t VSTCALLBACK
    callback(aeffect* effect, vstint32_t opcode, vstint32_t index, vstptr_t value, void* ptr, float opt)
    //---------------------------------------------------------------------------------------------
    {
        vstptr_t result = 0;

        switch(opcode)
        {
        case audioMasterAutomate: break;
        case audioMasterCurrentId: break;
        case audioMasterVersion: return 2400;
        case audioMasterIdle: return 1;
        case audioMasterGetCurrentProcessLevel: return kVstProcessLevelUser;
        case audioMasterGetProductString:
            std::copy_n("wpn214", 7, static_cast<char*>(ptr));
        }

        return result;
    }

    //---------------------------------------------------------------------------------------------
    Vst2x() : m_view(new QWidget)
    //---------------------------------------------------------------------------------------------
    {

    }

    //---------------------------------------------------------------------------------------------
    virtual
    ~Vst2x() override
    //---------------------------------------------------------------------------------------------
    {
        delete m_view;
    }

    //---------------------------------------------------------------------------------------------
    QString
    path() const { return m_path; }

    //---------------------------------------------------------------------------------------------
    void
    set_path(QString path)
    //---------------------------------------------------------------------------------------------
    {
        m_path = path;
    }

    //---------------------------------------------------------------------------------------------
    virtual void
    componentComplete() override
    //---------------------------------------------------------------------------------------------
    {
        m_dl = dlopen(CSTR(m_path), RTLD_NOW);

        auto proc = (vstentry_t) dlsym(m_dl, "VSTPluginMain");
        if  (proc == nullptr)
             proc = (vstentry_t) dlsym(m_dl, "main");

        assert(proc); // for now..
        m_aeffect = proc(callback);
        m_aeffect->dispatcher(m_aeffect, effOpen, 0, 0, nullptr, 0);

        uint16_t
        esize[2] = { 0, 0 };

        ERect* rect = nullptr;
        m_aeffect->dispatcher(m_aeffect, effEditGetRect, 0, 0, &rect, 0);

        if (rect) {
            esize[0] = rect->right-rect->left;
            esize[1] = rect->bottom-rect->top;
        }

        m_view->setFixedSize(esize[0], esize[1]);
        m_view->update();
        emit loaded();
    }

    //---------------------------------------------------------------------------------------------
    Q_INVOKABLE void
    display()
    //---------------------------------------------------------------------------------------------
    {
        m_aeffect->dispatcher(m_aeffect, effEditOpen, 0, 0, (void*) m_view->winId(), 0);
        m_view->show();
    }

    //---------------------------------------------------------------------------------------------
    virtual void
    initialize(Graph::properties const& properties) override
    //---------------------------------------------------------------------------------------------
    {
        m_aeffect->dispatcher(m_aeffect, effSetSampleRate, 0, 0, nullptr, properties.rate);
        m_aeffect->dispatcher(m_aeffect, effSetBlockSize, 0, properties.vector, nullptr, 0);
        m_aeffect->dispatcher(m_aeffect, effMainsChanged, 0, 1, nullptr, 0);
        m_aeffect->dispatcher(m_aeffect, effStartProcess, 0, 0, nullptr, 0);
    }

    //---------------------------------------------------------------------------------------------
    virtual void
    on_rate_changed(sample_t rate) override
    //---------------------------------------------------------------------------------------------
    {
        m_aeffect->dispatcher(m_aeffect, effSetSampleRate, 0, 0, nullptr, rate);
    }

    //---------------------------------------------------------------------------------------------
    virtual void
    rwrite(pool& inputs, pool& outputs, vector_t nframes) override
    //---------------------------------------------------------------------------------------------
    {
        auto audio_in  = inputs.audio[0];
        auto audio_out = outputs.audio[0];
        auto midi_in   = inputs.midi[0];
        auto midi_out  = outputs.midi[0];

        // send midi events first
        for (auto& mt : *midi_in[0])
        {
            // TODO
//            m_aeffect->dispatcher(m_aeffect, effProcessEvents, 0, 0, events, 0);
        }

        m_aeffect->processReplacing(m_aeffect, audio_in, audio_out, nframes);
    }
};

#endif // AUDIOPLUGIN_H
