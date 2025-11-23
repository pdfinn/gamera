#include <u.h>
#include <libc.h>
#include "../src/color.h"

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

#define ASSERT_COLOR_EQ(c, rr, gg, bb, aa) do { \
	ASSERT((c).r == (rr)); \
	ASSERT((c).g == (gg)); \
	ASSERT((c).b == (bb)); \
	ASSERT((c).a == (aa)); \
} while(0)

TEST(hex_rgb)
{
	Color c;
	int ret;

	/* #RGB format */
	ret = parse_color("#f00", &c);
	ASSERT(ret == 0);
	ASSERT_COLOR_EQ(c, 255, 0, 0, 255);

	ret = parse_color("#0f0", &c);
	ASSERT(ret == 0);
	ASSERT_COLOR_EQ(c, 0, 255, 0, 255);

	ret = parse_color("#00f", &c);
	ASSERT(ret == 0);
	ASSERT_COLOR_EQ(c, 0, 0, 255, 255);

	ret = parse_color("#abc", &c);
	ASSERT(ret == 0);
	ASSERT_COLOR_EQ(c, 0xaa, 0xbb, 0xcc, 255);
}

TEST(hex_rrggbb)
{
	Color c;
	int ret;

	/* #RRGGBB format */
	ret = parse_color("#ff0000", &c);
	ASSERT(ret == 0);
	ASSERT_COLOR_EQ(c, 255, 0, 0, 255);

	ret = parse_color("#00ff00", &c);
	ASSERT(ret == 0);
	ASSERT_COLOR_EQ(c, 0, 255, 0, 255);

	ret = parse_color("#0000ff", &c);
	ASSERT(ret == 0);
	ASSERT_COLOR_EQ(c, 0, 0, 255, 255);

	ret = parse_color("#123456", &c);
	ASSERT(ret == 0);
	ASSERT_COLOR_EQ(c, 0x12, 0x34, 0x56, 255);

	/* Case insensitive */
	ret = parse_color("#AbCdEf", &c);
	ASSERT(ret == 0);
	ASSERT_COLOR_EQ(c, 0xab, 0xcd, 0xef, 255);
}

TEST(rgb_function)
{
	Color c;
	int ret;

	/* rgb() with numbers */
	ret = parse_color("rgb(255, 0, 0)", &c);
	ASSERT(ret == 0);
	ASSERT_COLOR_EQ(c, 255, 0, 0, 255);

	ret = parse_color("rgb(0, 255, 0)", &c);
	ASSERT(ret == 0);
	ASSERT_COLOR_EQ(c, 0, 255, 0, 255);

	ret = parse_color("rgb(128, 64, 32)", &c);
	ASSERT(ret == 0);
	ASSERT_COLOR_EQ(c, 128, 64, 32, 255);

	/* rgb() with percentages */
	ret = parse_color("rgb(100%, 0%, 0%)", &c);
	ASSERT(ret == 0);
	ASSERT_COLOR_EQ(c, 255, 0, 0, 255);

	ret = parse_color("rgb(50%, 50%, 50%)", &c);
	ASSERT(ret == 0);
	/* 50% of 255 = 127.5, rounded to 127 */
	ASSERT_COLOR_EQ(c, 127, 127, 127, 255);
}

TEST(rgba_function)
{
	Color c;
	int ret;

	/* rgba() with alpha */
	ret = parse_color("rgba(255, 0, 0, 1.0)", &c);
	ASSERT(ret == 0);
	ASSERT_COLOR_EQ(c, 255, 0, 0, 255);

	ret = parse_color("rgba(0, 255, 0, 0.5)", &c);
	ASSERT(ret == 0);
	ASSERT_COLOR_EQ(c, 0, 255, 0, 127);

	ret = parse_color("rgba(0, 0, 255, 0.0)", &c);
	ASSERT(ret == 0);
	ASSERT_COLOR_EQ(c, 0, 0, 255, 0);
}

TEST(named_colors)
{
	Color c;
	int ret;

	/* Basic colors */
	ret = parse_color("red", &c);
	ASSERT(ret == 0);
	ASSERT_COLOR_EQ(c, 255, 0, 0, 255);

	ret = parse_color("green", &c);
	ASSERT(ret == 0);
	ASSERT_COLOR_EQ(c, 0, 128, 0, 255);

	ret = parse_color("blue", &c);
	ASSERT(ret == 0);
	ASSERT_COLOR_EQ(c, 0, 0, 255, 255);

	/* Case insensitive */
	ret = parse_color("RED", &c);
	ASSERT(ret == 0);
	ASSERT_COLOR_EQ(c, 255, 0, 0, 255);

	ret = parse_color("Black", &c);
	ASSERT(ret == 0);
	ASSERT_COLOR_EQ(c, 0, 0, 0, 255);

	ret = parse_color("WHITE", &c);
	ASSERT(ret == 0);
	ASSERT_COLOR_EQ(c, 255, 255, 255, 255);

	/* Transparent */
	ret = parse_color("transparent", &c);
	ASSERT(ret == 0);
	ASSERT_COLOR_EQ(c, 0, 0, 0, 0);
}

TEST(invalid_colors)
{
	Color c;
	int ret;

	/* Invalid formats */
	ret = parse_color("", &c);
	ASSERT(ret == -1);

	ret = parse_color("#", &c);
	ASSERT(ret == -1);

	ret = parse_color("#12", &c);
	ASSERT(ret == -1);

	ret = parse_color("#1234", &c);
	ASSERT(ret == -1);

	ret = parse_color("#12345", &c);
	ASSERT(ret == -1);

	ret = parse_color("#1234567", &c);
	ASSERT(ret == -1);

	ret = parse_color("rgb()", &c);
	ASSERT(ret == -1);

	ret = parse_color("rgb(256, 0, 0)", &c);
	ASSERT(ret == 0); /* Should clamp to 255 */
	ASSERT(c.r == 255);

	ret = parse_color("notacolor", &c);
	ASSERT(ret == -1);
}

TEST(whitespace_handling)
{
	Color c;
	int ret;

	/* Leading/trailing whitespace */
	ret = parse_color("  #ff0000  ", &c);
	ASSERT(ret == 0);
	ASSERT_COLOR_EQ(c, 255, 0, 0, 255);

	ret = parse_color("  red  ", &c);
	ASSERT(ret == 0);
	ASSERT_COLOR_EQ(c, 255, 0, 0, 255);

	/* Whitespace in rgb() */
	ret = parse_color("rgb( 255 , 128 , 64 )", &c);
	ASSERT(ret == 0);
	ASSERT_COLOR_EQ(c, 255, 128, 64, 255);
}

void
main(void)
{
	fprint(2, "=== Color Parser Tests ===\n");

	RUN_TEST(hex_rgb);
	RUN_TEST(hex_rrggbb);
	RUN_TEST(rgb_function);
	RUN_TEST(rgba_function);
	RUN_TEST(named_colors);
	RUN_TEST(invalid_colors);
	RUN_TEST(whitespace_handling);

	fprint(2, "\n=== All tests passed! ===\n");
	exits(nil);
}
