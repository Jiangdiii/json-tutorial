#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <errno.h>   /* errno, ERANGE */
#include <math.h>    /* HUGE_VAL */
#include <stdlib.h>  /* NULL, strtod() */

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISINT(ch)   (((ch) >= '0') && ((ch) <= '9'))
#define ISINT1TO9(ch) (((ch) >= '1') && ((ch) <= '9'))

typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}
#if 0
static int lept_parse_true(lept_context* c, lept_value* v) {
    EXPECT(c, 't');
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_TRUE;
    return LEPT_PARSE_OK;
}

static int lept_parse_false(lept_context* c, lept_value* v) {
    EXPECT(c, 'f');
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 4;
    v->type = LEPT_FALSE;
    return LEPT_PARSE_OK;
}

static int lept_parse_null(lept_context* c, lept_value* v) {
    EXPECT(c, 'n');
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_NULL;
    return LEPT_PARSE_OK;
}
#endif
#if 0
static int lept_parse_number(lept_context* c, lept_value* v) {
    char* end;
    /* \TODO validate number */
    v->n = strtod(c->json, &end);
    if (c->json == end)
        return LEPT_PARSE_INVALID_VALUE;
    c->json = end;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}
#endif
static int lept_parse_number(lept_context * c , lept_value *v){
    const char *temp=c->json;
    if(*temp == '-')temp++;
    if(*temp == '0')temp++;
    else{
        if(!ISINT1TO9(*temp)) return LEPT_PARSE_INVALID_VALUE;
        for(temp++;ISINT(*temp);temp++);
    }
    if(*temp == '.'){
        temp++;
        if (!ISINT(*temp)) return LEPT_PARSE_INVALID_VALUE;
        for (temp++; ISINT(*temp); temp++);
    }
    if(*temp == 'e' || *temp == 'E'){
        temp++;
        if(*temp == '+' || *temp == '-')
            temp++;   
        if (!ISINT(*temp)) return LEPT_PARSE_INVALID_VALUE;
        for(temp++;ISINT(*temp);temp++);    
    }
    errno = 0;
    v->n=strtod(c->json,NULL);
    if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL))
        return LEPT_PARSE_NUMBER_TOO_BIG;
    v->type=LEPT_NUMBER;
    c->json = temp;
    return LEPT_PARSE_OK;
}
static int lept_parse_litral(lept_context *c , lept_value *v,const char *string,lept_type type){
    int i;

    EXPECT(c,string[0]);
    for(i=0; string[i+1];i++)
        if(c->json[i] != string[i+1])
            return LEPT_PARSE_INVALID_VALUE;

    c->json+=i;
    v->type=type;
    return LEPT_PARSE_OK;
}
static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't': return lept_parse_litral(c,v,"true",LEPT_TRUE);
        case 'f': return lept_parse_litral(c,v,"false",LEPT_FALSE);
        case 'n': return lept_parse_litral(c,v,"null",LEPT_NULL);
#if 0
        case 't':  return lept_parse_true(c, v);
        case 'f':  return lept_parse_false(c, v);
        case 'n':  return lept_parse_null(c, v);
#endif   
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}