//------------------------------------------------------------------------------
// File: sound.h
// Written by: Michael Main (main@colorado.edu)
// 
// Note:
// When compiling with the sound class, you must link to the implementation
// of the class (in sound.cxx) and link to the winmm library with the
// linker option -lwinmm.
//
// This header file provides the definition for a new class called sound
// in the standard namespace. A sound object contains information from a
// .wav file. A programmer may use these operations on a sound object:
/*
   sound s;             // Define a sound variable called s.
   s.load(filename);    // Loads a .wav file into s.
                        // Note: Currently must be a stereo wav file with
			// 16-bit amplitudes. This may be expanded to other
			// kinds of files in future versions.
   s.start_play( );     // Start playing s
   s.stop_play( );      // Stop playing s

   A programmer may also obtain information about the sound s, using these
   functions:

   s.is_loaded( )       // True if a .wav file was successfully loaded into s.
   s.sample_number( )   // Returns the number of the sample that s is currently
                        // playing (or returns 0 if s is not playing).

   Two other functions return information about the sound
   that is currently playing. Each of these functions requires a parameter
   that can be the constant LEFT_TRACK, RIGHT_TRACK or BOTH_TRACKS. The
   volume function return values that range from 0.0 (lowest volume) to
   1.0 (highest volume). The frequency function returns its answer in Hertz.
   s.frequency(int track)  // Dominant frequency over the next 0.25 sec
   s.volume(int track)     // Average volume over the next 0.25 sec
*/
//------------------------------------------------------------------------------

class sound
{
public:
    static const int LEFT_TRACK = -1;
    static const int BOTH_TRACKS = 0;
    static const int RIGHT_TRACK = +1;
	
    sound( );
    sound(const sound& source);
    sound& operator =(const sound& source);
    ~sound() { unload( ); }

    void load(std::string filename);
    void unload( );  // Unloads the wave file (if one is loaded)

    bool is_loaded( ) const;
    size_t sample_number() const;
    void start_play() const;
    void stop_play() const;
    float frequency(int track) const;
    float volume(int track) const;

private:
    void make_arrays( );   // Set all the secondary arrays

    // The primary member variables. The actual wave amplitude data is
    // stored in format.lpData, but it should not be accessed directly.
    // Instead, use the secondary arrays listed below.
    size_t many_samples;   // Number of samples (or 0 if no file is loaded)
    WAVEFORMATEX format;   // Information such as samples/second
    WAVEHDR head;          // Mostly the wave file's data
    HWAVEOUT handle;       // Non-null windows media handle when playing

    // Secondary arrays. The data in these arrays is derived from the
    // primary member variables. The values of these arrays are:
    // 1. data[2*i] is the amplitude of the left channel of sample i,
    //    and data[2*i+1] is the amplitude of the right channel of sample i.
    // 2. left_volume[i] is the volume (from 0.0 to 1.0) of the left channel
    //    of sample i, and similarly for right_volume[i].
    // 3. left_av[i] is the average volume of the left channel over
    //    the samples numbered from i to i+width-1, where width is
    //    defined to be the number of samples in 0.25 seconds, and
    //    similarly for right_av[i].
    // 4. left_freq[i] is the dominant frequency of the left channel over
    //    the samples numbered from i to i+width-1, where width is
    //    defined to be the number of samples in 0.25 seconds, and
    //    similarly for right_freq[i].
    short *data;
    float *left_volume, *right_volume;
    float *left_av, *right_av;
    float *left_freq, *right_freq;
};
