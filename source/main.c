// Simple citro2d sprite drawing example
// Images borrowed from:
//   https://kenney.nl/assets/space-shooter-redux
#include <citro2d.h>

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_SPRITES   768
#define SCREEN_WIDTH  400
#define SCREEN_HEIGHT 240

// Simple sprite struct
typedef struct
{
	C2D_Sprite spr;
	float dx, dy; // velocity
} Sprite;

// Initialise the sprite sheet
static C2D_SpriteSheet spriteSheet;

// Sprites
Sprite spr_bear;
Sprite spr_wall;
Sprite spr_fire;
Sprite spr_star;

//---------------------------------------------------------------------------------
static void initSprites() {
//---------------------------------------------------------------------------------
	// Attach the sprites from the sprite sheet
	C2D_SpriteFromSheet(&spr_bear.spr, spriteSheet, 0);
	C2D_SpriteFromSheet(&spr_wall.spr, spriteSheet, 1);
	C2D_SpriteFromSheet(&spr_fire.spr, spriteSheet, 3);
	C2D_SpriteFromSheet(&spr_star.spr, spriteSheet, 4);

	spr_bear.dx = 150;
	spr_bear.dy = 150;

	//C2D_SpriteMove(&spr_bear.spr, spr_bear.dx, spr_bear.dy);
	C2D_SpriteSetPos(&spr_bear.spr, spr_bear.dx, spr_bear.dy);
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
