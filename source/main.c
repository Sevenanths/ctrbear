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
}

void read_input(struct Game *game) {
	hidScanInput();
		
	u32 kDown = hidKeysDown();

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
	init_game(game);

	// Main loop
	while (aptMainLoop())
	{
		// Read input
		read_input(game);

		// Render the scene
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
		C2D_SceneBegin(top);

		movement(game);
		draw(game);

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
