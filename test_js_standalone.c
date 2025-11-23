/* Standalone test of JavaScript engine */
#include <u.h>
#include <libc.h>
#include "src/js.c"

void
main(void)
{
    JSContext *ctx;

    print("Testing JavaScript engine...\n");

    ctx = js_init();
    if(ctx == nil)
        sysfatal("js_init failed");

    print("Test 1: console.log\n");
    js_eval(ctx, "console.log('Hello from JavaScript!')");

    print("\nTest 2: Variables\n");
    js_eval(ctx, "var x = 'test'; console.log(x)");

    print("\nTest 3: Multiple statements\n");
    js_eval(ctx, "var a = 'foo'; var b = 'bar'; console.log(a); console.log(b)");

    js_cleanup(ctx);
    print("\nAll tests completed\n");
    exits(nil);
}
