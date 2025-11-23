/* Image loading and rendering for Gamera */

typedef struct ImgInfo ImgInfo;
typedef struct ImgCache ImgCache;

/* Image information */
struct ImgInfo {
	char *url;          /* Image URL */
	Image *img;         /* Plan 9 libdraw Image */
	int width;
	int height;
	int loaded;         /* 1 if successfully loaded */
	ImgInfo *next;      /* For cache linked list */
};

/* Image cache */
struct ImgCache {
	ImgInfo *images;
	int nimages;
	int max_images;     /* Maximum cached images */
};

/* Initialize image cache */
ImgCache* imgcache_init(int max_images);

/* Free image cache */
void imgcache_free(ImgCache *cache);

/* Load image from URL (fetches and decodes) */
ImgInfo* img_load(const char *url, ImgCache *cache);

/* Load image from file path */
Image* img_load_file(const char *path);

/* Load image from data buffer */
Image* img_load_data(uchar *data, int len);

/* Get cached image or load if not cached */
ImgInfo* img_get(const char *url, ImgCache *cache);

/* Render image at position */
void img_render(Image *img, Point pos, Rectangle clip);

/* Get image dimensions without loading entire image */
int img_probe_size(const char *path, int *width, int *height);
