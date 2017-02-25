#ifndef SRC_BRICKS_OPENAL_OPENAL_HH_HH_INCLUDED
#define SRC_BRICKS_OPENAL_OPENAL_HH_HH_INCLUDED

#include <string.h>
#include <memory>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include <AL/al.h>
#include <AL/alc.h>
#include <vorbis/vorbisfile.h>

#include "audio.hh"
#include "context.hh"

using namespace std;

int endWithError(char* msg);

#define BUFFER_SIZE 32768  // 32 KB buffers
#define File_Type_WAV 0
#define File_Type_OGG 1
#define Internal_Error 3

namespace gdt::audio::openal {

class backend;
class openal_sound : public gdt::blueprints::audio::sound {
  private:
    backend* _backend;
    std::string _filename;

  public:
    openal_sound(const gdt::audio_context<backend>& ctx, std::string filename);

    void play() const override;

    void stop() const override;

    bool is_playing() const override;
    void volume(float vol) override;
    void position(math::vec3 pos) override;
};
//class openal_sound; 
//class backend : public gdt::audio_backend<backend, openal_sound> {
class backend : public gdt::blueprints::audio::backend<openal_sound> {
    // Intances of sound class represent individual sound files used. It handles things such as
    // play/stop, loading from hdd and decoding(I've made it able to use .wav and .ogg).
    // Each sound is identified by the location on disk it was loaded from.
    class sound_impl {
      public:
        sound_impl(string path);
        ~sound_impl(void)
        {
            shutdown();
        };

        int play();
        int stop();
        int load(string path);  // Load from hdd.
        bool is_playing;        // This is true while the sound is playing. You can also use
                                // alGetSourcei(source, AL_SOURCE_STATE, &state);
        void gain(float g);
        void pos(gdt::math::vec3 pos);

        void shutdown();

        bool invalid_file;
        string filepath;

        uint32_t size, chunk_size;
        short format_type, channels;
        uint32_t sample_rate, avg_bytes_per_sec;
        short bytes_per_sample, bits_per_sample;
        uint32_t data_size;

        // Load from hdd into RAM and decode it so it's ready to be transfered into the OpenAL
        // device.
        int load_wav(string path);
        bool load_ogg(string fileName);

        int load_into_al();
        int get_sound_file_type(string filepath);

        char* buf;

        ALuint source;
        ALuint buffer;
        ALenum format;  // The audio format (bits per sample, number of channels)
    };

  public:
    // Nothing is done upon construcion.
    backend(void)
    {
        init_open_al();
    };
    ~backend(void)
    {
        shutdown();
    };

    // This function returns the Sounds vector index of the file loaded from path in argument.
    int find_sound(string path);

    // Returns 1 on success.
    // Use: if( !AddASound(path) ) Error(); else PlayASound(path);
    int add_sound(string path);
    // This is more of a joke than it's actually useful, I just felt like making it possible to
    // use += to add new sounds.
    int operator+=(string path)
    {
        return add_sound(path);
    }
    void remove_sound(string path);

    void play_sound(string path);
    void stop_sound(string path);

    bool is_playing(string path);

  private:
    // This function does the same as FindSound() except it returns the sound object instead of
    // just the index.
    sound_impl & get_sound(string path)
    {
        return *_sounds[find_sound(path)];
    }

    int init_open_al();
    void shutdown();

    // OpenAL internal stuff, similar to OpenGL.
    ALCdevice* device;
    ALCcontext* context;
    std::vector<std::unique_ptr<backend::sound_impl>>
        _sounds;  // This vector holds all the sounds we have loaded so far.
};

//class openal_sound {// : public gdt::sound {
}

#endif  // src/backends/openal/openal_hh_hh_INCLUDED
