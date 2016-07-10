/* This file is part of NOWELL.
 *
 * NOWELL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * NOWELL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NOWELL.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "common.h"
#include "oggplayer.h"
#include <fat.h>
#include <fcntl.h>
#include <gccore.h>
#include <math.h>
#include <mp3player.h>
#include <stdio.h>
#include <wiiuse/wpad.h>
#include <unistd.h>

/* magnitude minimal avant l'acceptation de l'orientation */
#define NUNCHUK_MAG_MIN 0.7f

float nunchuk_x;
float nunchuk_y;

void wii_music_init(void)
{	
	SND_Init(INIT_RATE_48000);
}

void *wii_music_load(const char *file)
{
	int f;

	f = open(file, O_RDONLY);
	if (f < 0) 
		stuff_quit_msg("Unable to open music : %s\n", file);
	return (void *)f;
}

void wii_music_play(void *music)
{
	if (PlayOgg( (int)music, 0, OGG_INFINITE_TIME) < 0 )
		stuff_quit_msg("Unable to play music\n");
}

int wii_music_is_playing(void)
{
	return StatusOgg() == OGG_STATUS_RUNNING ? TRUE : FALSE;
}

void wii_music_stop()
{
	StopOgg();	
}

void wii_music_unload(void *music)
{
	close((int)music);
}

void wii_initialize(void)
{
	fatInitDefault();
	WPAD_Init();
}


static void wii_handle_nunchuk(struct nunchuk_t *nunchuk)
{	
	nunchuk_x = 0;
	nunchuk_y = 0;
	if (nunchuk->js.mag > NUNCHUK_MAG_MIN) {
		float angle;

		angle = nunchuk->js.ang;		
		motion_angle = -angle;
		angle = DEG2RAD(angle);
		nunchuk_x = sin(angle);
		nunchuk_y = -cos(angle);
	}
}

void wii_handle_key(void)
{
	WPADData *wpad;

	WPAD_ScanPads();
	wpad = WPAD_Data(0);
	if (wpad) {
		if (wpad->exp.type == EXP_NUNCHUK) {
			wii_handle_nunchuk(&wpad->exp.nunchuk);
		}
	}
}
