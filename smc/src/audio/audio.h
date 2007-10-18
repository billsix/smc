/***************************************************************************
			audio.h  -  header for the corresponding cpp file
                         -------------------
    copyright            : (C) 2003 - 2007 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __AUDIO_H__
#define __AUDIO_H__

#include "../core/globals.h"

/* *** *** *** *** *** *** *** Sound Resource ID's  *** *** *** *** *** *** *** *** *** *** */

// ingame sounds which shouldn't be played twice at the same time
enum AudioChannel
{
	RID_MARYO_JUMP		= 1,
	RID_MARYO_TOCK		= 2,
	RID_MARYO_POWERDOWN = 3,
	RID_MARYO_DEATH		= 5,
	RID_MARYO_BALL		= 4,
	RID_MARYO_AU		= 8,
	RID_MARYO_STOP		= 9,

	RID_FIREPLANT		= 6,
	RID_MUSHROOM_BLUE	= 6,
	RID_MUSHROOM_GHOST	= 6,
	RID_MUSHROOM		= 6,
	RID_FEATHER			= 6,
	RID_1UP_MUSHROOM	= 7,
	RID_MOON			= 7
};

/* *** *** *** *** *** *** *** Sound object *** *** *** *** *** *** *** *** *** *** */
	
// Callback for a sound finished playing 
void Finished_Sound( int channel );

class cAudio_Sound
{
public:
	cAudio_Sound( void );
	~cAudio_Sound( void );
	
	// Load the data
	bool Load( string nfilename );
	// Free the data
	void Free( void );
	// Finished playing
	void Finished( void );

	/* Play the Sound
	 * if resource id is given stops all sounds using the same resource id
	*/
	int Play( int use_res_id = -1 );
	// Stop the Sound if playing
	void Stop( void );

	// sound filename
	string filename;
	// sound data if loaded else null
	Mix_Chunk *chunk;
	// channel if playing else -1
	int channel;

	// the last used resource id
	int resource_id;
};

typedef vector<cAudio_Sound *> SoundList;

/* *** *** *** *** *** *** *** Audio class *** *** *** *** *** *** *** *** *** *** */

class cAudio
{
public:
	cAudio( void );
	~cAudio( void );

	// Initializes the Audio Engine
	bool Init( bool sound = 1, bool music = 1 );
	// De-initializes the Audio Engine
	void Close( void );

	// Sets the maximum of sounds at once playable
	void Set_Max_Sounds( unsigned int limit = 10 );

	/* Checks if the sound was already loaded and returns a pointer to it else it will be loaded
	 * The returned sound should not be deleted or modified.
	 */
	cAudio_Sound *Get_Sound( string filename );

	// Plays the given sound
	bool PlaySound( string filename, int res_id = -1, int volume = -1 );
	// If no forcing it will be played after the current Music file
	bool PlayMusic( string filename, int loops = 0, bool force = 1, unsigned int fadein_ms = 0 ); 


	// Switches Music on and off
	void ToggleMusic( void );
	// Switches Sounds on and off
	void ToggleSounds( void );

	// Pause Music
	void PauseMusic( void );

	/* Resume halted sound
	 * if channel is -1 all halted sounds will be resumed
	*/
	void Resume_Sound( int channel = -1 );
	// Resumes Music
	void Resume_Music( void );


	// Fades the given Sound channels with the given time out
	void FadeOutSounds( unsigned int ms  = 200, int channel = -1, bool overwrite_fading = 0 );
	// Fades the Music with the given time out
	void FadeOutMusic( unsigned int ms = 500, bool overwrite_fading = 0 );

	// Sets the Music position ( if .ogg in seconds )
	void SetMusicPosition( float position );


	// Returns 1 if the Music is currently fading in and 2 if it's fading out else 0
	Mix_Fading isMusicFading( void );
	// Returns 1 if the Sound is currently fading in and 2 if it's fading out else 0
	Mix_Fading isSoundFading( int sound_channel );


	// Returns true if the Music is paused
	bool isMusicPaused( void );
	// Returns true if the Music is playing
	bool isMusicPlaying( void );

	// Halts the given sounds
	void HaltSounds( int channel = -1 );
	// halts the Music
	void HaltMusic( void );

	// Stops all sounds
	void StopSounds( void ); 

	// Sets the Sound Volume
	void SetSoundVolume( Uint8 volume, int channel = -1 );
	// Sets the Music Volume
	void SetMusicVolume( Uint8 volume );

	// Updates the Audio Engine
	void Update( void );

	// is the audio engine initialized
	bool initialised;
	// is sound enabled
	bool sound_enabled;
	// is music enabled
	bool music_enabled;
	// is the debug mode enabled
	bool debug;

	// current music and sound volume
	Uint8 sound_volume, music_volume;

	// current playing Music pointer
	Mix_Music *music;
	// if new music should play after the current this is the old data
	Mix_Music *music_old;

	// The current sounds pointer array
	SoundList sounds;

	// maximum sounds allowed at once
	unsigned int max_sounds;
	// last used sound array buffer
	unsigned int sound_count;

	// initialization information
	int audio_buffer, audio_channels;
	// sound and music played counter since initialization
	unsigned int sounds_played, music_played;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Audio Handler
extern cAudio *pAudio;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
