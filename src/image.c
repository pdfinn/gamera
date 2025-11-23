#include <u.h>
#include <libc.h>
#include <bio.h>
#include <draw.h>
#include "image.h"

/*
 * Initialize image cache
 */
ImgCache*
imgcache_init(int max_images)
{
	ImgCache *cache;

	cache = mallocz(sizeof(ImgCache), 1);
	if(!cache)
		return nil;

	cache->max_images = max_images > 0 ? max_images : 100;
	cache->nimages = 0;
	cache->images = nil;

	return cache;
}

/*
 * Free image cache
 */
void
imgcache_free(ImgCache *cache)
{
	ImgInfo *img, *next;

	if(!cache)
		return;

	img = cache->images;
	while(img){
		next = img->next;
		free(img->url);
		if(img->img)
			freeimage(img->img);
		free(img);
		img = next;
	}

	free(cache);
}

/*
 * Detect image format from file header
 */
static int
detect_format(uchar *data, int len)
{
	/* PNG: starts with 0x89 0x50 0x4E 0x47 */
	if(len >= 4 && data[0] == 0x89 && data[1] == 0x50 &&
	   data[2] == 0x4E && data[3] == 0x47)
		return 1; /* PNG */

	/* JPEG: starts with 0xFF 0xD8 */
	if(len >= 2 && data[0] == 0xFF && data[1] == 0xD8)
		return 2; /* JPEG */

	/* GIF: starts with "GIF87a" or "GIF89a" */
	if(len >= 6 && memcmp(data, "GIF8", 4) == 0 &&
	   (data[4] == '7' || data[4] == '9') && data[5] == 'a')
		return 3; /* GIF */

	return 0; /* Unknown */
}

/*
 * Load image from file using Plan 9 readimage
 * This is a simplified version - Plan 9 has more sophisticated
 * image loading in libdraw
 */
Image*
img_load_file(const char *path)
{
	int fd;
	Image *img;

	if(!path || !display)
		return nil;

	fd = open(path, OREAD);
	if(fd < 0){
		fprint(2, "img_load_file: cannot open %s: %r\n", path);
		return nil;
	}

	/* Use Plan 9's readimage to load the image */
	img = readimage(display, fd, 0);
	close(fd);

	if(!img){
		fprint(2, "img_load_file: failed to read image %s: %r\n", path);
		return nil;
	}

	return img;
}

/*
 * Load image from data buffer
 * Write to temp file and use readimage
 */
Image*
img_load_data(uchar *data, int len)
{
	int fd;
	Image *img;
	char tmpfile[256];
	int format;

	if(!data || len <= 0 || !display)
		return nil;

	format = detect_format(data, len);
	if(format == 0){
		fprint(2, "img_load_data: unknown image format\n");
		return nil;
	}

	/* Create temporary file */
	snprint(tmpfile, sizeof tmpfile, "/tmp/gamera-img-%d.tmp", getpid());

	fd = create(tmpfile, OWRITE, 0600);
	if(fd < 0){
		fprint(2, "img_load_data: cannot create temp file: %r\n");
		return nil;
	}

	/* Write image data */
	if(write(fd, data, len) != len){
		fprint(2, "img_load_data: write failed: %r\n");
		close(fd);
		remove(tmpfile);
		return nil;
	}
	close(fd);

	/* Load image from temp file */
	img = img_load_file(tmpfile);

	/* Clean up temp file */
	remove(tmpfile);

	return img;
}

/*
 * Get image dimensions without fully loading
 * For now, just load and measure - could be optimized
 */
int
img_probe_size(const char *path, int *width, int *height)
{
	Image *img;

	img = img_load_file(path);
	if(!img)
		return -1;

	*width = Dx(img->r);
	*height = Dy(img->r);

	freeimage(img);
	return 0;
}

/*
 * Load image from URL
 * This is a stub - full implementation would fetch via HTTP
 */
ImgInfo*
img_load(const char *url, ImgCache *cache)
{
	ImgInfo *info;
	Image *img;

	if(!url)
		return nil;

	/* For now, treat URL as file path for testing */
	/* TODO: Integrate with fetcher.c for HTTP downloads */

	img = img_load_file(url);
	if(!img)
		return nil;

	/* Create info structure */
	info = mallocz(sizeof(ImgInfo), 1);
	if(!info){
		freeimage(img);
		return nil;
	}

	info->url = strdup(url);
	info->img = img;
	info->width = Dx(img->r);
	info->height = Dy(img->r);
	info->loaded = 1;

	/* Add to cache if provided */
	if(cache){
		/* If cache is full, remove oldest (simple FIFO for now) */
		if(cache->nimages >= cache->max_images){
			/* TODO: Implement LRU eviction */
			fprint(2, "img_load: cache full, not caching\n");
		} else {
			info->next = cache->images;
			cache->images = info;
			cache->nimages++;
		}
	}

	return info;
}

/*
 * Get cached image or load if not cached
 */
ImgInfo*
img_get(const char *url, ImgCache *cache)
{
	ImgInfo *info;

	if(!url)
		return nil;

	/* Search cache */
	if(cache){
		for(info = cache->images; info; info = info->next){
			if(strcmp(info->url, url) == 0)
				return info;
		}
	}

	/* Not in cache - load it */
	return img_load(url, cache);
}

/*
 * Render image at position
 */
void
img_render(Image *img, Point pos, Rectangle clip)
{
	Rectangle r;

	if(!img || !screen)
		return;

	/* Destination rectangle */
	r.min = pos;
	r.max = addpt(pos, Pt(Dx(img->r), Dy(img->r)));

	/* Clip to screen bounds */
	if(clip.min.x != clip.max.x)
		r = rectclip(r, clip);

	/* Draw image */
	draw(screen, r, img, nil, img->r.min);
	flushimage(display, 1);
}
