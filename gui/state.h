#ifndef _STATE_H_
#define _STATE_H_

#include <glib.h>

typedef struct GameStateInfo
{
	gboolean rom_exists;
	gboolean has_state_file;
} GameStateInfo;
typedef enum GameState
{
	GAME_STATE_NONE = 0,
	GAME_STATE_PAUSED,
	GAME_STATE_STOP
} GameState;

void game_state_fill(GameStateInfo * info, const char * rom_file);
void game_state_set(GameState newState);

#endif
