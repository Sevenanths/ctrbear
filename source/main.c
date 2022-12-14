#include <citro2d.h>

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "sound.h"

#define OBJECT_WIDTH 16
#define OBJECT_HEIGHT 16

#define GC_WIDTH 400
#define GC_HEIGHT 240

#define TOUCH_WIDTH 320
#define TOUCH_HEIGHT 240

#define BEAR_SPEED_CLASSIC 5
#define BEAR_SPEED_HARD 6

#define OBJECT_SPEED_CLASSIC 2.5 
#define OBJECT_SPEED_HARD 3

#define NUM_OBJECTS_CLASSIC 2
#define NUM_OBJECTS_HARD 3

#define NUM_LIVES_CLASSIC 3
#define NUM_LIVES_HARD 1

int BEAR_SPEED;
int OBJECT_SPEED;
int NUM_OBJECTS;

// Simple sprite struct
typedef struct
{
	C2D_Sprite spr;
	float dx, dy; // velocity
} Sprite;

struct Bear {
	int x;
	int y;
	int direction;
};

enum BearDirection {
	BEAR_UP = 1,
	BEAR_DOWN = 2,
	BEAR_LEFT = 3,
	BEAR_RIGHT = 4
};

struct BearObject {
	int type;
	int direction;
	int x;
	int y;
};

enum BearObjectDirection {
	UP_LEFT = 1,
	UP_RIGHT = 2,
	DOWN_LEFT = 3,
	DOWN_RIGHT = 4
};

enum BearObjectTypes {
	FIRE = 1,
	STAR = 2
};

enum GameModes {
	TITLE, GAME, GAME_OVER
};

enum Difficulties {
	CLASSIC, HARD
};

struct Game {
	struct Bear bear;
	struct BearObject objects[NUM_OBJECTS_HARD * 2];
	int score;
};

int random_integer(int minimum_number, int max_number) {
	return rand() % (max_number + 1 - minimum_number) + minimum_number;
}

int random_coordinate_x() {
	return random_integer(OBJECT_WIDTH, GC_WIDTH - (OBJECT_WIDTH * 2));
}

int random_coordinate_y() {
	return random_integer(OBJECT_HEIGHT, GC_HEIGHT - (OBJECT_WIDTH * 2));
}

int random_direction() {
	return random_integer(1, 4);
}

int score = 0;
int lives = 0;
bool paused = false;
bool quit_game = false;
int game_mode = TITLE;
int difficulty = CLASSIC;
int flicker_timer = 0;
bool show_button_prompt = true;

// Initialise the sprite sheet
static C2D_SpriteSheet spriteSheet;

// Initialise text-related things
static C2D_Font fnt_dinbek;

// Static text
C2D_TextBuf g_staticBuf;
C2D_Text g_staticText[3];

enum StringKeys {
	PAUSED = 0,
};

// Dynamic text
C2D_TextBuf g_dynamicBuf;
C2D_Text g_dynamicText;

int white;
int black;
float score_size = 1;

// Sprites
Sprite spr_bear;
Sprite spr_wall;
Sprite spr_fire;
Sprite spr_star;
Sprite spr_classic;
Sprite spr_hard;
Sprite bg_background;
Sprite bg_bottom;
Sprite bg_title;
Sprite bg_title_bottom;
Sprite bg_game_over;
Sprite bg_game_over_bottom;

// Sound variable
struct sound *home;

//---------------------------------------------------------------------------------
static void initSprites() {
//---------------------------------------------------------------------------------
	// Attach the sprites from the sprite sheet
	C2D_SpriteFromSheet(&spr_bear.spr, spriteSheet, 0);
	C2D_SpriteFromSheet(&spr_wall.spr, spriteSheet, 1);
	C2D_SpriteFromSheet(&spr_fire.spr, spriteSheet, 3);
	C2D_SpriteFromSheet(&spr_star.spr, spriteSheet, 4);
	C2D_SpriteFromSheet(&bg_background.spr, spriteSheet, 5);
	C2D_SpriteFromSheet(&bg_bottom.spr, spriteSheet, 6);
	C2D_SpriteFromSheet(&bg_title.spr, spriteSheet, 7);
	C2D_SpriteFromSheet(&bg_title_bottom.spr, spriteSheet, 8);
	C2D_SpriteFromSheet(&spr_classic.spr, spriteSheet, 9);
	C2D_SpriteFromSheet(&spr_hard.spr, spriteSheet, 10);
	C2D_SpriteFromSheet(&bg_game_over.spr, spriteSheet, 11);
	C2D_SpriteFromSheet(&bg_game_over_bottom.spr, spriteSheet, 12);

	// Background position is always 0 0
	C2D_SpriteSetPos(&bg_background.spr, 0, 0);
	C2D_SpriteSetPos(&bg_bottom.spr, 0, 0);
	C2D_SpriteSetPos(&bg_title.spr, 0, 0);
	C2D_SpriteSetPos(&bg_title_bottom.spr, 0, 0);
	C2D_SpriteSetPos(&bg_game_over.spr, 0, 0);
	C2D_SpriteSetPos(&bg_game_over_bottom.spr, 0, 0);
}

static void initStrings() {
	C2D_TextFontParse(&g_staticText[PAUSED], fnt_dinbek, g_staticBuf, "PAUSED");
	C2D_TextOptimize(&g_staticText[PAUSED]);
}

void init_game(struct Game *game) {
	/* 
    	Bear
    */

    // Set bear coordinates to the middle of the screen
    game->bear.x = (GC_WIDTH / 2) - (OBJECT_WIDTH / 2);
    game->bear.y = (GC_HEIGHT / 2) - (OBJECT_HEIGHT / 2);
    game->bear.direction = random_direction();

    /*
		Objects
    */
    /* Set the first group of NUM_OBJECTS objects to fire */
    for (int i = 0; i < NUM_OBJECTS; ++i)
    {
    	game->objects[i].type = FIRE;
    }
    /* Set the second group op NUM_OBJECTS objects to fire */
    for (int i = NUM_OBJECTS; i < 2 * NUM_OBJECTS; ++i)
    {
    	game->objects[i].type = STAR;
    }

    /* Generate random coordinates and directions for all objects */
    for (int i = 0; i < NUM_OBJECTS * 2; ++i)
    {
    	game->objects[i].x = random_coordinate_x();
    	game->objects[i].y = random_coordinate_y();
    	game->objects[i].direction = random_direction();
    }

    score = 0;
}

void start_game(struct Game *game) {
	if (difficulty == CLASSIC) {
		NUM_OBJECTS = NUM_OBJECTS_CLASSIC;
		BEAR_SPEED = BEAR_SPEED_CLASSIC;
		OBJECT_SPEED = OBJECT_SPEED_CLASSIC;
		lives = NUM_LIVES_CLASSIC;
	} else if (difficulty == HARD) {
		NUM_OBJECTS = NUM_OBJECTS_HARD;
		BEAR_SPEED = BEAR_SPEED_HARD;
		OBJECT_SPEED = OBJECT_SPEED_HARD;
		lives = NUM_LIVES_HARD;
	}

	//srand(gettime()); TODO
	init_game(game);
	game_mode = GAME;
}

void draw_text_outline(const C2D_Text *text, int x, int y, int size) {
	// The first four calls are used to draw the font outline
	C2D_DrawText(text, C2D_WithColor | C2D_AlignCenter, x + 1, y + 1, 0.5f, size, size, black);
	C2D_DrawText(text, C2D_WithColor | C2D_AlignCenter, x + 1, y - 1, 0.5f, size, size, black);
	C2D_DrawText(text, C2D_WithColor | C2D_AlignCenter, x - 1, y + 1, 0.5f, size, size, black);
	C2D_DrawText(text, C2D_WithColor | C2D_AlignCenter, x - 1, y - 1, 0.5f, size, size, black);

	// The final call is the actual centred font
	C2D_DrawText(text, C2D_WithColor | C2D_AlignCenter, x, y, 0.5f, size, size, white);
}

void draw(struct Game *game) {
	// Background image
	C2D_DrawSprite(&bg_background.spr);

	// Horizontal walls
	for (int x = 0; x < 25; ++x)
	{
		// Top walls
		C2D_SpriteSetPos(&spr_wall.spr, x * OBJECT_WIDTH, 0);
		C2D_DrawSprite(&spr_wall.spr);

		// Bottom walls
		C2D_SpriteSetPos(&spr_wall.spr, x * OBJECT_WIDTH, GC_HEIGHT - OBJECT_HEIGHT);
		C2D_DrawSprite(&spr_wall.spr);
	}

	// Vertical walls
	for (int y = 0; y < 15; ++y)
	{
		// Left walls
		C2D_SpriteSetPos(&spr_wall.spr, 0, y * OBJECT_HEIGHT);
		C2D_DrawSprite(&spr_wall.spr);

		// Right walls
		C2D_SpriteSetPos(&spr_wall.spr, GC_WIDTH - OBJECT_WIDTH, y * OBJECT_HEIGHT);
		C2D_DrawSprite(&spr_wall.spr);
	}

	// Draw objects
	for (int i = 0; i < NUM_OBJECTS * 2; ++i)
	{
		if (game->objects[i].type == FIRE) {
			C2D_SpriteSetPos(&spr_fire.spr, game->objects[i].x, game->objects[i].y);
			C2D_DrawSprite(&spr_fire.spr);
		} else if (game->objects[i].type == STAR) {
			C2D_SpriteSetPos(&spr_star.spr, game->objects[i].x, game->objects[i].y);
			C2D_DrawSprite(&spr_star.spr);
		}
	}

	C2D_SpriteSetPos(&spr_bear.spr, spr_bear.dx, spr_bear.dy);
	C2D_DrawSprite(&spr_bear.spr);

	if (paused) {
		draw_text_outline(&g_staticText[PAUSED], (GC_WIDTH / 2), (GC_HEIGHT / 2) - 20, 1.5f);
	}
}

void draw_score() {
	// Initialise the string to store the score in
	char str_score[32];
	itoa(score, str_score, 10);

	// Initialise the text
	// By pre-initialising the text, we will be able to figure out its dimensions pre-emptively
	C2D_TextFontParse(&g_dynamicText, fnt_dinbek, g_dynamicBuf, str_score);
	C2D_TextOptimize(&g_dynamicText);

	// Now, let's figure out the width (and height)
	float score_width;
	float score_height;

	C2D_TextGetDimensions(&g_dynamicText, score_size, score_size, &score_width, &score_height);

	// Draw score
	int score_x = (TOUCH_WIDTH / 2) + 1;
	int score_y = 42;

	
	// The first four calls are used to draw the font outline
	draw_text_outline(&g_dynamicText, score_x, score_y, score_size);

	C2D_TextBufClear(g_dynamicBuf);
}

void draw_lives() {
	for (int i = 0; i < lives; ++i)
	{
		C2D_SpriteSetPos(&spr_bear.spr, (TOUCH_WIDTH / 2) - (OBJECT_WIDTH / 2), 108 + (i * 18));
		C2D_DrawSprite(&spr_bear.spr);
	}
}

void draw_bottom(struct Game *game) {
	C2D_DrawSprite(&bg_bottom.spr);
	draw_score();
	draw_lives();
}

void draw_title_top() {
	C2D_DrawSprite(&bg_title.spr);
}

void draw_title_bottom() {
	C2D_DrawSprite(&bg_title_bottom.spr);

	int difficulty_x = (TOUCH_WIDTH / 2) - (64 / 2);
	int difficulty_y = 85;

	if (difficulty == CLASSIC) {
		C2D_SpriteSetPos(&spr_classic.spr, difficulty_x, difficulty_y);
		C2D_DrawSprite(&spr_classic.spr);
	} else if (difficulty == HARD) {
		C2D_SpriteSetPos(&spr_hard.spr, difficulty_x, difficulty_y);
		C2D_DrawSprite(&spr_hard.spr);
	}
}

void draw_game_over_top() {
	C2D_DrawSprite(&bg_game_over.spr);
}

void draw_game_over_bottom() {
	C2D_DrawSprite(&bg_game_over_bottom.spr);
}

void read_input(struct Game *game) {
	hidScanInput();
		
	u32 kDown = hidKeysDown();

	// Check for pause
	if (kDown & KEY_START)
		paused = !paused;

	// Check for exit
	if (kDown & KEY_SELECT)
		quit_game = true;

	// Digital input
	if (kDown & KEY_UP)
		game->bear.direction = BEAR_UP;
	if (kDown & KEY_DOWN)
		game->bear.direction = BEAR_DOWN;
	if (kDown & KEY_LEFT)
		game->bear.direction = BEAR_LEFT;
	if (kDown & KEY_RIGHT)
		game->bear.direction = BEAR_RIGHT;
}

void read_title_input(struct Game *game) {
	hidScanInput();
		
	u32 kDown = hidKeysDown();

	// Check for pause
	if (kDown & KEY_START)
		start_game(game);

	// Check for exit
	if (kDown & KEY_SELECT)
		quit_game = true;

	// Digital input
	if (kDown & KEY_LEFT)
		difficulty = CLASSIC;
	if (kDown & KEY_RIGHT)
		difficulty = HARD;
}

void read_game_over_input() {
	hidScanInput();
		
	u32 kDown = hidKeysDown();

	// Check for pause
	if (kDown & KEY_START)
		game_mode = TITLE;

	// Check for exit
	if (kDown & KEY_SELECT)
		quit_game = true;
}

void movement(struct Game *game) {
	/* 
		Bear collision (walls)
	*/
	if (game->bear.x - BEAR_SPEED <= OBJECT_HEIGHT) {
		game->bear.direction = BEAR_RIGHT;
	} else if (game->bear.x + BEAR_SPEED >= GC_WIDTH - (OBJECT_WIDTH * 2)) {
		game->bear.direction = BEAR_LEFT;
	} else if (game->bear.y - BEAR_SPEED <= OBJECT_HEIGHT) {
		game->bear.direction = BEAR_DOWN;
	} else if (game->bear.y + BEAR_SPEED >= GC_HEIGHT - (OBJECT_HEIGHT * 2)) {
		game->bear.direction = BEAR_UP;
	}

	/*
		Bear movement
	*/
	switch(game->bear.direction) {
		case BEAR_UP:
			game->bear.y -= BEAR_SPEED;
			break;
		case BEAR_DOWN:
			game->bear.y += BEAR_SPEED;
			break;
		case BEAR_LEFT:
			game->bear.x -= BEAR_SPEED;
			break;
		case BEAR_RIGHT:
			game->bear.x += BEAR_SPEED;
			break;
	}

	spr_bear.dx = game->bear.x;
	spr_bear.dy = game->bear.y;

	/*
		Object movement
	*/
	for (int i = 0; i < NUM_OBJECTS * 2; ++i)
	{
		// Left wall collision
		if (game->objects[i].x - OBJECT_SPEED <= OBJECT_WIDTH) {
			if (game->objects[i].direction == UP_LEFT) {
				game->objects[i].direction = UP_RIGHT;
			} else if (game->objects[i].direction == DOWN_LEFT) {
				game->objects[i].direction = DOWN_RIGHT;
			}
		// Right wall collision
		} else if (game->objects[i].x + OBJECT_SPEED >= GC_WIDTH - (OBJECT_WIDTH * 2)) {
			if (game->objects[i].direction == UP_RIGHT) {
				game->objects[i].direction = UP_LEFT;
			} else if (game->objects[i].direction == DOWN_RIGHT) {
				game->objects[i].direction = DOWN_LEFT;
			}
		// Top wall collision
		} else if (game->objects[i].y - OBJECT_SPEED <= OBJECT_HEIGHT) {
			if (game->objects[i].direction == UP_LEFT) {
				game->objects[i].direction = DOWN_LEFT;
			} else if (game->objects[i].direction == UP_RIGHT) {
				game->objects[i].direction = DOWN_RIGHT;
			}
		// Bottom wall collision
		} else if (game->objects[i].y + OBJECT_SPEED >= GC_HEIGHT - (OBJECT_HEIGHT * 2)) {
			if (game->objects[i].direction == DOWN_LEFT) {
				game->objects[i].direction = UP_LEFT;
			} else if (game->objects[i].direction == DOWN_RIGHT) {
				game->objects[i].direction = UP_RIGHT;
			}
		}
	
		if (game->objects[i].direction == UP_LEFT) {
			game->objects[i].x -= OBJECT_SPEED;
			game->objects[i].y -= OBJECT_SPEED;
		} else if (game->objects[i].direction == UP_RIGHT) {
			game->objects[i].x += OBJECT_SPEED;
			game->objects[i].y -= OBJECT_SPEED;
		} else if (game->objects[i].direction == DOWN_LEFT) {
			game->objects[i].x -= OBJECT_SPEED;
			game->objects[i].y += OBJECT_SPEED;
		} else if (game->objects[i].direction == DOWN_RIGHT) {
			game->objects[i].x += OBJECT_SPEED;
			game->objects[i].y += OBJECT_SPEED;
		}
	}

	/* 
		Bear and object collision
	*/
	for (int i = 0; i < NUM_OBJECTS * 2; ++i)
	{
		// All hail the mighty bounding box calculation 
		// I ported this from my LUA code in 2015! It might just work!
		if (game->bear.x < game->objects[i].x + OBJECT_WIDTH &&
			game->bear.x + OBJECT_WIDTH > game->objects[i].x &&
			game->bear.y < game->objects[i].y + OBJECT_HEIGHT &&
			OBJECT_HEIGHT + game->bear.y > game->objects[i].y) {
	
			game->objects[i].x = random_coordinate_x();
			game->objects[i].y = random_coordinate_y();
			game->objects[i].direction = random_direction();
	
			if (game->objects[i].type == FIRE) {
				lives -= 1;

				if (lives == 0)
					game_mode = GAME_OVER;
			} else if (game->objects[i].type == STAR) {
				score += 1000;
			}
		}
	}
}

//---------------------------------------------------------------------------------
int main(int argc, char* argv[]) {
//---------------------------------------------------------------------------------
	// Init libs
	romfsInit();
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();

	audio_init();

	// Create screens
	C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	C3D_RenderTarget* bot = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

	// Font
	fnt_dinbek = C2D_FontLoad("romfs:/gfx/dinbekbold.bcfnt");
	g_dynamicBuf  = C2D_TextBufNew(4096); // support up to 4096 glyphs in the buffer
	g_staticBuf  = C2D_TextBufNew(4096); // support up to 4096 glyphs in the buffer
	white = C2D_Color32f(255.0f,255.0f,255.0f,255.0f);
	black = C2D_Color32f(17.0f,17.0f,17.0f,255.0f);

	// Load graphics
	spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/sprites.t3x");
	if (!spriteSheet) svcBreak(USERBREAK_PANIC);

	// Initialize sprites
	initSprites();
	// Initialize strings
	initStrings();

	home = sound_create(BGM);
    if (home != NULL) audio_load_ogg("romfs:/bg.ogg", home);
    else home->status = -1;

	// Create a game instance
	struct Game* game = malloc(sizeof(struct Game));

	// Main loop
	while (aptMainLoop())
	{

		// Render the scene
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
		C2D_TargetClear(bot, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
		if (game_mode == TITLE) {
			read_title_input(game);

			C2D_SceneBegin(top);
			draw_title_top();

			C2D_SceneBegin(bot);
			draw_title_bottom();
		} else if (game_mode == GAME) {
			// Read input
			read_input(game);

			if (!paused)
				movement(game);

			C2D_SceneBegin(top);
			draw(game);
			
			C2D_SceneBegin(bot);
			draw_bottom(game);
		} else if (game_mode == GAME_OVER) {
			read_game_over_input();

			C2D_SceneBegin(top);
			draw_game_over_top();

			C2D_SceneBegin(bot);
			draw_game_over_bottom();
			draw_score();
		}

		C3D_FrameEnd(0);

		if (quit_game) {
			break;
		}
	}

	// Delete graphics
	C2D_SpriteSheetFree(spriteSheet);

	// Delete font
	C2D_FontFree(fnt_dinbek);

	sound_stop(home);
	audio_stop();

	free(game);

	// Deinit libs
	C2D_Fini();
	C3D_Fini();
	gfxExit();
	romfsExit();
	return 0;
}
