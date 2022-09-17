// Simple citro2d sprite drawing example
// Images borrowed from:
//   https://kenney.nl/assets/space-shooter-redux
#include <citro2d.h>

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define OBJECT_WIDTH 16
#define OBJECT_HEIGHT 16

#define GC_WIDTH 400
#define GC_HEIGHT 240

#define BEAR_SPEED 6
#define OBJECT_SPEED 3
#define NUM_OBJECTS 3

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

struct Game {
	struct Bear bear;
	struct BearObject objects[NUM_OBJECTS * 2];
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
int game_mode = TITLE;
int flicker_timer = 0;
bool show_button_prompt = true;

// Initialise the sprite sheet
static C2D_SpriteSheet spriteSheet;

// Sprites
Sprite spr_bear;
Sprite spr_wall;
Sprite spr_fire;
Sprite spr_star;
Sprite bg_background;

//---------------------------------------------------------------------------------
static void initSprites() {
//---------------------------------------------------------------------------------
	// Attach the sprites from the sprite sheet
	C2D_SpriteFromSheet(&spr_bear.spr, spriteSheet, 0);
	C2D_SpriteFromSheet(&spr_wall.spr, spriteSheet, 1);
	C2D_SpriteFromSheet(&spr_fire.spr, spriteSheet, 3);
	C2D_SpriteFromSheet(&spr_star.spr, spriteSheet, 4);
	C2D_SpriteFromSheet(&bg_background.spr, spriteSheet, 5);

	spr_bear.dx = 150;
	spr_bear.dy = 150;

	//C2D_SpriteMove(&spr_bear.spr, spr_bear.dx, spr_bear.dy);
	C2D_SpriteSetPos(&spr_bear.spr, spr_bear.dx, spr_bear.dy);

	// Background position is always 0 0
	C2D_SpriteSetPos(&bg_background.spr, 0, 0);
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
	//srand(gettime()); TODO
	init_game(game);
	game_mode = GAME;
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
	consoleInit(GFX_BOTTOM, NULL);

	// Create screens
	C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);

	// Load graphics
	spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/sprites.t3x");
	if (!spriteSheet) svcBreak(USERBREAK_PANIC);

	// Initialize sprites
	initSprites();

	char str[10];

	utoa(C2D_SpriteSheetCount(spriteSheet), str, 10);
	printf(str);
	printf("\x1b[8;1HPress Up to increment sprites");
	printf("\x1b[9;1HPress Down to decrement sprites");

	// Create a game instance
	struct Game* game = malloc(sizeof(struct Game));

	// Main loop
	while (aptMainLoop())
	{
		hidScanInput();
		/*
		// Respond to user input
		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu

		u32 kHeld = hidKeysHeld();
		if ((kHeld & KEY_UP) && numSprites < MAX_SPRITES)
			numSprites++;
		if ((kHeld & KEY_DOWN) && numSprites > 1)
			numSprites--;

		moveSprites();

		printf("\x1b[1;1HSprites: %zu/%u\x1b[K", numSprites, MAX_SPRITES);
		printf("\x1b[2;1HCPU:     %6.2f%%\x1b[K", C3D_GetProcessingTime()*6.0f);
		printf("\x1b[3;1HGPU:     %6.2f%%\x1b[K", C3D_GetDrawingTime()*6.0f);
		printf("\x1b[4;1HCmdBuf:  %6.2f%%\xx1b[K", C3D_GetCmdBufUsage()*100.0f);
		*/

		// Render the scene
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
		C2D_SceneBegin(top);

		C2D_DrawSprite(&bg_background.spr);
		C2D_DrawSprite(&spr_bear.spr);

		C3D_FrameEnd(0);
	}

	// Delete graphics
	C2D_SpriteSheetFree(spriteSheet);

	// Deinit libs
	C2D_Fini();
	C3D_Fini();
	gfxExit();
	romfsExit();
	return 0;
}
