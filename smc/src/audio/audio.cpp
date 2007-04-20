/***************************************************************************
           audio.cpp  -  Audio Engine
                             -------------------
    copyright            : (C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#include "../audio/audio.h"
#include "../audio/sound_manager.h"
#include "../core/game_core.h"
#include "../level/level.h"
#include "../overworld/overworld.h"

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void Finished_Sound( int channel )
{
	// find the finished sound and free the data
	for( unsigned int i = 0; i < pAudio->sounds.size(); i++ )
	{
		if( !pAudio->sounds[i] )
		{
			continue;
		}

		if( pAudio->sounds[i]->channel == channel )
		{
			pAudio->sounds[i]->Finished();
		}
	}
}

/* *** *** *** *** *** *** *** *** Audio Sound *** *** *** *** *** *** *** *** *** */

cAudio_Sound :: cAudio_Sound( void )
{
	chunk = NULL;
	channel = -1;
	resource_id = -1;
}

cAudio_Sound :: ~cAudio_Sound( void )
{
	Free();
}

bool cAudio_Sound :: Load( string nfilename )
{
	Free();
	
	chunk = Mix_LoadWAV( nfilename.c_str() );

	if( chunk )
	{
		filename = nfilename;
		return 1;
	}
	
	return 0;
}

void cAudio_Sound :: Free( void )
{
	if( chunk )
	{
		Mix_FreeChunk( chunk );
		chunk = NULL;
	}
	
	channel = -1;
	resource_id = -1;
	filename.clear();
}

void cAudio_Sound :: Finished( void )
{
	channel = -1;
}

int cAudio_Sound :: Play( int use_res_id /* = -1 */ )
{
	if( !chunk )
	{
		return 0;
	}

	if( use_res_id >= 0 )
	{
		for( unsigned int i = 0; i < pAudio->sounds.size(); i++ )
		{
			if( !pAudio->sounds[i] || pAudio->sounds[i] == this )
			{
				continue;
			}

			// stop Sounds using the given resource id
			if( pAudio->sounds[i]->resource_id == use_res_id )
			{
				pAudio->sounds[i]->Stop();
			}
		}
	}

	resource_id = use_res_id;
	// play sound
	channel = Mix_PlayChannel( -1, chunk, 0 );
	// add callback if sound finished playing
	Mix_ChannelFinished( &Finished_Sound );

	return channel;
}

void cAudio_Sound :: Stop( void )
{
	if( !chunk || channel < 0 )
	{
		return;
	}
	
	Mix_HaltChannel( channel );
	channel = -1;
}

/* *** *** *** *** *** *** *** *** Audio *** *** *** *** *** *** *** *** *** */

cAudio :: cAudio( void )
{
	music = NULL;
	music_old = NULL;
	
	sound_volume = MIX_MAX_VOLUME;
	music_volume = MIX_MAX_VOLUME;
	sounds_played = 0;
	music_played = 0;
	initialised = 0;
	sound_enabled = 0;
	music_enabled = 0;

	debug = 0;

	sound_count = 0;

	audio_hz = 44800; // default Ogg Samples ( never change this )
	audio_buffer = 4096; // below 2000 can be choppy
	audio_channels = MIX_DEFAULT_CHANNELS; // 1 = Mono, 2 = Stereo

	max_sounds = 0;
}

cAudio :: ~cAudio( void )
{
	Close();
}

bool cAudio :: Init( bool sound /* = 1 */, bool music /* = 1 */ )
{
	// if no change return
	if( music_enabled == music && sound_enabled == sound )
	{
		return 1;
	}

	// if both disabled - unload
	if( !music && !sound )
	{
		Close();
		return 1;
	}

	// if audio system is not initialised
	if( !initialised )
	{
		if( debug )
		{
			printf( "Initializing Audio System - Buffer %i, Frequency %i, Speaker Channels %i\n", audio_buffer, audio_hz, audio_channels );
		}

		/*	Initializing preferred Audio System specs with Mixer Standard format (Stereo)
		*
		*	frequency	: Output sampling frequency in samples per second (Hz).
		*	format		: Output sample format.
		*	channels	: Number of sound channels in output. 2 for stereo and 1 for mono.
		*	chunk size	: Bytes used per output sample.
		*/

		if( Mix_OpenAudio( audio_hz, MIX_DEFAULT_FORMAT, audio_channels, audio_buffer ) < 0 ) 
		{
			printf( "Warning : Could not init 16-bit Audio\n- Reason : %s\n", SDL_GetError() );
			return 0;
		}

		initialised = 1;
	}


	if( debug )
	{
		printf( "Audio Sound Channels available : %d\n", Mix_AllocateChannels( -1 ) );
	}

	// music initialization
	if( music && !music_enabled )
	{
		music_enabled = 1;

		// set music volume
		SetMusicVolume( music_volume );
	}
	// music de initialization
	else if( !music && music_enabled )
	{
		HaltMusic();

		music_enabled = 0;
	}

	// sound initialization
	if( sound && !sound_enabled )
	{
		sound_enabled = 1;

		// create sound array
		Set_Max_Sounds();
		// set sound volume
		SetSoundVolume( sound_volume );
	}
	// sound de initialization
	else if( !sound && sound_enabled )
	{
		StopSounds();

		sound_enabled = 0;
	}

	return 1;
}

void cAudio :: Close( void )
{
	if( initialised )
	{
		if( debug )
		{
			printf( "Closing Audio System\n" );
		}

		if( sound_enabled )
		{
			StopSounds();

			sounds.clear();

			Mix_AllocateChannels( 0 );
			max_sounds = 0;
			sound_enabled = 0;
		}

		if( music_enabled )
		{
			HaltMusic();

			if( music )
			{
				Mix_FreeMusic( music );
				music = NULL;
			}

			if( music_old )
			{
				Mix_FreeMusic( music_old );
				music_old = NULL;
			}

			music_enabled = 0;
		}

		Mix_CloseAudio();

		initialised = 0;
	}
}

void cAudio :: Set_Max_Sounds( unsigned int limit /* = 10 */ )
{
	if( !initialised || !sound_enabled )
	{
		return;
	}

	// if limit is too small set it to the minimum
	if( limit < 5 )
	{
		limit = 5;
	}

	max_sounds = limit;

	while( sounds.size() < max_sounds )
	{
		sounds.push_back( NULL );
	}

	while( sounds.size() > max_sounds )
	{
		sounds.erase( sounds.end() );
	}

	// change channels managed by the mixer
	Mix_AllocateChannels( max_sounds );

	if( debug )
	{
		printf( "Audio Sound Channels changed : %d\n", Mix_AllocateChannels( -1 ) );
	}
}

cAudio_Sound *cAudio :: Get_Sound( string filename )
{
	if( !initialised || !sound_enabled )
	{
		return NULL;
	}

	if( filename.find( SOUNDS_DIR ) == string::npos )
	{
		filename.insert( 0, SOUNDS_DIR "/" );
	}

	cAudio_Sound *sound = pSoundManager->Get_Pointer( filename );

	// if not already cached
	if( !sound )
	{
		sound = new cAudio_Sound();

		// loaded sound
		if( sound->Load( filename ) )
		{
			pSoundManager->Add( sound );

			if( debug )
			{
				printf( "Loaded sound file : %s\n", filename.c_str() );
			}
		}
		// failed loading
		else
		{
			printf( "Couldn't load sound file : %s \nReason : %s\n", filename.c_str(), SDL_GetError() );
			
			delete sound;
			return NULL;
		}
	}

	return sound;
}

bool cAudio :: PlaySound( string filename, int res_id /* = -1 */, int volume /* = -1 */ )
{
	if( !initialised || !sound_enabled )
	{
		return 0;
	}

	if( filename.find( SOUNDS_DIR ) == string::npos )
	{
		filename.insert( 0, SOUNDS_DIR "/" );
	}

	if( !valid_file( filename ) )
	{
		printf( "Couldn't find sound file : %s\n", filename.c_str() );
		return 0;
	}

	sound_count++;

	if( sound_count >= sounds.size() )
	{
		sound_count = 0;
	}

	// stop old sound
	if( sounds[sound_count] )
	{
		sounds[sound_count]->Stop();
	}

	sounds[sound_count] = pAudio->Get_Sound( filename );

	// failed loading
	if( !sounds[sound_count] )
	{
		return 0;
	}

	sounds[sound_count]->Play( res_id );

	if( sounds[sound_count]->channel < 0 )
	{
		if( debug )
		{
			printf( "Couldn't play sound file : %s\n", filename.c_str() );
		}

		return 0;
	}
	else
	{
		sounds_played++;

		// if volume is given
		if( volume > MIX_MAX_VOLUME || volume < 0 )
		{
			Mix_Volume( sounds[sound_count]->channel, volume );
		}
		// if no volume is given or given value is out of range use default
		else
		{
			printf( "PlaySound Volume out is of range : %d\n", volume );
			Mix_Volume( sounds[sound_count]->channel, sound_volume );
		}
	}

	return 1;
}

bool cAudio :: PlayMusic( string filename, int loops /* = 0 */, bool force /* = 1 */, unsigned int fadein_ms /* = 0 */ )
{
	if( !music_enabled || !initialised )
	{
		return 0;
	}

	if( filename.find( MUSIC_DIR ) == string::npos )
	{
		filename.insert( 0, MUSIC_DIR "/" );
	}

	// no valid file
	if( !valid_file( filename ) )
	{
		printf( "Couldn't find music file : %s\n", filename.c_str() );
		return 0;
	}

	// if music is stopped resume it
	Resume_Music();

	// if no music is playing or force to play the given music
	if( !isMusicPlaying() || force ) 
	{
		// stop and free current music
		if( music )
		{
			HaltMusic();
			Mix_FreeMusic( music );
		}
		// free old music
		if( music_old )
		{
			Mix_FreeMusic( music_old );
			music_old = NULL;
		}

		// load the given music
		music = Mix_LoadMUS( filename.c_str() );

		// loaded
		if( music )
		{
			// count success
			music_played++;

			// no fade in
			if( !fadein_ms )
			{
				Mix_PlayMusic( music, loops );
			}
			// fade in
			else
			{
				Mix_FadeInMusic( music, loops, fadein_ms );
			}
		}
		// not loaded
		else 
		{
			if( debug )
			{
				printf( "Couldn't load music file : %s\n", filename.c_str() );
			}

			// failed to play
			return 0;
		}
	}
	// music is playing and is not forced
	else
	{
		// if music is loaded
		if( music )
		{
			// if old music is loaded free the wanted next playing music data
			if( music_old )
			{
				Mix_FreeMusic( music );
				music = NULL;
			}
			// if no old music move current to old music
			else
			{
				music_old = music;
				music = NULL;
			}
		}

		// load the wanted next playing music
		music = Mix_LoadMUS( filename.c_str() );
	}
	
	return 1;
}


void cAudio :: ToggleMusic( void )
{
	Init( sound_enabled, !music_enabled );

	// play music
	if( music_enabled )
	{
		// valid level music available
		if( pLevel->valid_music )
		{
			PlayMusic( pLevel->musicfile, -1, 1, 2000 );
		}
		// in overworld
		else if( Game_Mode == MODE_OVERWORLD )
		{
			PlayMusic( pActive_Overworld->musicfile, -1, 1, 2000 );
		}
		// in menu
		else
		{
			PlayMusic( "game/menu.ogg", -1, 1, 2000 );
		}

		// Warning if no music pack is installed and music got enabled
		if( !valid_file( MUSIC_DIR "/game/menu.ogg" ) && !valid_file( MUSIC_DIR "/land/land_1.ogg" ) )
		{
			Draw_StaticText( "Warning : No Music Addon detected", &orange, &lightgreyalpha64 );
		}
	}
}

void cAudio :: ToggleSounds( void )
{
	Init( !sound_enabled, music_enabled );

	// play a test sound
	if( sound_enabled )
	{
		PlaySound( "audio_on.ogg" );
	}
}

void cAudio :: PauseMusic( void )
{
	if( !music_enabled || !initialised )
	{
		return;
	}

	if( Mix_PlayingMusic() ) // Check if music is currently playing
	{
		Mix_PauseMusic();
	}
}

void cAudio :: Resume_Sound( int channel /* = -1 */ )
{
	if( !sound_enabled || !initialised )
	{
		return;
	}

	// resume playback on all previously active channels
	Mix_Resume( channel );
}

void cAudio :: Resume_Music( void )
{
	if( !music_enabled || !initialised )
	{
		return;
	}

	// Check if music is currently paused
	if( Mix_PausedMusic() )
	{
		Mix_ResumeMusic();
	}
}

void cAudio :: FadeOutSounds( unsigned int ms /* = 200 */, int channel /* = -1 */, bool overwrite_fading /* = 0 */ )
{
	if( !sound_enabled || !initialised )
	{
		return;
	}

	if( Mix_Playing( channel ) ) // Check the Channels
	{
		// Don't fade out the Sounds again
		if( !overwrite_fading && isSoundFading( -1 ) == MIX_FADING_OUT )
		{
			return;
		}

		Mix_FadeOutChannel( channel, ms );
	}
}

void cAudio :: FadeOutMusic( unsigned int ms /* = 500 */, bool overwrite_fading /* = 0 */ )
{
	if( !music_enabled || !initialised )
	{
		return;
	}

	if( Mix_PlayingMusic() ) // Check if music is currently playing
	{
		Mix_Fading status = isMusicFading();

		// Don't fade the Music out again
		if( !overwrite_fading && status == MIX_FADING_OUT )
		{
			return;
		} 
		// Can't stop fade in with SDL_Mixer and fade out is ignored when fading in
		else if( status == MIX_FADING_IN )
		{
			HaltMusic();
		}

		Mix_FadeOutMusic( ms );
	}
}

void cAudio :: SetMusicPosition( float position )
{	
	if( !music_enabled || !initialised || isMusicFading() == MIX_FADING_OUT )
	{
		return;
	}

	Mix_SetMusicPosition( position );
}

Mix_Fading cAudio :: isMusicFading( void )
{
	if( !music_enabled || !initialised )
	{
		return MIX_NO_FADING;
	}

	return Mix_FadingMusic();
}

Mix_Fading cAudio :: isSoundFading( int sound_channel )
{
	if( !sound_enabled || !initialised )
	{
		return MIX_NO_FADING;
	}

	 return Mix_FadingChannel( sound_channel );
}

bool cAudio :: isMusicPaused( void )
{
	if( !music_enabled || !initialised )
	{
		return 0;
	}
	
	if( Mix_PausedMusic() )
	{
		return 1;
	}
	
	return 0;
}

bool cAudio :: isMusicPlaying( void )
{
	if( !music_enabled || !initialised )
	{
		return 0;
	}
	
	if( Mix_PlayingMusic() )
	{
		return 1;
	}
	
	return 0;
}

void cAudio :: HaltSounds( int channel /* = -1 */ )
{
	if( !sound_enabled || !initialised )
	{
		return;
	}

	// Check all Channels
	if( Mix_Playing( channel ) )
	{
		Mix_HaltChannel( channel );
	}
}

void cAudio :: HaltMusic( void )
{
	if( !initialised )
	{
		return;
	}

	// Checks if music is playing
	if( Mix_PlayingMusic() )
	{
		Mix_HaltMusic();
	}
}

void cAudio :: StopSounds( void )
{
	if( !initialised )
	{
		return;
	}

	// Stop all channels
	if( Mix_Playing( -1 ) )
	{
		Mix_HaltChannel( -1 );
	}
}

void cAudio :: SetSoundVolume( Uint8 volume, int channel /* = -1 */ )
{
	if( volume > MIX_MAX_VOLUME || !initialised )
	{
		return;
	}

	Mix_Volume( channel, volume );

	// retrieve the current volume
	sound_volume = Mix_Volume( channel, -1 );
}

void cAudio :: SetMusicVolume( Uint8 volume )
{
	if( volume > MIX_MAX_VOLUME || !initialised )
	{
		return;
	}

	Mix_VolumeMusic( volume );

	music_volume = volume;
}

void cAudio :: Update( void )
{
	if( !initialised )
	{
		return;
	}

	// if music is enabled
	if( music_enabled )
	{
		// if no music is playing
		if( !Mix_PlayingMusic() && music ) 
		{
			Mix_PlayMusic( music, 0 );
			music_played++;

			// delete old music if available
			if( music_old )
			{
				Mix_FreeMusic( music_old );
				music_old = NULL;
			}
		}
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cAudio *pAudio = NULL;
