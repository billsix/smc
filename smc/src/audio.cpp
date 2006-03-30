/***************************************************************************
           audio.cpp  -  Audio Engine
                             -------------------
    copyright            : (C) 2003 - 2006 by FluXy
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#include "audio.h"


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

bool cAudio_Sound :: Load( string filename )
{
	Free();
	
	chunk = Mix_LoadWAV( filename.c_str() );

	return 1;
}

void cAudio_Sound :: Free( void )
{
	if( chunk )
	{
		Mix_FreeChunk( chunk );
		chunk = NULL;
	}
}

int cAudio_Sound :: Play( int use_res_id /* = -1 */ )
{
	if( !chunk )
	{
		return 0;
	}

	if( use_res_id > 0 )
	{
		for( unsigned int i = 0; i < pAudio->sounds.size(); i++ )
		{
			if( pAudio->sounds[i] == this )
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

	channel = Mix_PlayChannel( -1, chunk, 0 );
	return channel;
}

void cAudio_Sound :: Stop( void )
{
	if( !chunk || channel < 0 )
	{
		return;
	}
	
	Mix_HaltChannel( channel );
}

cAudio :: cAudio( void )
{
	music = NULL;
	
	sound_volume = 110;
	music_volume = 128;
	sounds_played = 0;
	music_played = 0;
	sound_enabled = 0;
	music_enabled = 0;
	debug = 0;
	initialised = 0;

	sound_count = 0;

	audio_hz = 44800; // default Ogg Samples ( never change this )
	audio_buffer = 4096; // below 2000 can be choppy
	audio_channels = MIX_DEFAULT_CHANNELS; // 1 = Mono, 2 = Stereo

	max_sounds = 0;
	// create sound array
	Set_Max_Sounds();
}

cAudio :: ~cAudio( void )
{
	Close();
}

bool cAudio :: Init( void )
{
	if( initialised || ( !music_enabled && !sound_enabled ) )
	{
		return 0;
	}

	if( debug )
	{
		printf( "Initialising Audio System ,Buffer (%i), Frequenzy (%i)\n", audio_buffer, audio_hz );
	}

	/*	Initialising prefered Audio System specs with Mixer Standard format (Stereo)
	*
	*	format		: Output sample format.
	*	channels	: Number of sound channels in output.
	*	Set this to 2 for stereo, 1 for mono.
	*	chunksize	: Bytes used per output sample.
	*/

	if( Mix_OpenAudio( audio_hz, MIX_DEFAULT_FORMAT , audio_channels, audio_buffer ) < 0 ) 
	{
		printf( "Warning : Could not init 16-bit Audio\n- Reason : %s\n", SDL_GetError() );
		return 0;
	}

	initialised = 1;
	SetMusicVolume( music_volume );
	SetSoundVolume( sound_volume );

	return 1;
}

void cAudio :: Close( void )
{
	if( initialised )
	{
		if( sound_enabled )
		{
			StopSounds();

			for( unsigned int i = 0; i < sounds.size(); i++ )
			{
				delete sounds[i];
			}

			sounds.clear();
		}

		if( music_enabled )
		{
			Mix_FreeMusic( music );
			music = NULL;
		}

		Mix_CloseAudio();

		initialised = 0;
	}
}

void cAudio :: Set_Max_Sounds( unsigned int limit /* = 15 */ )
{
	if( limit < 5 )
	{
		return;
	}

	// difference between current and given sound limit
	int diff = limit - max_sounds;

	// undo the difference
	while( diff != 0 )
	{
		// delete 
		if( diff > 0 )
		{
			sounds.push_back( ( new cAudio_Sound() ) );
			diff--;
		}
		// create
		else if( diff < 0 )
		{
			delete sounds[sounds.size()];
			sounds.erase( sounds.begin() + sounds.size() );
			diff++;
		}
	}

	max_sounds = limit;
}

int cAudio :: PlaySound( string filename, int res_id /* = -1 */, int volume /* = -1 */ )
{
	if( !sound_enabled || !initialised )
	{
		return 0;
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
	
	// load the given sound
	if( !sounds[sound_count]->Load( filename ) )
	{
		printf( "Couldn't load sound file : %s\n", filename.c_str() );
		return 0;
	}

	int channel_used = sounds[sound_count]->Play( res_id );

	if( channel_used == -1 )
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
		if( volume != -1 )
		{
			// if given value is out of range use default
			if( volume > 128 || volume < 0 )
			{
				printf( "PlaySound Volume out is of range : %d\n", volume );
				volume = sound_volume;
			}

			Mix_Volume( channel_used, volume );
		}
		// if no volume is given use default
		else
		{
			Mix_Volume( channel_used, sound_volume );
		}
	}

	return 1;
}

int cAudio :: PlayMusic( string filename, int loops /* = 0 */, bool force /* = 1 */, unsigned int fadein_ms /* = 0 */ )
{
	if( !music_enabled || !initialised )
	{
		return 0;
	}

	if( !valid_file( filename ) )
	{
		printf( "Couldn't find music file : %s\n", filename.c_str() );
		return 0;
	}

	Resume_Music();

	if( !isMusicPlaying() || force ) 
	{
		if( music )
		{
			if( force )
			{
				HaltMusic();
			}

			Mix_FreeMusic( music );
			music = NULL;
		}

		music = Mix_LoadMUS( filename.c_str() );

		if( music )
		{
			music_played++;
			
			if( !fadein_ms ) // No Fading
			{
				Mix_PlayMusic( music, loops );
			}
			else // Fade the Music in
			{
				Mix_FadeInMusic( music, loops, fadein_ms );
			}

		}
		else 
		{
			if( debug )
			{
				printf( "PlayMusic couldn't play music file : %s\n", filename.c_str() );
			}

			return 0;
		}
	}
	else
	{
		music = Mix_LoadMUS( filename.c_str() );
	}
	
	return 1;
}


void cAudio :: ToggleMusic( void )
{
	music_enabled = !music_enabled;

	if( !music_enabled ) // stop music
	{
		HaltMusic();
	}
	else // play music
	{
		Init();
		
		if( pLevel->valid_music ) // valid level music
		{
			PlayMusic( pLevel->musicfile, -1, 1, 2000 );
		}
		else if( Game_Mode == MODE_OVERWORLD ) // if in overworld
		{
			pAudio->PlayMusic( MUSIC_DIR "/overworld/land_1.ogg", -1, 1, 2000 );
		}
		else // menu
		{
			PlayMusic( MUSIC_DIR "/game/menu.ogg", -1, 1, 2000 );
		}
	}
}

void cAudio :: ToggleSounds( void )
{
	sound_enabled = !sound_enabled;

	if( !sound_enabled ) // stop sounds if disabled
	{
		StopSounds();
	}
	else // init and play a test sound
	{
		Init();
		PlaySound( SOUNDS_DIR "/audio_on.ogg" );
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

	if( Mix_PausedMusic() ) // Check if music is currently paused
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

void cAudio :: SetMusicPosition( double position )
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

	// Check all Channels
	if( Mix_Playing( -1 ) )
	{
		Mix_HaltChannel( -1 );

		for( unsigned int i = 0; i < sounds.size(); i++ )
		{
			sounds[i]->Free();
		}
	}
}

void cAudio :: SetSoundVolume( Uint8 volume, int channel /* = -1 */ )
{
	if( ( volume > 128 ) || !initialised )
	{
		return;
	}

	Mix_Volume( channel, volume );

	// retrieve the current volume
	sound_volume = Mix_Volume( channel, -1 );
}

void cAudio :: SetMusicVolume( Uint8 volume )
{
	if( ( volume > 128 ) || !initialised )
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
		}
	}
}
