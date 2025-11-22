/* font.h - Font management interface */

typedef enum {
    FONT_REGULAR = 0,
    FONT_MONOSPACE = 1
} FontType;

int font_init(void);
void font_cleanup(void);
Font* font_get_current(void);
void font_set_current(FontType type);
