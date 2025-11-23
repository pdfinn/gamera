/* js.h - JavaScript engine interface for Gamera */

#ifndef GAMERA_JS_H
#define GAMERA_JS_H

/* JavaScript context - opaque pointer to engine state */
typedef struct JSContext JSContext;

/* JavaScript value - represents any JS value */
typedef struct JSValue JSValue;

/* DOM node for JavaScript bindings */
typedef struct JSNode {
    char *tag;
    char *id;
    char *className;
    char *innerHTML;
    struct JSNode *parent;
    struct JSNode *firstChild;
    struct JSNode *nextSibling;
} JSNode;

/* Initialize JavaScript engine */
JSContext* js_init(void);

/* Cleanup JavaScript engine */
void js_cleanup(JSContext *ctx);

/* Execute JavaScript code */
int js_eval(JSContext *ctx, const char *code);

/* Set up DOM bindings for JavaScript */
void js_setup_dom(JSContext *ctx, JSNode *root);

/* Get element by ID (called from JS) */
JSNode* js_get_element_by_id(JSContext *ctx, const char *id);

/* Register console.log implementation */
void js_set_console_handler(JSContext *ctx, void (*handler)(const char*));

/* Execute inline script from HTML */
int js_exec_script(JSContext *ctx, const char *script);

/* Handle onclick and other event attributes */
void js_bind_event(JSContext *ctx, JSNode *node, const char *event, const char *handler);

#endif
