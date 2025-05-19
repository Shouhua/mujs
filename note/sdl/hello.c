#include <SDL2/SDL.h>

SDL_Window *gWindow = NULL;
SDL_Surface *gScreenSurface = NULL;
SDL_Surface *gHelloWorld = NULL;

int init()
{
	int success = 0;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		fprintf(stderr, "SDL init failed\n");
		success = -1;
	}
	else
	{
		gWindow = SDL_CreateWindow(
			"SDL tutorial",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			630,
			460,
			SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			fprintf(stderr, "Create window failed\n");
			success = -1;
		}
		else
		{
			gScreenSurface = SDL_GetWindowSurface(gWindow);
		}
	}
	return success;
}

int loadMedia()
{
	int success = 0;
	gHelloWorld = SDL_LoadBMP("/Users/pengshouhua/project/mujs/note/ffmpeg/a.bmp");
	if (!gHelloWorld)
	{
		fprintf(stderr, "Load bmp failed\n");
		return -1;
	}
	return 0;
}

void close()
{
	SDL_FreeSurface(gHelloWorld);
	gHelloWorld = NULL;

	SDL_DestroyWindow(gWindow);
	gWindow = NULL;

	SDL_Quit();
}

int main(int argc, char *argv[])
{
	init();
	loadMedia();
	SDL_BlitSurface(gHelloWorld, NULL, gScreenSurface, NULL);
	SDL_UpdateWindowSurface(gWindow);
	// Hack to get window to stay up
	SDL_Event e;
	int quit = -1;
	while (quit == -1)
	{
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
				quit = 0;
		}
	}
	close();
	return 0;
}