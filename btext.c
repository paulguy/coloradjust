#include <stdio.h>

#include "btext.h"

#define PIXELS(SURFACE)	((Uint32 *)(SURFACE->pixels))
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

bfont *btext_loadFromSurface(SDL_Surface *fontsurface) {
	bfont *f;
	Uint32 black;
	Uint32 x, y, c;
	Uint8 *data;
	Uint32 offsets[96];
	Uint32 pix, lstart, cpix;

	SDL_Surface *surface, *surface32;

	if(fontsurface->h < 2)
		return(NULL);

	/* convert surface to 32 bit for easier operation */
	surface32 = SDL_CreateRGBSurface(SDL_SWSURFACE, 1, 1, 32, RMASK, GMASK, BMASK, AMASK);
	if(surface32 == NULL)
		return(NULL);
	surface = SDL_ConvertSurface(fontsurface, surface32->format, SDL_SWSURFACE);
	SDL_FreeSurface(surface32);
	if(surface == NULL)
		return(NULL);

	f = malloc(sizeof(bfont));
	if(f == NULL) {
		SDL_FreeSurface(surface);
		return(NULL);
	}

	f->height = surface->h - 1;

	black = SDL_MapRGB(surface->format, 0, 0, 0);

	if(SDL_LockSurface(surface) < 0) {
		free(f);
		SDL_FreeSurface(surface);
		return(NULL);
	}
/* ----- Find Widths ----- */
	for(c = 0; c < 96; c++)
		(f->widths)[c] = 1;
	c = 0;
	offsets[0] = 0;
	for(x = 0; x < surface->w; x++) {
		if(PIXELS(surface)[x] == black)
			((f->widths)[c])++;
		else {
			c++;
			if(c == 96)
				break;
			offsets[c] = offsets[c - 1] + (f->widths)[c - 1];
		}
	}
	if(c != 96) {
		free(f);
		return(NULL);
	}

	data = malloc(sizeof(Uint8) * (x + 1) * f->height);
	if(data == NULL) {
		free(f);
		return(NULL);
	}

/* ----- Convert data -----
 * Format:
 * fontdata[character][pixel data offset]
 * Pixel data is stored starting at each char's top left corner and goes to the right for width, then down for height
 * Pixel value is 0 for background and 1 for foreground
 */
	for(c = 0; c < 96; c++) {
		lstart = pix = surface->w + offsets[c]; /* skip top line and set pointer to top left pixel of char and set line start pointer */
		cpix = 0; /* set font data pointer to start */
		(f->fontData)[c] = &(data[offsets[c] * f->height]); /* allocate memory for glyph */
		for(y = 0; y < f->height; y++) {
			for(x = 0; x < (f->widths)[c]; x++) {
				if(PIXELS(surface)[pix] == black) {
					(f->fontData)[c][cpix] = 0;
					cpix++;
				} else {
					(f->fontData)[c][cpix] = 1;
					cpix++;
				}
				pix++; /* next pixel */
			}
			lstart = pix = lstart + surface->w; /* next line and update line start pointer */
		}
	}

	SDL_UnlockSurface(surface);
	SDL_FreeSurface(surface);

	return(f);
}

bfont *btext_loadFromBMP(char *bmpfile) {
	bfont *f;
	SDL_Surface *surface;

	surface = SDL_LoadBMP(bmpfile);
	if(surface == NULL)
		return(NULL);
	if((f = btext_loadFromSurface(surface)) == NULL)
		return(NULL);
	SDL_FreeSurface(surface);

	return(f);
}
	
void btext_free(bfont *f) {
	free((f->fontData)[0]);
	free(f);
}

Uint32 btext_calcWidth(bfont *f, char *text) {
	Uint32 width, offset;
	unsigned char *utext;

	utext = (unsigned char *)text;

	offset = width = 0;
	while(utext[offset] != '\0') {
		if(utext[offset] >= ' ' && utext[offset] <= '\x7f')
			width += f->widths[utext[offset] - 32];
		else
			width += f->widths[95];
		offset++;
	}

	return(width);
}

Uint32 btext_clipTextToWidth(bfont *f, char *text, Uint32 limit, int overlap) {
	Uint32 width, offset;
	unsigned char *utext;

	utext = (unsigned char *)text;

	offset = width = 0;
	while(utext[offset] != '\0') {
		if(utext[offset] >= ' ' && utext[offset] <= '\x7f')
			width += f->widths[utext[offset] - 32];
		else
			width += f->widths[95];
		if(width > limit) {
			if(overlap == 0)
				return(offset - 1);
			return(offset);
		}
		offset++;
	}

	return(offset);
}

/*SDL_Surface *btext_render(bfont *f, char *text, SDL_Color *bg, SDL_Color *fg);*/

int btext_renderToSurface(bfont *f, char *text, Uint32 bgval, Uint32 fgval, SDL_Surface *surface, SDL_Rect *rect, Uint32 flags) {
	Uint32 length;
	Uint32 c, x, y;
	Uint32 start, lstart, blitoffset;
	Uint32 dataoffset;
	unsigned char *utext;

	if(rect->w == 0 || rect->w > surface->w - rect->x)
		rect->w = surface->w - rect->x; /* clip to surface dimensions */
	length = btext_clipTextToWidth(f, text, rect->w, 1);
	if(length == 0)

	if(SDL_LockSurface(surface) < 0)
		return(-1);

	utext = (unsigned char *)text;
	start = (rect->y * surface->w) + rect->x;
	for(c = 0; c < length; c++) {
		lstart = start;
		dataoffset = 0;
		if(utext[c] >= ' ' && utext[c] <= '\x7f') {
			for(y = 0; y < f->height; y++) {
				blitoffset = lstart;
				for(x = 0; x < (f->widths)[utext[c] - 32]; x++) {
					if((f->fontData)[utext[c] - 32][dataoffset] == 0) {
						if(!(flags & BTEXT_BGTRANSPARENT))
							PIXELS(surface)[blitoffset] = bgval;
					} else {
						if(!(flags & BTEXT_FGTRANSPARENT))
							PIXELS(surface)[blitoffset] = fgval;
					}
					blitoffset++;
					dataoffset++;
				}
				lstart += surface->w;
			}
			start += (f->widths)[text[c] - 32];
		}
	}

	SDL_UnlockSurface(surface);

	return(0);
}
