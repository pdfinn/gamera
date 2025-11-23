#include <u.h>
#include <libc.h>
#include <bio.h>
#include "js.h"

/*
 * Minimal JavaScript interpreter for Gamera
 * Handles common JavaScript patterns for DOM manipulation
 * Designed following Plan 9 principles: simple, understandable, sufficient
 */

#define MAX_NODES 1024
#define MAX_VARS 256
#define MAX_FUNCTIONS 128

typedef struct Variable {
    char *name;
    char *value;
} Variable;

typedef struct Function {
    char *name;
    char *body;
} Function;

struct JSContext {
    JSNode *dom_root;
    JSNode *nodes[MAX_NODES];
    int nnodes;
    Variable vars[MAX_VARS];
    int nvars;
    Function funcs[MAX_FUNCTIONS];
    int nfuncs;
    void (*console_log)(const char*);
};

static void default_console_log(const char *msg);
static char* js_eval_expr(JSContext *ctx, const char *expr);
static void js_exec_statement(JSContext *ctx, const char *stmt);

/*
 * Initialize JavaScript context
 */
JSContext*
js_init(void)
{
    JSContext *ctx;

    ctx = mallocz(sizeof(JSContext), 1);
    if(ctx == nil)
        return nil;

    ctx->console_log = default_console_log;
    ctx->nnodes = 0;
    ctx->nvars = 0;
    ctx->nfuncs = 0;
    return ctx;
}

/*
 * Cleanup JavaScript context
 */
void
js_cleanup(JSContext *ctx)
{
    int i;

    if(ctx == nil)
        return;

    /* Free variables */
    for(i = 0; i < ctx->nvars; i++){
        free(ctx->vars[i].name);
        free(ctx->vars[i].value);
    }

    /* Free functions */
    for(i = 0; i < ctx->nfuncs; i++){
        free(ctx->funcs[i].name);
        free(ctx->funcs[i].body);
    }

    free(ctx);
}

/*
 * Default console.log implementation
 */
static void
default_console_log(const char *msg)
{
    fprint(2, "[JS] %s\n", msg);
}

/*
 * Set custom console handler
 */
void
js_set_console_handler(JSContext *ctx, void (*handler)(const char*))
{
    if(ctx && handler)
        ctx->console_log = handler;
}

/*
 * Set up DOM tree for JavaScript access
 */
void
js_setup_dom(JSContext *ctx, JSNode *root)
{
    if(ctx == nil)
        return;
    ctx->dom_root = root;
    ctx->nnodes = 0;

    /* Register root node */
    if(ctx->nnodes < MAX_NODES){
        ctx->nodes[ctx->nnodes++] = root;
    }
}

/*
 * Get element by ID from DOM tree
 */
JSNode*
js_get_element_by_id(JSContext *ctx, const char *id)
{
    int i;
    JSNode *node;

    if(ctx == nil || id == nil)
        return nil;

    /* Search registered nodes for matching ID */
    for(i = 0; i < ctx->nnodes; i++){
        node = ctx->nodes[i];
        if(node->id && strcmp(node->id, id) == 0)
            return node;
    }

    return nil;
}

/*
 * Set variable value
 */
static void
js_set_var(JSContext *ctx, const char *name, const char *value)
{
    int i;

    /* Check if variable exists */
    for(i = 0; i < ctx->nvars; i++){
        if(strcmp(ctx->vars[i].name, name) == 0){
            free(ctx->vars[i].value);
            ctx->vars[i].value = strdup(value);
            return;
        }
    }

    /* Add new variable */
    if(ctx->nvars < MAX_VARS){
        ctx->vars[ctx->nvars].name = strdup(name);
        ctx->vars[ctx->nvars].value = strdup(value);
        ctx->nvars++;
    }
}

/*
 * Get variable value
 */
static char*
js_get_var(JSContext *ctx, const char *name)
{
    int i;

    for(i = 0; i < ctx->nvars; i++){
        if(strcmp(ctx->vars[i].name, name) == 0)
            return ctx->vars[i].value;
    }
    return nil;
}

/*
 * Handle console.log() calls
 */
static void
handle_console_log(JSContext *ctx, const char *args)
{
    char *msg, *eval;

    /* Evaluate the argument */
    eval = js_eval_expr(ctx, args);
    msg = eval ? eval : (char*)args;

    if(ctx->console_log)
        ctx->console_log(msg);

    if(eval)
        free(eval);
}

/*
 * Handle getElementById() calls
 */
static char*
handle_get_element_by_id(JSContext *ctx, const char *id_expr)
{
    JSNode *node;
    char *id;
    static char result[256];

    /* Strip quotes from ID */
    id = (char*)id_expr;
    if(*id == '"' || *id == '\''){
        id = strdup(id + 1);
        if(id[strlen(id)-1] == '"' || id[strlen(id)-1] == '\'')
            id[strlen(id)-1] = 0;
    }

    node = js_get_element_by_id(ctx, id);
    if(node){
        snprint(result, sizeof result, "[Element:%s]", node->id);
        return strdup(result);
    }

    return strdup("null");
}

/*
 * Evaluate a simple JavaScript expression
 * Handles: variables, strings, numbers, basic function calls
 */
static char*
js_eval_expr(JSContext *ctx, const char *expr)
{
    char *p, *result, *trimmed;

    if(expr == nil)
        return nil;

    /* Trim whitespace */
    while(*expr == ' ' || *expr == '\t')
        expr++;

    trimmed = strdup(expr);
    p = trimmed + strlen(trimmed) - 1;
    while(p > trimmed && (*p == ' ' || *p == '\t' || *p == ';'))
        *p-- = 0;

    /* String literal */
    if(*trimmed == '"' || *trimmed == '\''){
        result = strdup(trimmed + 1);
        if(result[strlen(result)-1] == '"' || result[strlen(result)-1] == '\'')
            result[strlen(result)-1] = 0;
        free(trimmed);
        return result;
    }

    /* Number */
    if(*trimmed >= '0' && *trimmed <= '9'){
        free(trimmed);
        return strdup(expr);
    }

    /* console.log() */
    if(strncmp(trimmed, "console.log(", 12) == 0){
        p = strchr(trimmed + 12, ')');
        if(p){
            *p = 0;
            handle_console_log(ctx, trimmed + 12);
        }
        free(trimmed);
        return nil;
    }

    /* document.getElementById() */
    if(strncmp(trimmed, "document.getElementById(", 24) == 0){
        p = strchr(trimmed + 24, ')');
        if(p){
            *p = 0;
            result = handle_get_element_by_id(ctx, trimmed + 24);
            free(trimmed);
            return result;
        }
    }

    /* Variable reference */
    result = js_get_var(ctx, trimmed);
    free(trimmed);
    return result ? strdup(result) : nil;
}

/*
 * Execute a JavaScript statement
 */
static void
js_exec_statement(JSContext *ctx, const char *stmt)
{
    char *p, *name, *value, *expr;

    if(stmt == nil || *stmt == 0)
        return;

    /* Skip whitespace */
    while(*stmt == ' ' || *stmt == '\t' || *stmt == '\n')
        stmt++;

    /* Variable assignment: var x = value; */
    if(strncmp(stmt, "var ", 4) == 0){
        stmt += 4;
        while(*stmt == ' ') stmt++;

        p = strchr(stmt, '=');
        if(p){
            name = strndup(stmt, p - stmt);
            /* Trim name */
            while(name[strlen(name)-1] == ' ')
                name[strlen(name)-1] = 0;

            value = js_eval_expr(ctx, p + 1);
            js_set_var(ctx, name, value ? value : "");
            free(name);
            if(value) free(value);
        }
        return;
    }

    /* Assignment: x = value; */
    p = strchr(stmt, '=');
    if(p && *(p-1) != '!' && *(p-1) != '=' && *(p+1) != '='){
        name = strndup(stmt, p - stmt);
        /* Trim name */
        while(name[strlen(name)-1] == ' ')
            name[strlen(name)-1] = 0;

        value = js_eval_expr(ctx, p + 1);
        js_set_var(ctx, name, value ? value : "");
        free(name);
        if(value) free(value);
        return;
    }

    /* Expression statement (function call, etc.) */
    expr = js_eval_expr(ctx, stmt);
    if(expr)
        free(expr);
}

/*
 * Execute JavaScript code (multiple statements)
 */
int
js_eval(JSContext *ctx, const char *code)
{
    char *copy, *stmt, *next;

    if(ctx == nil || code == nil)
        return -1;

    copy = strdup(code);
    stmt = copy;

    /* Split by semicolons and execute each statement */
    while(stmt && *stmt){
        next = strchr(stmt, ';');
        if(next){
            *next = 0;
            next++;
        }

        js_exec_statement(ctx, stmt);
        stmt = next;
    }

    free(copy);
    return 0;
}

/*
 * Execute inline script from HTML
 */
int
js_exec_script(JSContext *ctx, const char *script)
{
    return js_eval(ctx, script);
}

/*
 * Bind event handler to DOM node
 */
void
js_bind_event(JSContext *ctx, JSNode *node, const char *event, const char *handler)
{
    /* Store event handler for later execution */
    /* For now, we'll execute immediately when event fires */
    USED(ctx);
    USED(node);
    USED(event);
    USED(handler);

    /* TODO: Implement event queue and handler storage */
}
