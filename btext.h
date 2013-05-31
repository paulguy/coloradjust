#include <SDL.h>

#define BTEXT_DEFAULT (0)
#define BTEXT_FGTRANSPARENT (1)
#define BTEXT_BGTRANSPARENT (2)

typedef struct {
	Uint8 *fontData[96];	/* 1 for on, 0 for off */
	Uint8 height;			/* global height */
	Uint8 widths[96];		/* array of char widths */
} bfont;

/*
 * Loads a font from an existing SDL_Surface
 *
 * surface		Surface containing font data.
 *				Font data consists of an image with a horizontal line of
 *				ASCII characters from 32 to 128.  The colors should be white
 *				on black.  The top row if pixels is not displayed, but is used
 *				as markers for the ends of each char.
 *
 * return		A bfont handle, or NULL on error.
 */
bfont *btext_loadFromSurface(SDL_Surface *surface);

/*
 * Loads a font from a BMP file using SDL_LoadBMP then calls
 * btext_loadFromSurface, then frees the surface.
 *
 * bmpfile		The file to load from.
 *
 * return		A bfont handle, or NULL on error.
 */
bfont *btext_loadFromBMP(char *bmpfile);

/*
 * Free a font handle.
 *
 * f			Font handle to be freed.
 */
void btext_free(bfont *f);

/*
 * Find the width of a line of text
 *
 * f			Font handle for width calculation.
 * text			String of text to be calculated.
 *
 * return		Width of text when rendered.
 */
Uint32 btext_calcWidth(bfont *f, char *text);

/*
 * Determine the number of chars in the string that will fit within a certain
 * width, in pixels.
 *
 * f			Font handle for width calculation.
 * text			String of text to be clipped.
 * limit		Width limit in pixels.
 * overlap		Zero if the last char should be fully visible.
 *				Non-zero if the last char can be partially clipped.
 *
 * return		Width in chars.
 */
Uint32 btext_clipTextToWidth(bfont *f, char *text, Uint32 limit, int overlap);

/*
 * Render text to it's own new surface.
 *
 * f			Font handle for rendering.
 * text			String of text to be rendered.
 * bg			Background color or NULL for transparent.
 * fg			Foreground color or NULL for transparent.
 *
 * return		SDL_Surface containing render.
 */
/*SDL_Surface *btext_render(bfont *f, char *text, SDL_Color *bg, SDL_Color *fg);*/

/*
 * Render the font to a surface at a particular location with the specified color.
 *
 * f			Font handle for rendering.
 * text			String of text to be rendered.
 * bg			Background color, alpha of 0 for transparent.
 * fg			Foreground color, alpha of 0 for transparent.
 * surface		Surface to be rendered to.
 * rect			Rectangle describing the square within which text should
 *				appear.  w member set to 0 for clipping to entire surface,
 *				otherwise clips to specified rectangle or surface.  h is
 *				ignored.
 *
 * return		Zero on success.
 */
int btext_renderToSurface(bfont *f, char *text, Uint32 bgval, Uint32 fgval, SDL_Surface *surface, SDL_Rect *rect, Uint32 flags);

