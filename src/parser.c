#include <u.h>
#include <libc.h>
#include "parser.h"

/*
 * extract_text removes HTML tags, CSS, scripts, and entities leaving clean text.
 * Returned string must be freed by the caller.
 */
char*
extract_text(const char *html)
{
    char *out;
    int len, capacity;
    int intag, inscript, instyle;
    const char *p;
    char c;

    if(html == nil)
        return nil;

    capacity = 1024;
    out = malloc(capacity);
    if(out == nil)
        return nil;
    
    len = 0;
    intag = 0;
    inscript = 0;
    instyle = 0;

    for(p = html; *p; p++){
        /* Check for script and style block starts */
        if(!intag && strncmp(p, "<script", 7) == 0){
            inscript = 1;
            intag = 1;
            continue;
        }
        if(!intag && strncmp(p, "<style", 6) == 0){
            instyle = 1;
            intag = 1;
            continue;
        }
        
        /* Check for script and style block ends */
        if(inscript && strncmp(p, "</script>", 9) == 0){
            inscript = 0;
            p += 8; /* Skip the closing tag */
            continue;
        }
        if(instyle && strncmp(p, "</style>", 8) == 0){
            instyle = 0;
            p += 7; /* Skip the closing tag */
            continue;
        }
        
        /* Skip everything inside script or style blocks */
        if(inscript || instyle)
            continue;
            
        /* Handle block elements that should create line breaks */
        if(!intag && (*p == '<')){
            /* Check for block elements that should add newlines */
            if(strncmp(p, "<p", 2) == 0 || strncmp(p, "<div", 4) == 0 || 
               strncmp(p, "<h1", 3) == 0 || strncmp(p, "<h2", 3) == 0 ||
               strncmp(p, "<h3", 3) == 0 || strncmp(p, "<h4", 3) == 0 ||
               strncmp(p, "<h5", 3) == 0 || strncmp(p, "<h6", 3) == 0 ||
               strncmp(p, "</p>", 4) == 0 || strncmp(p, "</div>", 6) == 0 ||
               strncmp(p, "</h1>", 5) == 0 || strncmp(p, "</h2>", 5) == 0 ||
               strncmp(p, "</h3>", 5) == 0 || strncmp(p, "</h4>", 5) == 0 ||
               strncmp(p, "</h5>", 5) == 0 || strncmp(p, "</h6>", 5) == 0 ||
               strncmp(p, "<br", 3) == 0){
                /* Add newline before block elements (except if we just added one) */
                if(len > 0 && out[len-1] != '\n'){
                    if(len >= capacity - 1){
                        capacity *= 2;
                        char *tmp = realloc(out, capacity);
                        if(tmp == nil){
                            free(out);
                            return nil;
                        }
                        out = tmp;
                    }
                    out[len++] = '\n';
                }
            }
        }
            
        /* Handle regular HTML tags */
        if(*p == '<'){
            intag = 1;
            continue;
        }
        if(*p == '>'){
            intag = 0;
            continue;
        }
        if(intag)
            continue;
            
        /* Handle HTML entities */
        if(*p == '&'){
            if(strncmp(p, "&nbsp;", 6) == 0){
                c = ' ';
                p += 5;
            } else if(strncmp(p, "&lt;", 4) == 0){
                c = '<';
                p += 3;
            } else if(strncmp(p, "&gt;", 4) == 0){
                c = '>';
                p += 3;
            } else if(strncmp(p, "&amp;", 5) == 0){
                c = '&';
                p += 4;
            } else if(strncmp(p, "&quot;", 6) == 0){
                c = '"';
                p += 5;
            } else {
                c = *p; /* Unknown entity, keep as-is */
            }
        } else {
            c = *p;
        }
        
        /* Skip control characters except newlines and tabs */
        if(c < ' ' && c != '\n' && c != '\t')
            continue;
            
        /* Convert multiple whitespace to single space, but preserve newlines */
        if(c == ' ' || c == '\t'){
            if(len > 0 && out[len-1] != ' ' && out[len-1] != '\n'){
                c = ' ';
            } else {
                continue; /* Skip redundant whitespace */
            }
        }
        
        /* Ensure we have enough capacity */
        if(len >= capacity - 1){
            capacity *= 2;
            char *tmp = realloc(out, capacity);
            if(tmp == nil){
                free(out);
                return nil;
            }
            out = tmp;
        }
        
        out[len++] = c;
    }
    
    /* Clean up trailing whitespace and add null terminator */
    while(len > 0 && (out[len-1] == ' ' || out[len-1] == '\t'))
        len--;
    out[len] = '\0';

    return out;
}

/*
 * extract_links finds <a href="..."> tags and extracts URLs and link text
 * Returns a linked list of Link structures
 */
Link*
extract_links(const char *html)
{
    Link *head = nil, *tail = nil, *link;
    const char *p, *href_start, *href_end, *text_start, *text_end;
    char *url, *text;
    int in_tag, in_link;

    if(html == nil)
        return nil;

    p = html;
    in_tag = 0;
    in_link = 0;
    href_start = href_end = text_start = text_end = nil;

    while(*p){
        if(*p == '<' && strncmp(p, "<a ", 3) == 0){
            /* Found start of link tag */
            in_tag = 1;
            in_link = 1;
            p += 3;
            
            /* Look for href attribute */
            while(*p && *p != '>'){
                if(strncmp(p, "href=\"", 6) == 0){
                    href_start = p + 6;
                    p += 6;
                    while(*p && *p != '"')
                        p++;
                    href_end = p;
                }
                p++;
            }
            
            if(*p == '>'){
                in_tag = 0;
                text_start = p + 1;
            }
        }
        else if(in_link && strncmp(p, "</a>", 4) == 0){
            /* Found end of link tag */
            text_end = p;
            
            /* Create link if we have both URL and text */
            if(href_start && href_end && text_start && text_end){
                url = malloc(href_end - href_start + 1);
                text = malloc(text_end - text_start + 1);
                if(url && text){
                    memmove(url, href_start, href_end - href_start);
                    url[href_end - href_start] = 0;
                    memmove(text, text_start, text_end - text_start);
                    text[text_end - text_start] = 0;
                    
                    link = malloc(sizeof(Link));
                    if(link){
                        link->url = url;
                        link->text = text;
                        link->next = nil;
                        
                        if(tail)
                            tail->next = link;
                        else
                            head = link;
                        tail = link;
                    }
                }
            }
            
            in_link = 0;
            href_start = href_end = text_start = text_end = nil;
            p += 4;
        }
        else {
            p++;
        }
    }

    return head;
}

/*
 * free_links frees a linked list of Link structures
 */
void
free_links(Link *links)
{
    Link *next;
    
    while(links){
        next = links->next;
        free(links->url);
        free(links->text);
        free(links);
        links = next;
    }
}
