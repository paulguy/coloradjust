#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>

#define GETOFFSET(x, y, surface) ((surface->w * y) + x)

typedef struct {
	void *prev, *next;
	unsigned char value;
	unsigned char pos;
} Mark;

typedef enum {
	RED, GREEN, BLUE
} Mode;

int buildcurve(unsigned char *ramp, Mark *marks);
Mark *initmarks();
void addmark(Mark *curmark); // adds AFTER selected mark
void delmark(Mark *curmark);
void plotgraph(unsigned char *points, SDL_Surface *surface, unsigned char r, unsigned char g, unsigned char b);
void drawramps(SDL_Surface *surface, unsigned char *red, unsigned char *green, unsigned char *blue);
void drawscreen(SDL_Surface *surface, SDL_Surface *bg, unsigned char *red, unsigned char *green, unsigned char *blue, Mark *rmarks, Mark *gmarks, Mark *bmarks, Mark *curmark);

int main(int argc, char **argv) {
	int running, i;
	Mode mode;
	Mark *curmark;

	unsigned char red[256];
	unsigned char green[256];
	unsigned char blue[256];
	Mark *rmarks;
	Mark *gmarks;
	Mark *bmarks;
	Mark *temp;

	SDL_Surface *screen;
	SDL_Surface *bg, *text;
	SDL_Event event;

	if(SDL_Init(SDL_INIT_VIDEO) == -1) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE);
	if(screen == NULL) {
		fprintf(stderr, "Couldn't set video mode to 640x480x32bit: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	bg = SDL_LoadBMP("screen.bmp");
	if(bg == NULL) {
		fprintf(stderr, "Couldn't load screen.bmp: %s\n", SDL_GetError());
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	rmarks = initmarks();
	if(rmarks == NULL) {
		fprintf(stderr, "Couldn't initialize red marks linked list.");
		SDL_Quit();
		exit(EXIT_FAILURE);
	}
	if(buildcurve(red, rmarks) == -1) {
		fprintf(stderr, "Red marks list is invalid.\n");
		SDL_Quit();
		exit(EXIT_FAILURE);	}

	gmarks = initmarks();
	if(gmarks == NULL) {
		fprintf(stderr, "Couldn't initialize green marks linked list.");
		SDL_Quit();
		exit(EXIT_FAILURE);
	}
	if(buildcurve(green, gmarks) == -1) {
		fprintf(stderr, "Green marks list is invalid.\n");
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	bmarks = initmarks();
	if(bmarks == NULL) {
		fprintf(stderr, "Couldn't initialize blue marks linked list.");
		SDL_Quit();
		exit(EXIT_FAILURE);
	}
	if(buildcurve(blue, bmarks) == -1) {
		fprintf(stderr, "Blue marks list is invalid.\n");
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	mode = RED;
	curmark = rmarks;
	running = 1;

	drawscreen(screen, bg, red, green, blue, rmarks, gmarks, bmarks, curmark);
	while(running == 1) {
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_QUIT:
					running = 0;
					break;
				case SDL_KEYDOWN:
					switch(event.key.keysym.sym) {
						case SDLK_q:
							running = 0;
							break;
						case SDLK_a:
							if(mode != RED) {
								mode = RED;
								curmark = rmarks;
							}
							break;
						case SDLK_s:
							if(mode != GREEN) {
								mode = GREEN;
								curmark = gmarks;
							}
							break;
						case SDLK_d:
							if(mode != BLUE) {
								mode = BLUE;
								curmark = bmarks;
							}
							break;
						case SDLK_z:
							if(curmark->prev != NULL) {
								curmark = curmark->prev;
							}
							break;
						case SDLK_x:
							if(curmark->next != NULL) {
								curmark = curmark->next;
							}
							break;
						case SDLK_UP:
							if(curmark->value < 255) {
								curmark->value++;
							}
							switch(mode) {
								case RED:
									if(buildcurve(red, rmarks) == -1) {
										fprintf(stderr, "Red marks list is invalid.\n");
										SDL_Quit();
										exit(EXIT_FAILURE);
									}
									break;
								case GREEN:
									if(buildcurve(green, gmarks) == -1) {
										fprintf(stderr, "Green marks list is invalid.\n");
										SDL_Quit();
										exit(EXIT_FAILURE);
									}
									break;
								case BLUE:
									if(buildcurve(blue, bmarks) == -1) {
										fprintf(stderr, "Blue marks list is invalid.\n");
										SDL_Quit();
										exit(EXIT_FAILURE);
									}
									break;
							}
							drawscreen(screen, bg, red, green, blue, rmarks, gmarks, bmarks, curmark);
							break;
						case SDLK_DOWN:
							if(curmark->value > 0) {
								curmark->value--;
							}
							switch(mode) {
								case RED:
									if(buildcurve(red, rmarks) == -1) {
										fprintf(stderr, "Red marks list is invalid.\n");
										SDL_Quit();
										exit(EXIT_FAILURE);
									}
									break;
								case GREEN:
									if(buildcurve(green, gmarks) == -1) {
										fprintf(stderr, "Green marks list is invalid.\n");
										SDL_Quit();
										exit(EXIT_FAILURE);
									}
									break;
								case BLUE:
									if(buildcurve(blue, bmarks) == -1) {
										fprintf(stderr, "Blue marks list is invalid.\n");
										SDL_Quit();
										exit(EXIT_FAILURE);
									}
									break;
							}
							drawscreen(screen, bg, red, green, blue, rmarks, gmarks, bmarks, curmark);
							break;
						case SDLK_LEFT:
							if(curmark->prev != NULL && curmark->next != NULL) {
								if(curmark->pos > ((Mark *)curmark->prev)->pos + 1) {
									curmark->pos--;
								}
							}
							switch(mode) {
								case RED:
									if(buildcurve(red, rmarks) == -1) {
										fprintf(stderr, "Red marks list is invalid.\n");
										SDL_Quit();
										exit(EXIT_FAILURE);
									}
									break;
								case GREEN:
									if(buildcurve(green, gmarks) == -1) {
										fprintf(stderr, "Green marks list is invalid.\n");
										SDL_Quit();
										exit(EXIT_FAILURE);
									}
									break;
								case BLUE:
									if(buildcurve(blue, bmarks) == -1) {
										fprintf(stderr, "Blue marks list is invalid.\n");
										SDL_Quit();
										exit(EXIT_FAILURE);
									}
									break;
							}
							drawscreen(screen, bg, red, green, blue, rmarks, gmarks, bmarks, curmark);
							break;
						case SDLK_RIGHT:
							if(curmark->prev != NULL && curmark->next != NULL) {
								if(curmark->pos < ((Mark *)curmark->next)->pos - 1) {
									curmark->pos++;
								}
							}
							switch(mode) {
								case RED:
									if(buildcurve(red, rmarks) == -1) {
										fprintf(stderr, "Red marks list is invalid.\n");
										SDL_Quit();
										exit(EXIT_FAILURE);
									}
									break;
								case GREEN:
									if(buildcurve(green, gmarks) == -1) {
										fprintf(stderr, "Green marks list is invalid.\n");
										SDL_Quit();
										exit(EXIT_FAILURE);
									}
									break;
								case BLUE:
									if(buildcurve(blue, bmarks) == -1) {
										fprintf(stderr, "Blue marks list is invalid.\n");
										SDL_Quit();
										exit(EXIT_FAILURE);
									}
									break;
							}
							drawscreen(screen, bg, red, green, blue, rmarks, gmarks, bmarks, curmark);
							break;
						case SDLK_c:
							if(curmark->next != NULL && ((Mark *)curmark->next)->pos > curmark->pos + 1) {
								addmark(curmark);
							}
							drawscreen(screen, bg, red, green, blue, rmarks, gmarks, bmarks, curmark);
							break;
						case SDLK_v:
							if(curmark->prev != NULL && curmark->next != NULL) {
								temp = curmark->prev;
								delmark(curmark);
								curmark = temp;
							}
							drawscreen(screen, bg, red, green, blue, rmarks, gmarks, bmarks, curmark);
							break;
						case SDLK_w:
							if(saveicc("skeleton.icc", "out.icc", red, green, blue) == -1) {
								fprintf(stderr, "Couldn't save to out.icc!\n");
							} else {
								fprintf(stderr, "ICC file written to out.icc successfully.\n");
							}
						default:
							break;
					}
					break;
				default:
					break;
			}
		}

		SDL_Flip(screen);
		SDL_Delay(100);
	}

	SDL_Quit();
	exit(EXIT_SUCCESS);
}

int buildcurve(unsigned char *ramp, Mark *marks) {
	int start, change, length, initial, i;
	double slope, current;
	Mark *curmark;

	if(marks->pos != 0) {
		return(-1);
	}

	curmark = marks;

	while(curmark->next != NULL) {
		start = curmark->pos;
		length = ((Mark *)curmark->next)->pos - start;
		initial = curmark->value;
		change = ((Mark *)curmark->next)->value - curmark->value;

		if(length < 1) {
			return(-1);
		}

		slope = (double)change / (double)length;
		for(i = 0; i < length; i++) {
			ramp[i + start] = (unsigned char)(slope * (double)i) + initial;
		}

		curmark = curmark->next;
	}

	if(curmark->pos != 255) {
		return(-1);
	}
	ramp[255] = curmark->value;
	return(0);
}

Mark *initmarks() {
	Mark *marks;

	marks = (Mark *)malloc(sizeof(Mark));
	if(marks == NULL) {
		return(NULL);
	}
	marks->prev = NULL;
	marks->next = malloc(sizeof(Mark));
	if(marks->next == NULL) {
		free(marks);
		return(NULL);
	}
	marks->value = 0;
	marks->pos = 0;
	((Mark *)marks->next)->next = NULL;
	((Mark *)marks->next)->prev = marks;
	((Mark *)marks->next)->value = 255;
	((Mark *)marks->next)->pos = 255;
	return(marks);
}

void addmark(Mark *curmark) {
	Mark *new;
	//initialize
	new = (Mark *)malloc(sizeof(Mark));
	new->prev = curmark; 
	new->next = curmark->next;
	new->pos = curmark->pos + 1;
	new->value = curmark->value;
	//insert
	((Mark *)curmark->next)->prev = new;
	curmark->next = new;
}

void delmark(Mark *curmark) {
	((Mark *)curmark->next)->prev = curmark->prev;
	((Mark *)curmark->prev)->next = curmark->next;
	free(curmark);
}

void plotgraph(unsigned char *points, SDL_Surface *surface, unsigned char r, unsigned char g, unsigned char b) {
	int i;
	int pcolor = SDL_MapRGB(surface->format, r, g, b);

	SDL_LockSurface(surface);

	for(i = 0; i < 255; i++) {
		((int *)surface->pixels)[GETOFFSET(i, (255 - points[i]), surface)] = pcolor;
	}

	SDL_UnlockSurface(surface);
}

void drawramps(SDL_Surface *surface, unsigned char *red, unsigned char *green, unsigned char *blue) {
	int i;
	SDL_Rect rect;
	rect.w = 1;
	rect.h = 100;

	rect.y = 0;
	for(i = 0; i <= 255; i++) {
		rect.x = 384 + i;
		SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, red[i], green[i], blue[i]));
	}
	rect.y = 100;
	for(i = 0; i <= 255; i++) {
		rect.x = 384 + i;
		SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, red[i], 0, 0));
	}
	rect.y = 200;
	for(i = 0; i <= 255; i++) {
		rect.x = 384 + i;
		SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, 0, green[i], 0));
	}
	rect.y = 300;
	for(i = 0; i <= 255; i++) {
		rect.x = 384 + i;
		SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, 0, 0, blue[i]));
	}
}

void drawscreen(SDL_Surface *surface, SDL_Surface *bg, unsigned char *red, unsigned char *green, unsigned char *blue, Mark *rmarks, Mark *gmarks, Mark *bmarks, Mark *marksel) {
	Mark *curmark;
	SDL_Rect rect;
	rect.w = 3;
	rect.h = 3;

	SDL_BlitSurface(bg, NULL, surface, NULL);

	plotgraph(red, surface, 255, 0, 0);
	curmark = rmarks;
	while(curmark != NULL) {
		rect.x = curmark->pos;
		rect.y = 255 - curmark->value;
		SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, 255, 0, 0));
		curmark = curmark->next;
	}
	plotgraph(green, surface, 0, 255, 0);
	curmark = gmarks;
	while(curmark != NULL) {
		rect.x = curmark->pos;
		rect.y = 255 - curmark->value;
		SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, 0, 255, 0));
		curmark = curmark->next;
	}
	plotgraph(blue, surface, 0, 0, 255);
	curmark = bmarks;
	while(curmark != NULL) {
		rect.x = curmark->pos;
		rect.y = 255 - curmark->value;
		SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, 0, 0, 255));
		curmark = curmark->next;
	}
	rect.x = marksel->pos;
	rect.y = 255 - marksel->value;
	SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, 255, 255, 255));

	drawramps(surface, red, green, blue);
}

int saveicc(char *skeleton, char *iccfile, char *red, char *green, char *blue) {
	FILE *skel, *out;
	int size;
	char *skeldata;

	skel = fopen(skeleton, "rb");
	if(skel == NULL) {
		return(-1);
	}
	fseek(skel, 0, SEEK_END);
	size = ftell(skel);
	rewind(skel);

	out = fopen(iccfile, "wb");
	if(out == NULL) {
		return(-1);
	}

	skeldata = (char *)malloc(sizeof(char) * size);

	fread(skeldata, sizeof(char), size, skel);
	fwrite(skeldata, sizeof(char), size, out);
	fwrite(red, sizeof(char), 256, out);
	fwrite(green, sizeof(char), 256, out);
	fwrite(blue, sizeof(char), 256, out);

	fclose(skel);
	fclose(out);
	free(skeldata);
}
