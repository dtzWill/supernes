#include <SDL.h>

#include "platform.h"
#include "snes9x.h"
#include "soundux.h"

#define DIE(format, ...) do { \
		fprintf(stderr, "Died at %s:%d: ", __FILE__, __LINE__ ); \
		fprintf(stderr, format "\n", ## __VA_ARGS__); \
		abort(); \
	} while (0);


static SDL_AudioSpec spec;

static void audioCallback(void *userdata, Uint8 *stream, int len)
{
	int samplecount = len / 2; // 16 bit audio
	S9xMixSamples((short*)stream, samplecount);
}

void S9xInitAudioOutput()
{
	if (!Config.enableAudio) goto no_audio;

	if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) 
		DIE("SDL_InitSubSystem(AUDIO): %s", SDL_GetError());

	SDL_AudioSpec desired;
	desired.freq = Settings.SoundPlaybackRate;
	desired.format = AUDIO_S16LSB;
	desired.channels = Settings.Stereo ? 2 : 1;
	desired.samples = Settings.SoundBufferSize;
	desired.callback = audioCallback;
	desired.userdata = 0;

	if (SDL_OpenAudio(&desired, &spec) < 0) {
		fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
		goto no_audio_free;
	}

	Settings.APUEnabled = TRUE;
	Settings.SoundPlaybackRate = spec.freq;
	if (spec.format == AUDIO_S16LSB) {
		Settings.SixteenBitSound = TRUE;
	} else if (spec.format == AUDIO_S8) {
		Settings.SixteenBitSound = FALSE;
		fprintf(stderr, "Cannot output 8 bit sound\n");
		goto no_audio_free;
	} else {
		fprintf(stderr, "Invalid audio format\n");
		goto no_audio_free;
	}
	Settings.Stereo = spec.channels == 2 ? TRUE : FALSE;

	printf("Audio: %d Hz, %d %s, %s, %u samples in buffer\n", spec.freq,
		spec.channels, Settings.Stereo ? "channels" : "channel",
		Settings.SixteenBitSound ? "16 bits" : "8 bits",
		spec.samples);

	return;

no_audio_free:
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
	/* Fall through */
no_audio:
	Config.enableAudio = false;
	Settings.APUEnabled = FALSE;
	printf("Audio: no audio\n");

	return;
}

void S9xDeinitAudioOutput()
{
	if (!Config.enableAudio) return;
	SDL_CloseAudio();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void S9xAudioOutputEnable(bool enable)
{
	if (!Config.enableAudio) return;
	if (enable)	{
		CPU.APU_APUExecuting = Settings.APUEnabled = TRUE;
		so.stereo = Settings.Stereo;
		so.playback_rate = Settings.SoundPlaybackRate;
		S9xSetPlaybackRate(so.playback_rate);
		S9xSetSoundMute(FALSE);
		SDL_PauseAudio(FALSE);
	} else {
		S9xSetSoundMute(TRUE);
		SDL_PauseAudio(TRUE);
	}
}

