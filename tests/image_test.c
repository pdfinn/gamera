#include <u.h>
#include <libc.h>
#include <draw.h>
#include "../src/image.h"

#define TEST(name) void test_##name(void)
#define RUN_TEST(name) do { \
	fprint(2, "Running %s...\n", #name); \
	test_##name(); \
	fprint(2, "  PASS\n"); \
} while(0)

#define ASSERT(cond) do { \
	if(!(cond)) { \
		fprint(2, "ASSERTION FAILED: %s:%d: %s\n", __FILE__, __LINE__, #cond); \
		exits("test failed"); \
	} \
} while(0)

TEST(cache_init)
{
	ImgCache *cache;

	cache = imgcache_init(50);
	ASSERT(cache != nil);
	ASSERT(cache->max_images == 50);
	ASSERT(cache->nimages == 0);
	ASSERT(cache->images == nil);

	imgcache_free(cache);
}

TEST(cache_default_size)
{
	ImgCache *cache;

	cache = imgcache_init(0);
	ASSERT(cache != nil);
	ASSERT(cache->max_images == 100); /* default */

	imgcache_free(cache);
}

TEST(detect_png_header)
{
	uchar png_header[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

	/* This tests the internal detect_format function indirectly */
	/* We can't call it directly as it's static, but we know it's used */
	/* in img_load_data, so we test the behavior */

	/* For now, just test that invalid data doesn't crash */
	Image *img = img_load_data((uchar*)"invalid", 7);
	ASSERT(img == nil); /* Should fail gracefully */
}

TEST(detect_jpeg_header)
{
	uchar jpeg_header[] = {0xFF, 0xD8, 0xFF, 0xE0};

	/* Test with invalid JPEG data - should fail gracefully */
	Image *img = img_load_data(jpeg_header, sizeof jpeg_header);
	ASSERT(img == nil); /* Not enough data for valid JPEG */
}

TEST(detect_gif_header)
{
	uchar gif_header[] = {'G', 'I', 'F', '8', '9', 'a'};

	/* Test with invalid GIF data - should fail gracefully */
	Image *img = img_load_data(gif_header, sizeof gif_header);
	ASSERT(img == nil); /* Not enough data for valid GIF */
}

TEST(load_nonexistent_file)
{
	Image *img;

	img = img_load_file("/nonexistent/image.png");
	ASSERT(img == nil); /* Should return nil for missing file */
}

TEST(load_invalid_data)
{
	Image *img;
	uchar invalid_data[] = "This is not image data";

	img = img_load_data(invalid_data, sizeof invalid_data);
	ASSERT(img == nil); /* Should return nil for invalid data */
}

TEST(cache_operations)
{
	ImgCache *cache;
	ImgInfo *info;

	cache = imgcache_init(10);

	/* Try to get non-existent image */
	info = img_get("/nonexistent.png", cache);
	ASSERT(info == nil); /* Should return nil */

	/* Cache should still be valid */
	ASSERT(cache->nimages >= 0);

	imgcache_free(cache);
}

TEST(probe_size_nonexistent)
{
	int width, height;
	int ret;

	ret = img_probe_size("/nonexistent/image.png", &width, &height);
	ASSERT(ret == -1); /* Should fail for nonexistent file */
}

/*
 * NOTE: Full image loading tests require:
 * 1. Display to be initialized (display variable)
 * 2. Valid image files to load
 * 3. Graphics environment (X11/devdraw)
 *
 * These tests cover the error handling and basic structure.
 * Integration tests with real images should be run in a
 * graphical environment with test image files.
 */

TEST(null_parameter_handling)
{
	ImgCache *cache;
	ImgInfo *info;
	Image *img;

	/* Test nil parameters */
	cache = imgcache_init(10);

	img = img_load_file(nil);
	ASSERT(img == nil);

	img = img_load_data(nil, 0);
	ASSERT(img == nil);

	info = img_load(nil, cache);
	ASSERT(info == nil);

	info = img_get(nil, cache);
	ASSERT(info == nil);

	/* These should not crash */
	imgcache_free(nil);
	img_render(nil, Pt(0, 0), Rect(0, 0, 0, 0));

	imgcache_free(cache);
}

void
main(void)
{
	fprint(2, "=== Image Loading Tests ===\n");
	fprint(2, "NOTE: These tests cover error handling and structure.\n");
	fprint(2, "Full image loading requires graphics environment.\n\n");

	RUN_TEST(cache_init);
	RUN_TEST(cache_default_size);
	RUN_TEST(detect_png_header);
	RUN_TEST(detect_jpeg_header);
	RUN_TEST(detect_gif_header);
	RUN_TEST(load_nonexistent_file);
	RUN_TEST(load_invalid_data);
	RUN_TEST(cache_operations);
	RUN_TEST(probe_size_nonexistent);
	RUN_TEST(null_parameter_handling);

	fprint(2, "\n=== All image tests passed! ===\n");
	exits(nil);
}
