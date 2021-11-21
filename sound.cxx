//------------------------------------------------------------------------------
// File: sound.cxx
// Written by: Michael Main (main@colorado.edu)
// 
// This is a simple program to play a particular kind of .wav file. The file
// must have these properties:
//   -- it is in the .wav format
//   -- it has two sound channels (stereo)
//   -- it has 16-bit amplitude samples
// It's easy to create a file of this sort from other music formats at the
// online site http://media.io
//
// For educational purposes, you may also download the sample wav file that
// we've been using in class (contact Michael for download instructions).
//
// Compilation of this program must include the bgi library and winmm:
//   bgi++ player.cxx -lwinmm -o player
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Directives and global named constants:
#include <algorithm>   // Provides fill_n and copy
#include <cassert>     // Provides the assert function
#include <cctype>      // Provides toupper
#include <cstdlib>     // Provides size_t and EXIT_SUCCESS
#include <fstream>     // Provides ifstream
#include <iostream>    // Provides cin and cout
#include <string>      // Provides the string class
#include <windows.h>   // Provides winmm functions
#include "graphics.h"  // Provides winbgim graphics
using namespace std;

const int S = 300;     // Size of a graphics window (width and height)
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This definition is for a new class called sound that contains the information
// from a .wav file. A programmer may use these operations on a sound object:
/*
   sound s;             // Define a sound variable called s.
   s.load(filename);    // Loads a .wav file into s.
   s.start_play( );     // Start playing s
   s.stop_play( );      // Stop playing s
   s.change_speed(1.1); // Increase playing speed by 10% 
   s.change_speed(0.9); // Decrease playing speed by 10% 
   s.reverse( );        // Reverse the sound
   s.modify( );         // See my comment below!

   The modify function changes the amplitude data in a song by calling
   the student-written function modify_data. There are several possible
   implementations of the modify function included below, but perhaps
   you can come up with your own idea!

   A programmer may also obtain information about the sound s, using these
   functions:

   s.is_loaded( )       // True if a .wav file was successfully loaded into s.
   s.sample_number( )   // Returns the number of the sample that s is currently
                        // playing (or returns 0 if s is not playing).

   Two other functions return information about the volume of the sound
   that is currently playing. Each of these functions requires a parameter
   that can be the constant LEFT_TRACK, RIGHT_TRACK or BOTH_TRACKS. The
   answer that each function returns ranges from 0.0 (lowest volume) to
   1.0 (highest volume).
   s.volume_now(int track)     // Returns the volume right now
   s.volume_forward(int track) // Returns average volume over the next 0.25 sec/
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
    ~sound() { destroy( ); }

    void change_speed(float factor);
    bool is_loaded( );
    void load(std::string filename);
    void modify( );
    void reverse( );
    size_t sample_number();
    void start_play();
    void stop_play();
    float volume_forward(int track);
    float volume_now(int track);

private:
    void destroy( );       // Unloads the wave file (if one is loaded)
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
    short *data;
    float *left_volume, *right_volume;
    float *left_av, *right_av;
};
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Function prototypes
// void compute_averages(float data[], float answer[], size_t n, size_t width);
// Precondition: Both data and answer are arrays with n components.
// Postcondition: This function sets each component answer[i] to the average of
// the numbers in data[i], data[i+1], data[i+2], ..., data[i+width-1].
// Note: This formula sometimes accesses elements beyond the end of the
// data array. To avoid this problem, this function uses the number 0.0 for
// each of those out-of-bounds components.
void compute_averages(float data[], float answer[], size_t n, size_t width);


// void modify_data(short data[], size_t n);
// The data in the array is the amplitude data for a song. This function
// modifies the data in some way that a CSCI 1300 student decides upon!
void modify_data(short data[], size_t n);

// void reverse_array(int data[], size_t n);
// Precondition: data is an array with n components.
// Postcondition: The data array contains the same numbers, but the order of
// those numbers has been reversed.
void reverse_array(int data[], size_t n);

// float(x) returns x*x.
float sqr(float x);

// void user_adjust(sound& s);
// This function changes the sound according to user keyboard input.
void user_adjust(sound& s);
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
int main( )
{
    string filename; // Name of the wave file
    sound s;         // The sound object
    size_t sn;       // Which sample number is now playing?

    // Get the .wav file:
    cout << endl << "To begin, please type the name of a wave file: ";
    cin >> filename;
    s.load(filename);
    if (!s.is_loaded( ))
    {
	cout << "File not in directory (or it's the wrong format)." << endl;
    }

    // Open a small graphics window and play the music:
    user_adjust(s);
    initwindow(S, S, "Dancing Man!", 0, 0, true);
    s.start_play( );

    do
    {
	// 1. Do computations
	sn =  s.sample_number();

	// 2. Student should remove this code and do an animation instead!
	cout << "Sample Number = " << sn << endl
	     << "Left Volume = " << s.volume_now(sound::LEFT_TRACK) << " "
	     << "Left Future = " << s.volume_forward(sound::LEFT_TRACK) << endl
	     << "Right Volume = " << s.volume_now(sound::RIGHT_TRACK) << " "
	     << "Right Future = " << s.volume_forward(sound::RIGHT_TRACK) << endl
	     << endl;

	// 3. Swap buffers and delay
	swapbuffers( );
	delay(10);
    }   while (sn > 0);
    
    cout << "Done Playing" << endl;
    return EXIT_SUCCESS;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
const int sound::LEFT_TRACK;
const int sound::BOTH_TRACKS;
const int sound::RIGHT_TRACK;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
sound::sound( )
{
    handle = NULL;
    many_samples = 0;
    fill_n((char *)(&head), sizeof(WAVEHDR), 0);
    fill_n((char *)(&format), sizeof(WAVEFORMATEX), 0);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
sound::sound(const sound& source)
{
    handle = NULL;
    many_samples = 0;
    fill_n((char *)(&head), sizeof(WAVEHDR), 0);
    fill_n((char *)(&format), sizeof(WAVEFORMATEX), 0);
    *this = source;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
sound& sound::operator =(const sound& source)
{
    if (this == &source) return *this;
    destroy( );

    many_samples = source.many_samples;
    head = source.head;
    format = source.format;
    handle = NULL;
    if (is_loaded( ))
    {
	head.lpData = new char[head.dwBufferLength];
	copy(
	    source.head.lpData,
	    source.head.lpData+head.dwBufferLength,
	    head.lpData
	    );
	make_arrays( );
    }

    return *this;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void sound::change_speed(float factor)
{
    if (is_loaded())
    {
	format.nSamplesPerSec =
            static_cast<int>(factor*format.nSamplesPerSec);
	delete [] left_volume;
	delete [] right_volume;
	delete [] left_av;
	delete [] right_av;
	make_arrays( );
    }
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void sound::destroy( )
{
    stop_play( );
    if (is_loaded( ))
    {
	delete [] head.lpData;
	delete [] left_volume;
	delete [] right_volume;
	delete [] left_av;
	delete [] right_av;
	many_samples = 0;
    }
    fill_n((char *)(&head), sizeof(WAVEHDR), 0);
    fill_n((char *)(&format), sizeof(WAVEFORMATEX), 0);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
bool sound::is_loaded( )
{
    return (many_samples > 0);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void sound::load(string filename)
{
    ifstream bin;           // Input .wav file 
    char xxxx[5] = "XXXX";  // Four characters read from the .wave file's head
    int i_number = 0;       // A four-byte integer read from the wave file
    short int s_number = 0; // A two-byte short integer read from the wave file

    // Destroy any old data and open the new file
    destroy( );
    bin.open(filename.c_str(), ios::in|ios::binary);
    if (!bin) return; // File not found

    // See http://www.signalogic.com/index.pl?page=ms_waveform for format.
    // Read information to set the sound's format:
    bin.read(xxxx, 4);
    if (strcmp(xxxx, "RIFF")) return;
    bin.read(reinterpret_cast<char*>(&i_number), 4); // discard
    bin.read(xxxx, 4);
    if (strcmp(xxxx, "WAVE")) return;
    bin.read(xxxx, 4);
    if (strcmp(xxxx, "fmt ")) return;
    bin.read(reinterpret_cast<char*>(&i_number), 4); // discard
    bin.read(reinterpret_cast<char*>(&s_number), 2); // Data format
    if (s_number != WAVE_FORMAT_PCM) return;
    format.wFormatTag = s_number;
    bin.read(reinterpret_cast<char*>(&s_number), 2); // Number of channels
    if (s_number != 2) return;
    format.nChannels = s_number;
    bin.read(reinterpret_cast<char*>(&i_number), 4); // Sampling freq in hz
    format.nSamplesPerSec = i_number;
    bin.read(reinterpret_cast<char*>(&i_number), 4); // Bytes per second
    format.nAvgBytesPerSec = i_number;
    bin.read(reinterpret_cast<char*>(&s_number), 2); // Block alignment
    format.nBlockAlign = s_number;
    bin.read(reinterpret_cast<char*>(&s_number), 2); // Bits per sample
    if (s_number != 16) return;
    format.wBitsPerSample = s_number;
    format.cbSize = 0;
    
    // Read the rest of the file and set the sound's data:
    bin.read(xxxx, 4);
    if (strcmp(xxxx, "data")) return;
    bin.read(reinterpret_cast<char*>(&i_number), 4); // Length of data section
    if (i_number == 0) return;
    head.dwBufferLength = i_number;
    head.lpData = new char[i_number];
    bin.read(head.lpData, i_number);

    // Close input file and set members that indicate a successful load:
    bin.close( );
    many_samples = head.dwBufferLength / 4; // (4 = 2bytes*2channels)
    make_arrays( );
	
    return;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void sound::make_arrays( )
{
    const float MAX_AMPLITUDE = 32767.0;
    size_t width = format.nSamplesPerSec/10;
    size_t i;

    data = reinterpret_cast<short*>(head.lpData);
    left_volume = new float[many_samples];
    right_volume = new float[many_samples];
    left_av = new float[many_samples];
    right_av = new float[many_samples];
    
    for (i = 0; i < many_samples; ++i)
    {
	left_volume[i] = sqr(data[2*i]/MAX_AMPLITUDE);
	right_volume[i] = sqr(data[2*i + 1]/MAX_AMPLITUDE);
    }
    compute_averages(left_volume, left_av, many_samples, width);
    compute_averages(right_volume, right_av, many_samples, width);
}
//------------------------------------------------------------------------------
    
//------------------------------------------------------------------------------
void sound::reverse( )
{
    if (is_loaded())
    {
	reverse_array(reinterpret_cast<int*>(data), many_samples);
	reverse_array(reinterpret_cast<int*>(left_volume), many_samples);
	reverse_array(reinterpret_cast<int*>(right_volume), many_samples);
	reverse_array(reinterpret_cast<int*>(left_av), many_samples);
	reverse_array(reinterpret_cast<int*>(right_av), many_samples);
    }
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void sound::modify( )
{
    if (is_loaded())
    {
	modify_data(data, many_samples);
	delete [] left_volume;
	delete [] right_volume;
	delete [] left_av;
	delete [] right_av;
	make_arrays( );
    }
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
size_t sound::sample_number( )
{
    MMTIME time;

    if (handle)
    {
	time.wType = TIME_SAMPLES;
	waveOutGetPosition(handle, &time, sizeof(MMTIME));
	if (time.u.sample+1 < many_samples)
	{
	    return time.u.sample+1;
	}
	stop_play( );
    }
    return 0;
}
//------------------------------------------------------------------------------
    
//------------------------------------------------------------------------------
void sound::start_play()
{
    waveOutOpen(&handle, WAVE_MAPPER, &format, 0, 0, 0);
    waveOutPrepareHeader(handle, &head, sizeof(WAVEHDR));
    waveOutWrite(handle, &head, sizeof(WAVEHDR));
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void sound::stop_play()
{
    if (handle)
    {
	waveOutClose(handle);
	handle = NULL;
    }
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
float sound::volume_now(int track)
{
    size_t sn = sample_number();
    
    if (sn == 0)
	return 0;
    else
    {
	switch(track)
	{
	case LEFT_TRACK:
	    return left_volume[sn];
	case RIGHT_TRACK:
	    return right_volume[sn];
	case BOTH_TRACKS:
	    return (left_volume[sn] + right_volume[sn])/2;
	default:
	    return 0;
	}
    }
}
//------------------------------------------------------------------------------
    
//------------------------------------------------------------------------------
float sound::volume_forward(int track)
{
    size_t sn = sample_number();
    if (sn == 0)
	return 0;
    else
    {
	switch(track)
	{
	case LEFT_TRACK:
	    return left_av[sn];
	case RIGHT_TRACK:
	    return right_av[sn];
	case BOTH_TRACKS:
	    return (left_av[sn] + right_av[sn])/2;
	default:
	    return 0;
	}
    }
}
//------------------------------------------------------------------------------
    
//------------------------------------------------------------------------------
float sqr(float x)
{
    return x*x;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void compute_averages(float data[], float answer[], size_t n, size_t width)
{
    cout << "WARNING! You have not yet written the compute_averages function."
	 << endl;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This version of modify_data sets the new amplitude equal to the sum of:
//   -- half of the current amplitude
//   plus
//   -- half of the amplitude from 300000 samples in the future.
// I thought that this would give a kind of echo effect.
void modify_data(short data[], size_t n)
{
    size_t i;

    for (i = 0; i < n-300000; ++i)
    {
	data[i] = data[i]/2 + data[i+300000]/2;
    }
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void reverse_array(int data[], size_t n)
{
    cout << "WARNING! You have not yet written the reverse_array function."
	 << endl;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void user_adjust(sound& s)
{
    char command;
    bool is_reversed = false;
    float speed = 1.0;
    
    cout << "Press + (to speed up), - (to slow down) or R (to reverse)." << endl;
    cout << "If you have implemented an interesting modify_data function," << endl;
    cout << "Then you may also press M to modify the amplitude data!" << endl;
    cout << "Press the enter key after each command." << endl;
    cout << "When you're done adjusting things, please press P to play." << endl;

    do
    {
	cin >> command;
	command = toupper(command);
	switch (command)
	{
	case '+':
	    s.change_speed(1.1);
	    speed *= 1.1;
	    cout << "Speed is now " << speed << endl;
	    break;
	case '-':
	    s.change_speed(1/1.1);
	    speed /= 1.1;
	    cout << "Speed is now " << speed << endl;
	    break;
	case 'R':
	    s.reverse( );
	    is_reversed = !is_reversed;
	    if (is_reversed)
	    {
		cout << "Direction is reversed." << endl;
	    }
	    else
	    {
		cout << "Direction is forward." << endl;
	    }
	    break;
	case 'M':
	    s.modify( );
	    cout << "Your modify function has been run!" << endl;
	    break;
	}
    }   while (command != 'P');    
}
//------------------------------------------------------------------------------
