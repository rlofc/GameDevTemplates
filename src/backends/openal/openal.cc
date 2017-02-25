#include "backends/openal/openal.hh"
#include "checks.hh"

#define AL_CHECK(WHAT)                                      \
  WHAT;                                                     \
  {                                                         \
      int err;                                              \
      if ((err = alGetError()) != AL_NO_ERROR) {            \
        LOG_DEBUG << "alGetError() " << err;                \
        throw std::runtime_error(                           \
            "Error GenSource");                             \
      }                                                     \
  } 

int endWithError(char *msg) {
  printf("%s\n", msg);
  return 0;
}

namespace gdt::audio::openal {
backend::sound_impl::sound_impl(string path) {
  LOG_DEBUG << "initializing sound for " << path;
  format = 0;
  data_size= 0;
  if (load(path) != 0) invalid_file= true;
  is_playing = false;
  LOG_DEBUG << "initialized sound for " << path;
};

int backend::sound_impl::get_sound_file_type(string filepath) {
  string extension = filepath.substr(filepath.length() - 4, 4);
  if (extension == ".ogg")
    return File_Type_OGG;
  else if (extension == ".wav")
    return File_Type_WAV;
  else
    return Internal_Error;
}

int backend::sound_impl::load(string path) {
  LOG_DEBUG << "loading " << path;
  invalid_file= false;
  filepath = path;

  // Find out what kind of file is given to us by checking it's extension.
  switch (get_sound_file_type(path)) {
    // Once we found it, use proper method to decode it and load into RAM.
    case File_Type_WAV:
      if (!load_wav(path)) return 1;
      break;
    case File_Type_OGG:
      LOG_DEBUG << "is ogg";
      if (!load_ogg(path)) return 2;
      break;
    default:
      return Internal_Error;
  }
  // Do final preparations and load the sound from memory into OpenAL
  // device(which is also in RAM but you can't see what it does behind the API).
  if (!load_into_al()) return Internal_Error;
  LOG_DEBUG << "loaded " << path;
  return 0;
}

int backend::sound_impl::load_wav(string path) {
  // Loading of the WAVE file
  FILE *fp = NULL;  // Create FILE pointer for the WAVE file
  fp = fopen((char *)path.c_str(), "rb");               // Open the WAVE file
  if (!fp) throw std::runtime_error("Failed to open file");  // Could not open file

  char type[4];

  // Check that the WAVE file is OK
  fread(type, sizeof(char), 4, fp);  // Reads the first bytes in the file
  if (type[0] != 'R' || type[1] != 'I' || type[2] != 'F' ||
      type[3] != 'F')                // Should be "RIFF"
    throw std::runtime_error("No RIFF");  // Not RIFF

  fread(&size, sizeof(uint32_t), 1, fp);  // Continue to read the file
  fread(type, sizeof(char), 4, fp);       // Continue to read the file
  if (type[0] != 'W' || type[1] != 'A' || type[2] != 'V' ||
      type[3] != 'E')                 // This part should be "WAVE"
    throw std::runtime_error("not WAVE");  // Not WAVE

  fread(type, sizeof(char), 4, fp);  // Continue to read the file
  if (type[0] != 'f' || type[1] != 'm' || type[2] != 't' ||
      type[3] != ' ')                 // This part should be "fmt "
    throw std::runtime_error("not fmt ");  // Not fmt

  // Now we know that the file is an acceptable WAVE file
  // Info about the WAVE data is now read and stored
  fread(&chunk_size, sizeof(uint32_t), 1, fp);
  fread(&format_type, sizeof(short), 1, fp);
  fread(&channels, sizeof(short), 1, fp);
  fread(&sample_rate, sizeof(uint32_t), 1, fp);
  fread(&avg_bytes_per_sec, sizeof(uint32_t), 1, fp);
  fread(&bytes_per_sample, sizeof(short), 1, fp);
  fread(&bits_per_sample, sizeof(short), 1, fp);

  fread(type, sizeof(char), 4, fp);
  if (type[0] != 'd' || type[1] != 'a' || type[2] != 't' ||
      type[3] != 'a')                     // This part should be "data"
    throw std::runtime_error("Missing DATA");  // not data

  fread(&data_size, sizeof(uint32_t), 1,
        fp);                 // The size of the sound data is read
  buf = new char[data_size];  // Allocate memory for the sound data
  fread(buf, sizeof(uint8_t), data_size,
        fp);  // Read the sound data and display the

  // Clean-up
  fclose(fp);  // Close the WAVE file

  // Figure out the format of the WAVE file
  if (bits_per_sample == 8) {
    if (channels == 1)
      format = AL_FORMAT_MONO8;
    else if (channels == 2)
      format = AL_FORMAT_STEREO8;
  } else if (bits_per_sample == 16) {
    if (channels == 1)
      format = AL_FORMAT_MONO16;
    else if (channels == 2)
      format = AL_FORMAT_STEREO16;
  }
  if (!format) throw std::runtime_error("Wrong BitPerSample");  // Not valid format

  return true;
}

bool backend::sound_impl::load_ogg(string filename) {
  int endian = 0;  // 0 for Little-Endian, 1 for Big-Endian
  int bit_stream;
  long bytes;
  char array[BUFFER_SIZE];  // Local fixed size array
  FILE *f;
  vector<char> buffer;

  // OPEN FILE FOR DECODING

  // Open for binary reading
  f = fopen(filename.c_str(), "rb");
  if (f == NULL) {
    printf("\nFile not found: %s", filename.c_str());
    return false;
  }

  vorbis_info *info;
  OggVorbis_File ogg_file;

  // Let the SDK do its stuff
  ov_open(f, &ogg_file, NULL, 0);

  // GET INFO

  info = ov_info(&ogg_file, -1);
  if (info == NULL) return false;

  // Check the number of channels... always use 16-bit samples
  if (info->channels == 1)
    format = AL_FORMAT_MONO16;
  else if (info->channels == 2)
    format = AL_FORMAT_STEREO16;
  else if (info->channels == 4)
    format = alGetEnumValue("AL_FORMAT_QUAD16");
  else if (info->channels == 6)
    format = alGetEnumValue("AL_FORMAT_51CHN16");

  // The frequency of the sampling rate
  sample_rate = info->rate;

  // DECODE THE DATA

  do {
    // Read up to a buffer's worth of decoded sound data
    bytes = ov_read(&ogg_file, array, BUFFER_SIZE, endian, 2, 1, &bit_stream);

    // Append to end of buffer
    buffer.insert(buffer.end(), array, array + bytes);

  } while (bytes > 0);

  data_size= buffer.size();

  buf = new char[data_size];
  memcpy(buf, buffer.data(), buffer.size());

  ov_clear(&ogg_file);
  LOG_DEBUG << "ogg loaded";

  return true;
}

int backend::sound_impl::load_into_al() {
  LOG_DEBUG << "Loading into AL";
  AL_CHECK(alGenBuffers(1, &buffer));  // Generate one OpenAL Buffer and link to "buffer"
  AL_CHECK(alGenSources(1, &source));  // Generate one OpenAL Source and link to "source"
  alBufferData(buffer, format, buf, data_size,
               sample_rate);  // Store the sound data in the OpenAL Buffer
  if (alGetError() != AL_NO_ERROR)
    throw std::runtime_error(
        "Error loading ALBuffer");  // Error during buffer loading
  //AL_CHECK(alBufferi(buffer, AL_CHANNELS, 1));

  // Sound setting variables. This is used for 3D spanning and other effects.
  ALfloat SourcePos[] = {0.0, 0.0, 0.0};    // Position of the source sound
  ALfloat SourceVel[] = {0.0, 0.0, 0.0};    // Velocity of the source sound
  ALfloat ListenerPos[] = {0.0, 0.0, 0.0};  // Position of the listener
  ALfloat ListenerVel[] = {0.0, 0.0, 0.0};  // Velocity of the listener
  ALfloat ListenerOri[] = {0.0, 0.0, -1.0,
                           0.0, 1.0, 0.0};  // Orientation of the listener
  // First direction vector, then vector pointing up)
  // Listener
  AL_CHECK(alListenerfv(AL_POSITION, ListenerPos));     // Set position of the listener
  AL_CHECK(alListenerfv(AL_VELOCITY, ListenerVel));     // Set velocity of the listener
  AL_CHECK(alListenerfv(AL_ORIENTATION, ListenerOri));  // Set orientation of the listener

  // Source
  AL_CHECK(alSourcei(source, AL_BUFFER, buffer));        // Link the buffer to the source
  AL_CHECK(alSourcef(source, AL_PITCH, 1.00f));          // Set the pitch of the source
  AL_CHECK(alSourcef(source, AL_GAIN, 1.00f));           // Set the gain of the source
  AL_CHECK(alSourcefv(source, AL_POSITION, SourcePos));  // Set the position of the source
  AL_CHECK(alSourcefv(source, AL_VELOCITY, SourceVel));  // Set the velocity of the source
  AL_CHECK(alSourcei(source, AL_LOOPING, AL_FALSE));      // Set if source is looping sound
  AL_CHECK(alSourcef(source, AL_MIN_GAIN, 0.00f));
  AL_CHECK(alSourcef(source, AL_MAX_GAIN, 1.00f));

  float globalMaxDistance = 1000;
  float globalRefDistance = 10;
  AL_CHECK(alSourcef(source, AL_REFERENCE_DISTANCE, globalRefDistance));
  AL_CHECK(alSourcef(source, AL_MAX_DISTANCE, globalMaxDistance));
  AL_CHECK(alSourcei(source, AL_SOURCE_RELATIVE, AL_FALSE));  // Already false by default.

  delete[] buf;  // Delete the sound data buffer
  LOG_DEBUG << "Loaded into AL";

  return 1;
}

void backend::sound_impl::gain(float g) {
  AL_CHECK(alSourcef(source, AL_GAIN, 1.00f));
}

void backend::sound_impl::pos(gdt::math::vec3 pos) {
  ALfloat SourcePos[] = {pos.x, pos.y, pos.z};    // Position of the source sound
  AL_CHECK(alSourcefv(source, AL_POSITION, SourcePos));  // Set the position of the source
}

int backend::sound_impl::play() {
  alSourcePlay(source);
  int error = alGetError();  // Play the sound buffer linked to the source
  if (error != AL_NO_ERROR) return error;  // Error when playing sound

  is_playing = true;
  return AL_NO_ERROR;
}

int backend::sound_impl::stop() {
  // PLAY
  alSourceStop(source);
  int error = alGetError();  // Play the sound buffer linked to the source
  if (error != AL_NO_ERROR) return error;  // Error when playing sound

  is_playing = false;
  return AL_NO_ERROR;
}

void backend::sound_impl::shutdown() {
  alDeleteSources(1, &source);  // Delete the OpenAL Source
  alDeleteBuffers(1, &buffer);  // Delete the OpenAL Buffer
}

int backend::init_open_al() {
  device = alcOpenDevice(NULL);  // Open the device
  if (!device)
    throw std::runtime_error("no sound device");  // Error during device opening

  context = alcCreateContext(device, NULL);  // Give the device a context
  alcMakeContextCurrent(context);            // Make the context the current
  if (!context)
    throw std::runtime_error("no sound context");  // Error during context handeling
  alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
  return 0;
}

int backend::add_sound(string path) {
  LOG_DEBUG << "adding sound " << path;
  _sounds.push_back(std::make_unique<sound_impl>(path));
//  _sounds.push_back(new sound(path));
  LOG_DEBUG << "added sound " << path;
  if (_sounds.back()->invalid_file)
    return -1;
  else
    return _sounds.size() - 1;
}

void backend::remove_sound(string path) {
  int i = find_sound(path);
  _sounds[i]->shutdown();
  _sounds.erase(_sounds.begin() + i);
}

void backend::play_sound(string path) {
  LOG_DEBUG << "finding sound " << path;
  int i = find_sound(path);
  LOG_DEBUG << "finding sound " << i;
  if (i == -1) return;
  if (!_sounds[i]->invalid_file) _sounds[i]->play();
}

void backend::stop_sound(string path) {
  int i = find_sound(path);
  if (i == -1) return;
  if (!_sounds[i]->invalid_file) _sounds[i]->stop();
}

int backend::find_sound(string path) {
  for (unsigned int i = 0; i < _sounds.size(); i++)
    if (_sounds[i]->filepath == path) return i;

  cout << "File not loaded: " << path << endl;
  return -1;
}

bool backend::is_playing(string path) {
  int i = find_sound(path);
  if (i == -1) return false;
  return _sounds[i]->is_playing;
}

void backend::shutdown() {
  alcMakeContextCurrent(NULL);  // Make no context current
  alcDestroyContext(context);   // Destroy the OpenAL Context
  alcCloseDevice(device);       // Close the OpenAL Device
}





openal_sound::openal_sound(const gdt::audio_context<backend>& ctx, std::string filename)
{
    _backend = ctx.audio;
    _filename = filename;
    _backend->add_sound(_filename);
}

void openal_sound::play() const // override
{
    _backend->play_sound(_filename);
};

void openal_sound::stop() const // override
{
    _backend->stop_sound(_filename);
};

bool openal_sound::is_playing() const //override
{
    return _backend->is_playing(_filename);
};

void openal_sound::volume(float vol)
{
}

void openal_sound::position(math::vec3 pos)
{
}

}
