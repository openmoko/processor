#ifndef PSR_CONSTANTS_H
#define PSR_CONSTANTS_H

/* constants */

#include <math.h>

// useful goodness

#define PI M_PI
#define HALF_PI M_PI_2
#define THIRD_PI (M_PI / 3.0f)
#define QUARTER_PI M_PI_4
#define TWO_PI (M_PI * 2.0f)

#define DEG_TO_RAD (PI/180.0f)
#define RAD_TO_DEG (180.0f/PI)

// for colors and/or images

#define RGB (1);		// image & color
#define ARGB (2);		// image
#define HSB (3);		// color
#define ALPHA (4);		// image

// image file types

#define TIFF (0)
#define TARGA (1)
#define JPEG (2)
#define GIF (3)

// filter/convert types

#define BLUR (11)
#define GRAY (12)
#define INVERT (13)
#define OPAQUE (14)
#define POSTERIZE (15)
#define THRESHOLD (16)
#define ERODE (17)
#define DILATE (18)

// blend mode keyword definitions

#define REPLACE (0)
#define BLEND (1 << 0)
#define ADD (1 << 1)
#define SUBTRACT (1 << 2)
#define LIGHTEST (1 << 3)
#define DARKEST (1 << 4)
#define DIFFERENCE (1 << 5)
#define EXCLUSION (1 << 6)
#define MULTIPLY (1 << 7)
#define SCREEN (1 << 8)
#define OVERLAY (1 << 9)
#define HARD_LIGHT (1 << 10)
#define SOFT_LIGHT (1 << 11)
#define DODGE (1 << 12)
#define BURN (1 << 13)

// colour component bitmasks

#define ALPHA_MASK (0xff000000)
#define RED_MASK (0x00ff0000)
#define GREEN_MASK (0x0000ff00)
#define BLUE_MASK (0x000000ff)


// for messages

#define CHATTER (0)
#define COMPLAINT (1)
#define PROBLEM (2)

// types of projection matrices


#define CUSTOM (0)		// user-specified fanciness
#define ORTHOGRAPHIC (2)	// 2D isometric projection
#define PERSPECTIVE (3)		// perspective matrix


// rendering settings

#define PIXEL_CENTER (0.5f)	// for polygon aa


// shapes

// the low four bits set the variety,
// higher bits set the specific shape type

#define POINTS ((1 << 4) | 0)

#define LINES ((1 << 5) | 0)
//static final int LINE_STRIP ((1 << 5) | 1)
//static final int LINE_LOOP ((1 << 5) | 2)

#define TRIANGLES ((1 << 6) | 0)
#define TRIANGLE_STRIP ((1 << 6) | 1)
#define TRIANGLE_FAN ((1 << 6) | 2)

#define QUADS ((1 << 7) | 0)
#define QUAD_STRIP ((1 << 7) | 1)

#define POLYGON ((1 << 8) | 0)
//static final int CONCAVE_POLYGON ((1 << 8) | 1)
//static final int CONVEX_POLYGON ((1 << 8) | 2)

#define OPEN (1)
#define CLOSE (2)


// shape drawing modes

/** Draw mode convention to use (x, y) to (width, height) */
#define CORNER (0)
/** Draw mode convention to use (x1, y1) to (x2, y2) coordinates */
#define CORNERS (1)
/** @deprecated Use RADIUS instead (as of 0125) */
#define CENTER_RADIUS (2)
/** Draw mode from the center, and using the radius */
#define RADIUS (2)
/** Draw from the center, using second pair of values as the diameter.
    Formerly called CENTER_DIAMETER in alpha releases */
#define CENTER (3)


// vertically alignment modes for text

/** Default vertical alignment for text placement */
#define BASELINE (0)
/** Align text to the top */
#define TOP (101)
/** Align text from the bottom, using the baseline. */
#define BOTTOM (102)


// uv texture orientation modes

//_SPACE 0;  // 0..1
#define NORMALIZED (1)
#define IMAGE (2)


// text placement modes

/**
 * textMode(MODEL) is the default, meaning that characters
 * will be affected by transformations like any other shapes.
 * <p/>
 * Changed value in 0093 to not interfere with LEFT, CENTER, and RIGHT.
 */
#define MODEL (4)

/**
 * textMode(SHAPE) draws text using the the glyph outlines of
 * individual characters rather than as textures. If the outlines are
 * not available, then textMode(SHAPE) will be ignored and textMode(MODEL)
 * will be used instead. For this reason, be sure to call textMode()
 * <EM>after</EM> calling textFont().
 * <p/>
 * Currently, textMode(SHAPE) is only supported by OPENGL mode.
 * It also requires Java 1.2 or higher (OPENGL requires 1.4 anyway)
 */
#define SHAPE (5)


// text alignment modes
// are inherited from LEFT, CENTER, RIGHT


// stroke modes

#define SQUARE (1 << 0)		// called 'butt' in the svg spec
#define ROUND (1 << 1)
#define PROJECT 1 << 2		// called 'square' in the svg spec
#define MITER (1 << 3)
#define BEVEL (1 << 5)


// lighting

#define AMBIENT (0)
#define DIRECTIONAL (1)
#define POINT (2)
#define SPOT (3)


// key constants

// only including the most-used of these guys
// if people need more esoteric keys, they can learn about
// the esoteric java KeyEvent api and of virtual keys

// both key and keyCode will equal these values
// for 0125, these were changed to 'char' values, because they
// can be upgraded to ints automatically by Java, but having them
// as ints prevented split(blah, TAB) from working
#define BACKSPACE (8)
#define TAB (9)
#define ENTER (10)
#define RETURN (13)
#define ESC (27)
#define DELETE (127)

// i.e. if ((key  CODED) && (keyCode  UP))
#define CODED (~((char)0))

// key will be CODED and keyCode will be this value
#define UP (0xff52)
#define DOWN (0xff54)
#define LEFT (0xff51)
#define RIGHT (0xff53)

// key will be CODED and keyCode will be this value
#define ALT (0xffe9)
#define CONTROL (0xffe3)
#define SHIFT (0xffe1)


// cursor types

#define ARROW (0)
#define CROSS (1)
#define HAND (2)
#define MOVE (3)
#define TEXT (4)
#define WAIT (5)


// hints

#define ENABLE_OPENGL_2X_SMOOTH (0)
#define ENABLE_OPENGL_4X_SMOOTH (1)
#define ENABLE_NATIVE_FONTS (2)
#define DISABLE_DEPTH_TEST (5)
#define DISABLE_FLYING_POO (6)
#define ENABLE_DEPTH_SORT (7)
#define DISABLE_ERROR_REPORT (8)
#define ENABLE_ACCURATE_TEXTURES (9)
#define DISABLE_AUTO_GZIP (10)

#define HINT_COUNT (11)

#endif /* PSR_CONSTANTS_H */
