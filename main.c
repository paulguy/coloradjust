#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>

#include "btext.h"

#define GETOFFSET(x, y, surface) ((surface->w * y) + x)
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define RMASK	(0xff000000)
#define GMASK	(0x00ff0000)
#define BMASK	(0x0000ff00)
#define AMASK	(0x000000ff)
#else
#define RMASK	(0x000000ff)
#define GMASK	(0x0000ff00)
#define BMASK	(0x00ff0000)
#define AMASK	(0xff000000)
#endif

typedef struct {
	void *prev, *next;
	Uint8 value;
	Uint8 pos;
} Mark;

typedef enum {
	RED, GREEN, BLUE
} Mode;

SDL_Surface *screen, *bg;
Uint32 gridcolor, bordercolor, textcolor, cursorcolor;
Uint32 redcolor, greencolor, bluecolor, colortextcolor;
bfont *font;
Mode mode;
SDL_Surface *statusmsg = NULL;
Uint32 statusx;
const char STATUS_READY[]	= "Ready";
const char STATUS_RED[]		= "Red graph selected";
const char STATUS_GREEN[]	= "Green graph selected";
const char STATUS_BLUE[]	= "Blue graph selected";
const char STATUS_CANTSAVE[]= "Couldn't save to out.icc!";
const char STATUS_SAVED[]	= "ICC file written to out.icc successfully.";

SDL_Surface *createBackground();
int buildcurve_linear(Uint8 *ramp, Mark *marks);
Mark *initmarks();
void addmark(Mark *curmark); // adds AFTER selected mark
void delmark(Mark *curmark);
void plotgraph(Uint8 *points, Uint8 r, Uint8 g, Uint8 b);
void drawramps(Uint8 *red, Uint8 *green, Uint8 *blue);
void drawscreen(Uint8 *red, Uint8 *green, Uint8 *blue, Mark *rmarks, Mark *gmarks, Mark *bmarks, Mark *marksel);
int saveicc(char *skeleton, char *iccfile, Uint8 *red, Uint8 *green, Uint8 *blue);
int updateStatus(const char *status);

int main(int argc, char **argv) {
	int running, i;
	Mark *curmark;

	Uint8 red[256];
	Uint8 green[256];
	Uint8 blue[256];
	Mark *rmarks;
	Mark *gmarks;
	Mark *bmarks;
	Mark *temp;

	SDL_Event event;
	int (*buildcurve)(Uint8 *ramp, Mark *marks);

	if(SDL_Init(SDL_INIT_VIDEO) == -1) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(-1);
	}

	screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE);
	if(screen == NULL) {
		fprintf(stderr, "Couldn't set video mode to 640x480x32bit: %s\n", SDL_GetError());
		exit(-1);
	}
	SDL_WM_SetCaption("Color Adjust", "Color Adjust");

	font = btext_loadFromBMP("font.bmp");
	if(font == NULL) {
		fprintf(stderr, "Failed to load font.\n");
		SDL_Quit();
		exit(-1);
	}

/* ----- SET UI COLORS HERE ----- */
	gridcolor = SDL_MapRGB(screen->format, 85, 85, 85);
	bordercolor = SDL_MapRGB(screen->format, 255, 255, 255);
	textcolor = SDL_MapRGB(screen->format, 255, 255, 255);
	cursorcolor = SDL_MapRGB(screen->format, 255, 255, 255);
	redcolor = SDL_MapRGB(screen->format, 170, 0, 0);
	greencolor = SDL_MapRGB(screen->format, 0, 170, 0);
	bluecolor = SDL_MapRGB(screen->format, 0, 0, 170);
	colortextcolor = SDL_MapRGB(screen->format, 0, 0, 0);

	bg = createBackground();
	if(bg == NULL) {
		fprintf(stderr, "Couldn't create background image: %s\n", SDL_GetError());
		SDL_Quit();
		exit(-1);
	}

	buildcurve = buildcurve_linear; /* set default curve */
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

	int moveup, movedown, moveleft, moveright;
	int moveupstep, movedownstep, moveleftstep, moverightstep;
	int rebuild;
	moveup = movedown = moveleft = moveright
	= moveupstep = movedownstep = moveleftstep = moverightstep
	= rebuild = 0;
	updateStatus(STATUS_READY);

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
								updateStatus(STATUS_RED);
								statusx = 640;
								mode = RED;
								curmark = rmarks;
							}
							break;
						case SDLK_s:
							if(mode != GREEN) {
								updateStatus(STATUS_GREEN);
								statusx = 640;
								mode = GREEN;
								curmark = gmarks;
							}
							break;
						case SDLK_d:
							if(mode != BLUE) {
								updateStatus(STATUS_BLUE);
								statusx = 640;
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
							if(movedown == 0) {
								moveup = 1;
								moveupstep = 4;
							} else
								movedown = 0;
							break;
						case SDLK_DOWN:
							if(moveup == 0) {
								movedown = 1;
								movedownstep = 4;
							} else
								moveup = 0;
							break;
						case SDLK_LEFT:
							if(moveright == 0) {
								moveleft = 1;
								moveleftstep = 4;
							} else
								moveright = 0;
							break;
						case SDLK_RIGHT:
							if(moveleft == 0) {
								moveright = 1;
								moverightstep = 4;
							} else
								moveleft = 0;
							break;
						case SDLK_c:
							if(curmark->next != NULL && ((Mark *)curmark->next)->pos > curmark->pos + 1) {
								addmark(curmark);
							}
							break;
						case SDLK_v:
							if(curmark->prev != NULL && curmark->next != NULL) {
								temp = curmark->prev;
								delmark(curmark);
								curmark = temp;
							}
							break;
						case SDLK_w:
							if(saveicc("skeleton.icc", "out.icc", red, green, blue) == -1) {
								updateStatus(STATUS_CANTSAVE);
								statusx = 640;
							} else {
								updateStatus(STATUS_SAVED);
								statusx = 640;
							}
						default:
							break;
					}
					break;
				case SDL_KEYUP:
					switch(event.key.keysym.sym) {
						case SDLK_UP:
							moveup = 0;
							break;
						case SDLK_DOWN:
							movedown = 0;
							break;
						case SDLK_LEFT:
							moveleft = 0;
							break;
						case SDLK_RIGHT:
							moveright = 0;
							break;
						default:
							break;
					}
				default:
					break;
			}
		}

		if(moveup != 0) {
			if(moveup == 1) {
				if(curmark->value < 255) {
					curmark->value++;
				}
				moveup = moveupstep;
				if(moveupstep > 1)
					moveupstep--;
			} else {
				moveup--;
			}
			rebuild = 1;
		}
		if(movedown != 0) {
			if(movedown == 1) {
				if(curmark->value > 0) {
					curmark->value--;
				}
				movedown = movedownstep;
				if(movedownstep > 1)
					movedownstep--;
			} else {
				movedown--;
			}
			rebuild = 1;
		}
		if(moveleft != 0) {
			if(moveleft == 1) {
				if(curmark->prev != NULL && curmark->next != NULL) {
					if(curmark->pos > ((Mark *)curmark->prev)->pos + 1) {
						curmark->pos--;
					}
				}
				moveleft = moveleftstep;
				if(moveleftstep > 1)
					moveleftstep--;
			} else {
				moveleft--;
			}
			rebuild = 1;
		}
		if(moveright != 0) {
			if(moveright == 1) {
				if(curmark->prev != NULL && curmark->next != NULL) {
					if(curmark->pos < ((Mark *)curmark->next)->pos - 1) {
						curmark->pos++;
					}
				}
				moveright = moverightstep;
				if(moverightstep > 1)
					moverightstep--;
			} else {
				moveright--;
			}
			rebuild = 1;
		}
		if(rebuild != 0) {
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
			rebuild = 0;
		}

		drawscreen(red, green, blue, rmarks, gmarks, bmarks, curmark);
		SDL_Flip(screen);
		SDL_Delay(50);
	}

	btext_free(font);

	SDL_Quit();
	exit(EXIT_SUCCESS);
}

int updateStatus(const char *status) {
	SDL_Surface *temp;

	if(statusmsg != NULL)
		SDL_FreeSurface(statusmsg);

	temp = btext_render(font, status, 0, textcolor, NULL, BTEXT_BGTRANSPARENT);
	if(temp == NULL)
		return(-1);
	statusmsg = SDL_ConvertSurface(temp, screen->format, SDL_SWSURFACE);
	SDL_FreeSurface(temp);
	if(statusmsg == NULL)
		return(-1);
	statusx = screen->w;

	return(0);
}

int buildcurve_linear(Uint8 *ramp, Mark *marks) {
	int i, distance, leftdist, rightdist;
	Mark *prevmark, *nextmark;

	if(marks->pos != 0 || marks->next == NULL) {
		return(-1);
	}

	prevmark = marks;
	nextmark = marks->next;
	ramp[0] = prevmark->value;
	distance = nextmark->pos - prevmark->pos;

	for(i = 0; i < 256; i++) {
		if(i == nextmark->pos) {
			ramp[i] = nextmark->value;
			if(nextmark->next == NULL)
				break;
			nextmark = nextmark->next;
			prevmark = prevmark->next;
			distance = nextmark->pos - prevmark->pos;
		}
		leftdist = i - prevmark->pos;
		rightdist = nextmark->pos - i;
		ramp[i] =	(prevmark->value * ((double)rightdist / (double)distance)) +
					(nextmark->value * ((double)leftdist / (double)distance));
	}

	return(0);
}

SDL_Surface *createBackground() {
	SDL_Surface *surface;
	SDL_Rect rect;
	char buffer[32];

	surface = SDL_CreateRGBSurface(SDL_SWSURFACE, 640, 480, 32, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
	if(surface == NULL)
		return(NULL);

	/* draw graph and borders */
	rect.x = 0;
	rect.w = 257;
	rect.h = 1;
	for(rect.y = 0; rect.y < 255; rect.y += 32)
		SDL_FillRect(surface, &rect, gridcolor);
	rect.y = 255;
	SDL_FillRect(surface, &rect, bordercolor);
	rect.y = 0;
	rect.w = 1;
	rect.h = 257;
	for(rect.x = 0; rect.x < 255; rect.x += 32)
		SDL_FillRect(surface, &rect, gridcolor);
	rect.x = 255;
	SDL_FillRect(surface, &rect, bordercolor);
	rect.x = 383;
	rect.y = 0;
	rect.w = 1;
	rect.h = 400;
	SDL_FillRect(surface, &rect, bordercolor);
	rect.x = 383;
	rect.y = 400;
	rect.w = 640 - 383;
	rect.h = 1;
	SDL_FillRect(surface, &rect, bordercolor);
	/* draw text */
	rect.w = 0;
	rect.h = 0;
	rect.y = 258;
	for(rect.x = 0; rect.x < 256; rect.x += 32) {
		snprintf(buffer, 32, "%d", rect.x);
		btext_renderToSurface(font, buffer, 0, textcolor, surface, &rect, BTEXT_BGTRANSPARENT);
	}
	rect.x = 258;
	for(rect.y = 0; rect.y < 256; rect.y += 32) {
		snprintf(buffer, 32, "%d", 255 - rect.y);
		btext_renderToSurface(font, buffer, 0, textcolor, surface, &rect, BTEXT_BGTRANSPARENT);
	}
	rect.y = 258;
	btext_renderToSurface(font, "255", 0, textcolor, surface, &rect, BTEXT_BGTRANSPARENT);
	rect.y = 249;
	btext_renderToSurface(font, "0", 0, textcolor, surface, &rect, BTEXT_BGTRANSPARENT);
	rect.x = 279;
	rect.y = 128 - (font->height + font->height / 2);
	btext_renderToSurface(font, "O", 0, textcolor, surface, &rect, BTEXT_BGTRANSPARENT);
	rect.y += font->height;
	btext_renderToSurface(font, "U", 0, textcolor, surface, &rect, BTEXT_BGTRANSPARENT);
	rect.y += font->height;
	btext_renderToSurface(font, "T", 0, textcolor, surface, &rect, BTEXT_BGTRANSPARENT);
	rect.x = 128;
	rect.y = 258 + font->height;
	btext_renderToSurface(font, "IN", 0, textcolor, surface, &rect, BTEXT_BGTRANSPARENT);
	rect.x = 0;
	rect.y += font->height;
	btext_renderToSurface(font, "LEFT/RIGHT: Decrement or incremenet input value.", 0, textcolor, surface, &rect, BTEXT_BGTRANSPARENT);
	rect.y += font->height;
	btext_renderToSurface(font, "LEFT/RIGHT: Decrement or incremenet output value.", 0, textcolor, surface, &rect, BTEXT_BGTRANSPARENT);
	rect.y += font->height;
	btext_renderToSurface(font, "A/S/D: Select RGB component.", 0, textcolor, surface, &rect, BTEXT_BGTRANSPARENT);
	rect.y += font->height;
	btext_renderToSurface(font, "Z/X: Select previous or next node.", 0, textcolor, surface, &rect, BTEXT_BGTRANSPARENT);
	rect.y += font->height;
	btext_renderToSurface(font, "C: Create new node.", 0, textcolor, surface, &rect, BTEXT_BGTRANSPARENT);
	rect.y += font->height;
	btext_renderToSurface(font, "V: Delete selected node.", 0, textcolor, surface, &rect, BTEXT_BGTRANSPARENT);
	rect.y += font->height;
	btext_renderToSurface(font, "W: Export ICC profile to out.icc.", 0, textcolor, surface, &rect, BTEXT_BGTRANSPARENT);
	rect.y += font->height;
	btext_renderToSurface(font, "Q: Quit", 0, textcolor, surface, &rect, BTEXT_BGTRANSPARENT);

	return(surface);
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

void plotgraph(Uint8 *points, Uint8 r, Uint8 g, Uint8 b) {
	int i;
	int pcolor = SDL_MapRGB(screen->format, r, g, b);

	SDL_LockSurface(screen);

	for(i = 0; i < 255; i++) {
		((Uint32 *)screen->pixels)[GETOFFSET(i, (255 - points[i]), screen)] = pcolor;
	}

	SDL_UnlockSurface(screen);
}

void drawramps(Uint8 *red, Uint8 *green, Uint8 *blue) {
	int i;
	SDL_Rect rect;
	rect.w = 1;
	rect.h = 100;

	rect.y = 0;
	for(i = 0; i <= 255; i++) {
		rect.x = 384 + i;
		SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, red[i], green[i], blue[i]));
	}
	rect.y = 100;
	for(i = 0; i <= 255; i++) {
		rect.x = 384 + i;
		SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, red[i], 0, 0));
	}
	rect.y = 200;
	for(i = 0; i <= 255; i++) {
		rect.x = 384 + i;
		SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 0, green[i], 0));
	}
	rect.y = 300;
	for(i = 0; i <= 255; i++) {
		rect.x = 384 + i;
		SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 0, 0, blue[i]));
	}
}

void drawscreen(Uint8 *red, Uint8 *green, Uint8 *blue, Mark *rmarks, Mark *gmarks, Mark *bmarks, Mark *marksel) {
	SDL_Rect rect;
	char buffer[32];
	Uint32 width;

	SDL_BlitSurface(bg, NULL, screen, NULL);

	plotgraph(red, 255, 0, 0);
	plotgraph(green, 0, 255, 0);
	plotgraph(blue, 0, 0, 255);
	while(rmarks != NULL) {
		rect.x = rmarks->pos - 1;
		rect.y = 255 - rmarks->value - 1;
		rect.w = 3;
		rect.h = 3;
		SDL_FillRect(screen, &rect, redcolor);
		rmarks = rmarks->next;
	}
	while(gmarks != NULL) {
		rect.x = gmarks->pos - 1;
		rect.y = 255 - gmarks->value - 1;
		rect.w = 3;
		rect.h = 3;
		SDL_FillRect(screen, &rect, greencolor);
		gmarks = gmarks->next;
	}
	while(bmarks != NULL) {
		rect.x = bmarks->pos - 1;
		rect.y = 255 - bmarks->value - 1;
		rect.w = 3;
		rect.h = 3;
		SDL_FillRect(screen, &rect, bluecolor);
		bmarks = bmarks->next;
	}
	rect.x = marksel->pos - 1;
	rect.y = 255 - marksel->value - 1;
	rect.w = 3;
	rect.h = 3;
	SDL_FillRect(screen, &rect, cursorcolor);
	snprintf(buffer, 32, "%d, %d", marksel->pos, marksel->value);
	width = btext_calcWidth(font, buffer);
	rect.x = marksel->pos + 2;
	rect.y = 255 - marksel->value + 2;
	rect.w = width + 2;
	rect.h = font->height + 2;
	switch(mode) {
		case RED:
			SDL_FillRect(screen, &rect, redcolor);
			break;
		case GREEN:
			SDL_FillRect(screen, &rect, greencolor);
			break;
		case BLUE:
			SDL_FillRect(screen, &rect, bluecolor);
			break;
	}
	rect.x++;
	rect.y++;
	rect.w = 0;
	btext_renderToSurface(font, buffer, colortextcolor, 0, screen, &rect, BTEXT_BGTRANSPARENT);

	drawramps(red, green, blue);

	rect.x = statusx;
	rect.y = screen->h - font->height;
	SDL_BlitSurface(statusmsg, NULL, screen, &rect);
	if(statusx > 1) {
		statusx *= 3;
		statusx /= 4;
	}
}

int saveicc(char *skeleton, char *iccfile, Uint8 *red, Uint8 *green, Uint8 *blue) {
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

	return(0);
}
