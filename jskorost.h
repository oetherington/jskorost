/*
 * JSkorost - ДжайСкорость
 * Copyright (C) Ollie Etherington 2021
 * <https://github.com/oetherington/jskorost>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * COMPILE-TIME CONFIGURATION:
 *  - JSKOROST_IMPLEMENTATION
 *  - JSK_HASH
 *  - JSK_LOAD_FACTOR
 *  - JSK_DEFAULT_ARRAY_SIZE
 *  - JSK_DEFAULT_OBJECT_SIZE
 *  - JSK_HEAP_CHUNK_SIZE
 *  - JSK_HEAP_MIN_OVERSIZED
 *  - JSK_MALLOC
 *  - JSK_FREE
 *  - JSK_EXPORT
 *  - JSK_NO_STDLIB
 *  - JSK_DEBUG
 *  - JSK_DEBUG_VERBOSE
 *  - JSK_DEBUG_ALLOC
*/

#ifndef JSKOROST_H
#define JSKOROST_H

#define JSK_VERSION_MAJOR 0
#define JSK_VERSION_MINOR 0
#define JSK_VERSION_MICRO 1
#define JSK_VERSION_STRING "0.0.1"

#ifndef JSK_HEAP_CHUNK_SIZE
#define JSK_HEAP_CHUNK_SIZE 4096
#endif

#ifndef JSK_HEAP_MIN_OVERSIZED
#define JSK_HEAP_MIN_OVERSIZED 2046
#endif

#ifndef JSK_MALLOC
#define JSK_MALLOC(ctx, bytes) malloc(bytes)
#endif

#ifndef JSK_FREE
#define JSK_FREE(ctx, ptr) free(ptr)
#endif

#ifndef JSK_EXPORT
#define JSK_EXPORT
#endif

#ifndef JSK_HASH
#define JSK_HASH(s) jsk_hash(s)
#endif

#ifndef JSK_LOAD_FACTOR
#define JSK_LOAD_FACTOR ((float)(7.f / 8.f))
#endif

#ifndef JSK_DEFAULT_ARRAY_SIZE
#define JSK_DEFAULT_ARRAY_SIZE 8
#endif

#ifndef JSK_DEFAULT_OBJECT_SIZE
#define JSK_DEFAULT_OBJECT_SIZE 16
#endif

#define JSK_VALUE_ALIGN 8

#ifdef JSK_DEBUG_VERBOSE
#define jsk_verbose(...) printf(__VA_ARGS__)
#else
#define jsk_verbose(...)
#endif

#ifdef __cplusplus
#define JSK_RESTRICT __restrict__
#else
#define JSK_RESTRICT restrict
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long long jsk_u64;

typedef struct jsk_oversized {
	struct jsk_oversized *next;
	char data[];
} jsk_oversized;

typedef struct jsk_heap {
	struct jsk_heap *next;
	struct jsk_heap *head;
	struct jsk_heap *tail;
	void *ctx;
	unsigned ptr;
	char *chunk;
	jsk_oversized *oversized;
} jsk_heap;

JSK_EXPORT jsk_heap *jsk_heap_new(void *ctx);
JSK_EXPORT void jsk_heap_free(jsk_heap *h);
JSK_EXPORT void *jsk_heap_alloc(jsk_heap *h, unsigned bytes, unsigned align);

typedef enum jsk_type {
	JSK_OBJECT,
	JSK_ARRAY,
	JSK_STRING,
	JSK_INT,
	JSK_FLOAT,
	JSK_BOOL,
	JSK_NULL,
} jsk_type;

typedef struct jsk_value {
	jsk_type type;
	void *value;
} jsk_value;

typedef struct jsk_object_entry {
	jsk_u64 hash;
	char *key;
	jsk_value value;
} jsk_object_entry;

typedef struct jsk_object {
	jsk_heap *heap;
	unsigned allocated;
	unsigned count;
	jsk_object_entry *entries;
} jsk_object;

typedef struct jsk_object_iter {
	jsk_object *obj;
	unsigned bucket;
} jsk_object_iter;

#define jsk_new_bool(v) ((jsk_value){ JSK_BOOL, (void *)((v) & 1) })
#define jsk_new_null()  ((jsk_value){ JSK_NULL, NULL })
#define jsk_new_array() ((jsk_value){ JSK_ARRAY, NULL })

JSK_EXPORT jsk_value jsk_new_int(long long v);
JSK_EXPORT jsk_value jsk_new_float(double f);

JSK_EXPORT jsk_value jsk_new_string_escaped(jsk_heap *h, const char *const s,
		unsigned len);
JSK_EXPORT jsk_value jsk_new_string_len(jsk_heap *h, const char *const s,
		unsigned len);
JSK_EXPORT jsk_value jsk_new_string(jsk_heap *h, const char *const s);

JSK_EXPORT jsk_value jsk_new_object(jsk_heap *h);
JSK_EXPORT void jsk_object_insert(jsk_value *object,
		const char *const name, jsk_value value);
JSK_EXPORT jsk_value *jsk_object_get(jsk_value object, const char *const name);
JSK_EXPORT jsk_object_iter jsk_object_iterate(jsk_value object);
JSK_EXPORT jsk_object_entry *jsk_object_next(jsk_object_iter *i);

#define jsk_get_bool(v) ((int)(long long)(v).value)
#define jsk_get_int(v) (*(long long *)&(v).value)
#define jsk_get_float(v) (*(double *)&(v).value)
#define jsk_get_string(v) ((char *)(v).value)
#define jsk_get_object(v) ((jsk_object *)(v).value)

#define jsk_get_bool_p(v) ((int)(long long)(v)->value)
#define jsk_get_int_p(v) (*(long long *)&(v)->value)
#define jsk_get_float_p(v) (*(double *)&(v)->value)
#define jsk_get_string_p(v) ((char *)(v)->value)
#define jsk_get_object_p(v) ((jsk_object *)(v)->value)

#define jsk_object_count(v) jsk_get_object(v)->count
#define jsk_object_count_p(v) jsk_get_object_p(v)->count

JSK_EXPORT unsigned jsk_array_length(jsk_value array);
JSK_EXPORT void jsk_array_push(jsk_heap *h, jsk_value *array, jsk_value value);
#define jsk_array_at(a, i) (((jsk_value *)((a).value))[i])

typedef enum jsk_status {
	JSK_OK,
	JSK_ERROR,
} jsk_status;

typedef struct jsk_result {
	jsk_status status;

	union {
		jsk_value value;
		char *error;
	} data;
} jsk_result;

JSK_EXPORT jsk_result jsk_parse(jsk_heap *heap,
		const char *const json, unsigned len);
JSK_EXPORT char *jsk_to_string(jsk_heap *heap, jsk_value v);

#ifdef JSKOROST_IMPLEMENTATION

#ifndef JSK_NO_STDLIB
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#endif

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#pragma GCC diagnostic ignored "-Wformat-zero-length"
#endif

#define JSK_LIKELY(x)    __builtin_expect((x),1)
#define JSK_UNLIKELY(x)  __builtin_expect((x),0)

JSK_EXPORT jsk_heap *jsk_heap_new(void *ctx)
{
	jsk_heap *h = (jsk_heap *)JSK_MALLOC(ctx, sizeof(jsk_heap));
	if (JSK_UNLIKELY(!h))
		return NULL;

	h->chunk = (char *)JSK_MALLOC(ctx, JSK_HEAP_CHUNK_SIZE);
	if (JSK_UNLIKELY(!h->chunk)) {
		JSK_FREE(ctx, h);
		return NULL;
	}

#ifdef JSK_DEBUG
	memset(h->chunk, 255, JSK_HEAP_CHUNK_SIZE);
#endif

	h->next = NULL;
	h->head = h;
	h->tail = h;
	h->ctx = ctx;
	h->ptr = 0;
	h->oversized = NULL;

	return h;
}

JSK_EXPORT void jsk_heap_free(jsk_heap *h)
{
	while (h->oversized) {
		jsk_oversized *o = h->oversized;
		h->oversized = o->next;
		JSK_FREE(h->ctx, o);
	}

	JSK_FREE(h->ctx, h->chunk);

	if (h->next)
		jsk_heap_free(h->next);

	JSK_FREE(h->ctx, h);
}

JSK_EXPORT void *jsk_heap_alloc(jsk_heap *h, unsigned bytes, unsigned align)
{
#ifdef JSK_DEBUG_ALLOC
	return (void *)JSK_MALLOC(h->ctx, bytes);
#endif

	jsk_heap *head = h;
	h = h->tail;

	if (bytes >= JSK_HEAP_MIN_OVERSIZED) {
		const unsigned n = bytes + sizeof(jsk_oversized *);
		jsk_oversized *o = (jsk_oversized *)JSK_MALLOC(h->ctx, n);
		if (JSK_UNLIKELY(!o))
			return NULL;
		o->next = h->oversized;
		h->oversized = o;
		return o->data;
	}

	if (align > 1 && h->ptr & (align - 1)) {
		h->ptr &= ~(align - 1);
		h->ptr += align;
	}

	if (h->ptr + bytes > JSK_HEAP_CHUNK_SIZE) {
		jsk_heap *h0 = jsk_heap_new(h->ctx);
		if (JSK_UNLIKELY(!h0))
			return NULL;

		h0->head = head;
		head->tail = h0;
		h->next = h0;
		h = h0;
	}

	void *alloc = &h->chunk[h->ptr];
	h->ptr += bytes;
	return alloc;
}

static void *jsk_heap_unify(jsk_heap *heap, int null_terminate)
{
	unsigned bytes = !!null_terminate;

	jsk_heap *h = heap;
	while (h) {
		bytes += h->ptr;
		h = h->next;
	}

	char *mem = (char *)JSK_MALLOC(heap->ctx, bytes);
	if (JSK_UNLIKELY(!mem))
		return NULL;

	bytes = 0;
	h = heap;
	while (h) {
		memcpy(&mem[bytes], h->chunk, h->ptr);
		bytes += h->ptr;
		h = h->next;
	}

	if (null_terminate)
		mem[bytes] = 0;

	return mem;
}

static char *jsk_vprintf(jsk_heap *h, int null_terminate,
		const char *const fmt, va_list args)
{
	jsk_heap *tail = h->tail;
	const unsigned len = JSK_HEAP_CHUNK_SIZE - tail->ptr;

#ifdef JSK_DEBUG_ALLOC
	char *s = (char *)JSK_MALLOC(h->ctx, len);
#else
	char *s = &tail->chunk[tail->ptr];
#endif

	const unsigned needed = vsnprintf(s, len, fmt, args) + 1;

	if (needed > len) {
		s = (char *)jsk_heap_alloc(h, needed, 1);
		vsnprintf(s, needed, fmt, args);
		return s;
	}

	tail->ptr += needed - (null_terminate ? 0 : 1);
	return s;
}

__attribute__((__format__ (__printf__, 3, 4)))
static char *jsk_printf(jsk_heap *h, int null_terminate,
		const char *const fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	char *s = jsk_vprintf(h, null_terminate, fmt, args);
	va_end(args);
	return s;
}

static jsk_u64 jsk_hash(const char *const str) /* Simple FNV-1a */
{
	unsigned char *s = (unsigned char *)str;

	jsk_u64 val = 0xcbf29ce484222325ULL;

	while (*s) {
		val ^= (jsk_u64)*s++;
		val *= 0x100000001b3ULL;
	}

	return val;
}

#define JSK_TOKENS                                 \
	JSK_X(JSKT_EOF,     0,   "end of file")    \
	JSK_X(JSKT_INVALID, 1,   "invalid token")  \
	JSK_X(JSKT_INT,     2,   "int")            \
	JSK_X(JSKT_FLOAT,   3,   "float")          \
	JSK_X(JSKT_STRING,  4,   "string")         \
	JSK_X(JSKT_TRUE,    5,   "true")           \
	JSK_X(JSKT_FALSE,   6,   "false")          \
	JSK_X(JSKT_NULL,    7,   "null")           \
	JSK_X(JSKT_LBRACK,  '[', "left bracket")   \
	JSK_X(JSKT_RBRACK,  ']', "right bracket")  \
	JSK_X(JSKT_LBRACE,  '{', "left brace")     \
	JSK_X(JSKT_RBRACE,  '}', "right brace")    \
	JSK_X(JSKT_COLON,   ':', "colon")          \
	JSK_X(JSKT_COMMA,   ',', "comma")

typedef enum jsk_token_type {
#define JSK_X(t, n, s) t = n,
	JSK_TOKENS
#undef JSK_X
} jsk_token_type;

static const char *jsk_token_names[] = {
#define JSK_X(t, n, s) s,
	JSK_TOKENS
#undef JSK_X
};

typedef struct jks_token {
	jsk_token_type type;
	const char *data;
	int len;
} jsk_token;

typedef struct jsk_context {
	jsk_heap *heap;
	const char *const json;
	unsigned long long len;
	unsigned long long ptr;
	jsk_token tkn;
} jsk_context;

static double jsk_ten_pow(int exponent)
{
	int recip;
	if (exponent < 0) {
		exponent = -exponent;
		recip = 1;
	} else {
		recip = 0;
	}

	if (JSK_UNLIKELY(exponent > 325))
		exponent = 325;

	double result = 1;

	while (exponent > 100) {
		result *= 10e100;
		exponent -= 100;
	}

	while (exponent > 33) {
		result *= 10e33;
		exponent -= 33;
	}

	while (exponent > 10) {
		result *= 10e10;
		exponent -= 10;
	}

	static const double m[] = {
		1, 10, 100, 1000, 10e3, 10e4, 10e5, 10e6, 10e7, 10e8, 10e9,
	};

	result *= m[exponent];
	return recip ? 1.f / result : result;
}

static void jsk_lex(jsk_context *ctx)
{
	enum {
		L_INV, /* invalid */
		L_SKP, /* skip */
		L_CHR, /* immediate character */
		L_STR, /* string */
		L_NUM, /* number */
		L_FLS, /* false */
		L_TRU, /* true */
		L_NUL, /* null */
	};

	static const char dispatch[] = {
		L_CHR, L_SKP, L_SKP, L_SKP, L_SKP, L_SKP, L_SKP, L_SKP,
		L_SKP, L_SKP, L_SKP, L_SKP, L_SKP, L_SKP, L_SKP, L_SKP,
		L_SKP, L_SKP, L_SKP, L_SKP, L_SKP, L_SKP, L_SKP, L_SKP,
		L_SKP, L_SKP, L_SKP, L_SKP, L_SKP, L_SKP, L_SKP, L_SKP,
		L_SKP, L_INV, L_STR, L_INV, L_INV, L_INV, L_INV, L_INV,
		L_CHR, L_CHR, L_INV, L_INV, L_CHR, L_NUM, L_INV, L_INV,
		L_NUM, L_NUM, L_NUM, L_NUM, L_NUM, L_NUM, L_NUM, L_NUM,
		L_NUM, L_NUM, L_CHR, L_INV, L_INV, L_INV, L_INV, L_INV,
		L_INV, L_INV, L_INV, L_INV, L_INV, L_INV, L_INV, L_INV,
		L_INV, L_INV, L_INV, L_INV, L_INV, L_INV, L_INV, L_INV,
		L_INV, L_INV, L_INV, L_INV, L_INV, L_INV, L_INV, L_INV,
		L_INV, L_INV, L_INV, L_CHR, L_INV, L_CHR, L_INV, L_INV,
		L_INV, L_INV, L_INV, L_INV, L_INV, L_INV, L_FLS, L_INV,
		L_INV, L_INV, L_INV, L_INV, L_INV, L_INV, L_NUL, L_INV,
		L_INV, L_INV, L_INV, L_INV, L_TRU, L_INV, L_INV, L_INV,
		L_INV, L_INV, L_INV, L_CHR, L_INV, L_CHR, L_INV, L_SKP,
	};

lex_next:
	if (JSK_UNLIKELY(ctx->ptr == ctx->len)) {
		ctx->tkn.type = JSKT_EOF;
		return;
	}

	const char c = ctx->json[ctx->ptr];

	if (JSK_UNLIKELY(c < 0)) {
		ctx->tkn.type = JSKT_INVALID;
		return;
	}

	switch (dispatch[(int)c]) {
	case L_INV:
		ctx->tkn.type = JSKT_INVALID;
		return;

	case L_SKP:
		ctx->ptr++;
		goto lex_next;

	case L_CHR:
		ctx->tkn.type = (jsk_token_type)c;
		ctx->ptr++;
		return;

	case L_STR:
		ctx->ptr++;
		ctx->tkn.data = &ctx->json[ctx->ptr];

		for (unsigned escapes = 0; 1; ctx->ptr++) {
			if (JSK_UNLIKELY(ctx->ptr == ctx->len)) {
				ctx->tkn.type = JSKT_INVALID;
				return;
			}

			const char c0 = ctx->json[ctx->ptr];

			if (c0 == c && ~escapes & 1) {
				const char *end = &ctx->json[ctx->ptr];
				ctx->tkn.len = end - ctx->tkn.data;
				ctx->tkn.type = JSKT_STRING;
				ctx->ptr++;
				return;
			} else if (c0 == '\\') {
				escapes++;
			} else {
				escapes = 0;
			}
		}

	case L_NUM: {
		long long multiplier;
		if (ctx->json[ctx->ptr] == '-') {
			multiplier = -1;
			ctx->ptr++;
		} else {
			multiplier = 1;
		}

		long long n = 0;
		ctx->tkn.type = JSKT_INT;

		char digit;
		while ((digit = ctx->json[ctx->ptr]) >= '0' && digit <= '9') {
			n = n * 10 + digit - '0';
			ctx->ptr++;
		}

		n *= multiplier;

		double f;
		if (ctx->json[ctx->ptr] == '.') {
			ctx->tkn.type = JSKT_FLOAT;
			f = n;
			ctx->ptr++;

			double magnitude = 0.1;
			while ((digit = ctx->json[ctx->ptr]) >= '0' &&
					digit <= '9') {
				f += (digit - '0') * magnitude;
				magnitude /= 10;
				ctx->ptr++;
			}
		}

		if (ctx->json[ctx->ptr] == 'e' || ctx->json[ctx->ptr] == 'E') {
			if (ctx->tkn.type != JSKT_FLOAT) {
				ctx->tkn.type = JSKT_FLOAT;
				f = n;
			}

			ctx->ptr++;

			if (ctx->json[ctx->ptr] == '-') {
				multiplier = -1;
				ctx->ptr++;
			} else if (ctx->json[ctx->ptr] == '+') {
				ctx->ptr++;
				multiplier = 1;
			} else {
				multiplier = 1;
			}

			int exponent = 0;
			while ((digit = ctx->json[ctx->ptr]) >= '0' &&
					digit <= '9') {
				exponent = exponent * 10 + digit - '0';
				ctx->ptr++;
			}

			f *= jsk_ten_pow(exponent * multiplier);
		}

		if (ctx->tkn.type == JSKT_FLOAT)
			ctx->tkn.data = (char *)*(void**)&f;
		else
			ctx->tkn.data = (char *)*(void**)&n;

		ctx->tkn.len = 0;

		return;
	}

	case L_FLS:
		if (JSK_LIKELY(ctx->ptr + 4 < ctx->len &&
					ctx->json[ctx->ptr + 1] == 'a' &&
					ctx->json[ctx->ptr + 2] == 'l' &&
					ctx->json[ctx->ptr + 3] == 's' &&
					ctx->json[ctx->ptr + 4] == 'e')) {
			ctx->tkn.type = JSKT_FALSE;
			ctx->ptr += 5;
		} else {
			ctx->tkn.type = JSKT_INVALID;
		}
		return;

	case L_TRU:
		if (JSK_LIKELY(ctx->ptr + 3 < ctx->len &&
					ctx->json[ctx->ptr + 1] == 'r' &&
					ctx->json[ctx->ptr + 2] == 'u' &&
					ctx->json[ctx->ptr + 3] == 'e')) {
			ctx->tkn.type = JSKT_TRUE;
			ctx->ptr += 4;
		} else {
			ctx->tkn.type = JSKT_INVALID;
		}
		return;

	case L_NUL:
		if (JSK_LIKELY(ctx->ptr + 3 < ctx->len &&
					ctx->json[ctx->ptr + 1] == 'u' &&
					ctx->json[ctx->ptr + 2] == 'l' &&
					ctx->json[ctx->ptr + 3] == 'l')) {
			ctx->tkn.type = JSKT_NULL;
			ctx->ptr += 4;
		} else {
			ctx->tkn.type = JSKT_INVALID;
		}
		return;
	}
}

#define jsk_success(v) ((jsk_result){ JSK_OK, { .value = v } })

__attribute__((__format__ (__printf__, 2, 3)))
static jsk_result jsk_error(jsk_context *ctx, const char *const fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	char *s = jsk_vprintf(ctx->heap, 1, fmt, args);
	va_end(args);
	return (jsk_result){ JSK_ERROR, { .error = s } };
}

JSK_EXPORT jsk_value jsk_new_int(long long v)
{
	return (jsk_value){ JSK_INT, *(void **)(&(v)) };
}

JSK_EXPORT jsk_value jsk_new_float(double f)
{
	return (jsk_value){ JSK_FLOAT, *(void **)(&(f)) };
}

static unsigned jsk_unescape(char *JSK_RESTRICT dest,
		const char *JSK_RESTRICT src)
{
	switch (*src) {
	case '"':	*dest = '"';	return 1;
	case '\\':	*dest = '\\';	return 1;
	case '/':	*dest = '/';	return 1;
	case 'b':	*dest = '\b';	return 1;
	case 'f':	*dest = '\f';	return 1;
	case 'n':	*dest = '\n';	return 1;
	case 'r':	*dest = '\r';	return 1;
	case 't':	*dest = '\t';	return 1;
	case 'u':	// TODO: Unicode escape sequences
	default:
		break;
	}

	return 0;
}

JSK_EXPORT jsk_value jsk_new_string_escaped(jsk_heap *h, const char *const s,
		unsigned len)
{
	char *mem = (char *)jsk_heap_alloc(h, len + 1, 1);

	unsigned dest = 0, src = 0;

	while (src < len) {
		if (s[src] == '\\') {
			src++;
			if (JSK_UNLIKELY(src >= len))
				break;
			const unsigned b = jsk_unescape(&mem[dest], &s[src]);
			if (JSK_UNLIKELY(b == 0))
				break;
			src++;
			dest += b;
		} else {
			mem[dest++] = s[src++];
		}
	}

	mem[dest] = 0;

	return (jsk_value){ JSK_STRING, mem };
}

JSK_EXPORT jsk_value jsk_new_string_len(jsk_heap *h, const char *const s,
		unsigned len)
{
	char *mem = (char *)jsk_heap_alloc(h, len + 1, 1);
	memcpy(mem, s, len);
	mem[len] = 0;
	return (jsk_value){ JSK_STRING, mem };
}

JSK_EXPORT jsk_value jsk_new_string(jsk_heap *h, const char *const s)
{
	return jsk_new_string_len(h, s, strlen(s));
}

JSK_EXPORT jsk_value jsk_new_object(jsk_heap *h)
{
	jsk_object *obj = (jsk_object *)jsk_heap_alloc(h, sizeof(jsk_object),
			JSK_VALUE_ALIGN);

	if (JSK_UNLIKELY(!obj))
		return jsk_new_null();

	obj->heap = h;
	obj->allocated = JSK_DEFAULT_OBJECT_SIZE;
	obj->count = 0;

	const unsigned bytes = obj->allocated * sizeof(jsk_object_entry);
	obj->entries = (jsk_object_entry *)jsk_heap_alloc(h, bytes,
			JSK_VALUE_ALIGN);

	if (JSK_UNLIKELY(!obj->entries))
		return jsk_new_null();

	memset(obj->entries, 0, bytes);

	return (jsk_value){ JSK_OBJECT, obj };
}

static void jsk_object_insert_unsafe(jsk_object *obj,
		char *name, jsk_value value)
{
	const jsk_u64 hash = JSK_HASH(name);

	jsk_u64 bucket = hash % obj->allocated;

	while (obj->entries[bucket].hash != 0) {
		bucket++;
		if (bucket == obj->allocated)
			bucket = 0;
	}

	obj->entries[bucket] = (jsk_object_entry){
		hash,
		name,
		value,
	};
}

static void jsk_object_grow_and_rehash(jsk_object *obj)
{
	jsk_object_entry *old = obj->entries;
	const unsigned old_allocated = obj->allocated;

	obj->allocated *= 2;

	const unsigned bytes = obj->allocated * sizeof(jsk_object_entry);
	obj->entries = (jsk_object_entry *)jsk_heap_alloc(obj->heap,
			bytes, JSK_VALUE_ALIGN);
	memset(obj->entries, 0, bytes);

	for (unsigned i = 0; i < old_allocated; i++) {
		if (old[i].hash == 0)
			continue;
		jsk_object_insert_unsafe(obj, old[i].key, old[i].value);
	}
}

JSK_EXPORT void jsk_object_insert(jsk_value *object,
		const char *const name, jsk_value value)
{
	jsk_object *obj = (jsk_object *)object->value;

	obj->count++;

	const float load = (float)obj->count / (float)obj->allocated;
	if (JSK_UNLIKELY(load >= JSK_LOAD_FACTOR))
		jsk_object_grow_and_rehash(obj);

	jsk_object_insert_unsafe(obj, (char *)name, value);
}

JSK_EXPORT jsk_value *jsk_object_get(jsk_value object, const char *const name)
{
	jsk_object *obj = (jsk_object *)object.value;

	const jsk_u64 hash = JSK_HASH(name);

	jsk_u64 bucket = hash % obj->allocated;

	while (1) {
		jsk_object_entry *e = &obj->entries[bucket];

		if (e->hash == hash && !strcmp(e->key, name))
			return &e->value;
		else if (e->hash == 0)
			return NULL;

		bucket++;
		if (JSK_UNLIKELY(bucket == obj->allocated))
			bucket = 0;
	}
}

JSK_EXPORT jsk_object_iter jsk_object_iterate(jsk_value object)
{
	return (jsk_object_iter){ (jsk_object *)object.value, 0, };
}

JSK_EXPORT jsk_object_entry *jsk_object_next(jsk_object_iter *i)
{
	while (i->bucket < i->obj->allocated) {
		if (i->obj->entries[i->bucket].hash) {
			jsk_object_entry *e = &i->obj->entries[i->bucket];
			i->bucket++;
			return e;
		}

		i->bucket++;
	}

	return NULL;
}

JSK_EXPORT unsigned jsk_array_length(jsk_value array)
{
	return array.value ? ((unsigned *)array.value)[-1] : 0;
}

JSK_EXPORT void jsk_array_push(jsk_heap *h, jsk_value *array, jsk_value value)
{
	if (array->value) {
		jsk_value *vs = (jsk_value *)array->value;
		const unsigned allocated = ((unsigned *)vs)[-2];
		const unsigned len = ((unsigned *)vs)[-1];

		if (JSK_LIKELY(len < allocated)) {
			vs[len] = value;
			((unsigned *)vs)[-1]++;
			return;
		}

		const unsigned n = allocated * 2;
		const unsigned b = 2 * sizeof(unsigned) + n * sizeof(jsk_value);
		unsigned *mem = (unsigned *)jsk_heap_alloc(h, b,
				JSK_VALUE_ALIGN);
		mem[0] = n;
		mem[1] = len + 1;
		jsk_value *new_vs = (jsk_value *)&mem[2];
		memcpy(new_vs, vs, allocated * sizeof(jsk_value));
		new_vs[allocated] = value;
		array->value = new_vs;
	} else {
		const unsigned n = JSK_DEFAULT_ARRAY_SIZE;
		const unsigned b = 2 * sizeof(unsigned) + n * sizeof(jsk_value);
		unsigned *mem = (unsigned *)jsk_heap_alloc(h, b,
				JSK_VALUE_ALIGN);
		mem[0] = n;
		mem[1] = 1;
		jsk_value *vs = (jsk_value *)&mem[2];
		vs[0] = value;
		array->value = vs;
	}
}

static jsk_result jsk_expected(jsk_context *ctx, const char *const what)
{
	return jsk_error(ctx, "Expected %s at index %llu but found %s",
			what, ctx->ptr - 1, jsk_token_names[ctx->tkn.type]);
}

static jsk_result jsk_parse_value(jsk_context *ctx)
{
	switch (ctx->tkn.type) {
	case JSKT_INT: {
		const jsk_value v = (jsk_value){
			JSK_INT,
			(void*)ctx->tkn.data,
		};
		jsk_verbose("D INT %lld @ %llu\n", jsk_get_int(v), ctx->ptr);
		jsk_lex(ctx);
		return jsk_success(v);
	}

	case JSKT_FLOAT: {
		const jsk_value v = (jsk_value){
			JSK_FLOAT,
			(void*)ctx->tkn.data,
		};
		jsk_verbose("D FLT %f @ %llu\n", jsk_get_float(v), ctx->ptr);
		jsk_lex(ctx);
		return jsk_success(v);
	}

	case JSKT_STRING: {
		const jsk_value v = jsk_new_string_escaped(ctx->heap,
				ctx->tkn.data, ctx->tkn.len);
		jsk_verbose("D STR %s @ %llu\n", jsk_get_string(v), ctx->ptr);
		jsk_lex(ctx);
		return jsk_success(v);
	}

	case JSKT_TRUE:
		jsk_verbose("D TRUE @ %llu\n", ctx->ptr);
		jsk_lex(ctx);
		return jsk_success(jsk_new_bool(1));

	case JSKT_FALSE:
		jsk_verbose("D FALSE @ %llu\n", ctx->ptr);
		jsk_lex(ctx);
		return jsk_success(jsk_new_bool(0));

	case JSKT_NULL:
		jsk_verbose("D NULL @ %llu\n", ctx->ptr);
		jsk_lex(ctx);
		return jsk_success(jsk_new_null());

	case JSKT_LBRACK: {
		jsk_verbose("D ARRAY @ %llu\n", ctx->ptr);

		jsk_value a = jsk_new_array();

		do {
			jsk_lex(ctx);

			if (JSK_UNLIKELY(ctx->tkn.type == JSKT_RBRACK))
				break;

			jsk_result res = jsk_parse_value(ctx);
			if (res.status != JSK_OK)
				return res;

			jsk_array_push(ctx->heap, &a, res.data.value);
		} while (ctx->tkn.type == JSKT_COMMA);

		if (ctx->tkn.type != JSKT_RBRACK)
			return jsk_expected(ctx, "']' after array");

		jsk_lex(ctx);

		return jsk_success(a);
	}

	case JSKT_LBRACE: {
		jsk_verbose("D OBJECT @ %llu\n", ctx->ptr);

		jsk_value o = jsk_new_object(ctx->heap);

		do {
			jsk_lex(ctx);

			if (JSK_UNLIKELY(ctx->tkn.type == JSKT_RBRACE))
				break;

			if (ctx->tkn.type != JSKT_STRING)
				return jsk_expected(ctx, "object key");

			char *name = (char *)jsk_heap_alloc(ctx->heap,
					ctx->tkn.len + 1, 1);
			memcpy(name, ctx->tkn.data, ctx->tkn.len);
			name[ctx->tkn.len] = 0;

			jsk_verbose("D OBJECT KEY %s @ %llu\n", name, ctx->ptr);

			jsk_lex(ctx);

			if (ctx->tkn.type != JSKT_COLON)
				return jsk_expected(ctx, "':'");

			jsk_lex(ctx);

			jsk_result res = jsk_parse_value(ctx);
			if (res.status != JSK_OK)
				return res;

			jsk_object_insert(&o, name, res.data.value);
		} while (ctx->tkn.type == JSKT_COMMA);

		if (ctx->tkn.type != JSKT_RBRACE)
			return jsk_expected(ctx, "'}' after object");

		jsk_lex(ctx);

		return jsk_success(o);
	}

	default:
		return jsk_error(ctx, "Unexpected %s at index %llu",
			jsk_token_names[ctx->tkn.type], ctx->ptr - 1);
	}
}

JSK_EXPORT jsk_result jsk_parse(jsk_heap *heap,
		const char *const json, unsigned len)
{
	jsk_context ctx = (jsk_context){
		heap,
		json,
		len,
		0,
		(jsk_token){ JSKT_INVALID, 0, 0, },
	};

	jsk_lex(&ctx);
	return jsk_parse_value(&ctx);
}

static void jsk_print_unescaped_string(jsk_heap *h, int null_terminate, char *s)
{
	jsk_printf(h, 0, "\"");

	char *start = s;

#define JSK_STR_CASE(fmt)					\
	jsk_printf(h, 0, fmt, (int)(s - start),  start);	\
	start = s + 1

	while (1) {
		switch (*s) {
		case 0:
			jsk_printf(h, null_terminate, "%s\"", start);
			return;

		case '"':	JSK_STR_CASE("%.*s\\\"");	break;
		case '\\':	JSK_STR_CASE("%.*s\\\\");	break;
		case '/':	JSK_STR_CASE("%.*s\\/");	break;
		case '\b':	JSK_STR_CASE("%.*s\\b");	break;
		case '\f':	JSK_STR_CASE("%.*s\\f");	break;
		case '\n':	JSK_STR_CASE("%.*s\\n");	break;
		case '\r':	JSK_STR_CASE("%.*s\\r");	break;
		case '\t':	JSK_STR_CASE("%.*s\\t");	break;

		// TODO: Handle unicode escape sequences

		default:
			break;
		}

		s++;
	}

#undef JSK_STR_CASE
}

static void jsk_to_string_internal(jsk_heap *h, jsk_value v)
{
	switch (v.type) {
	case JSK_OBJECT: {
		jsk_printf(h, 0, "{");

		jsk_object_iter it = jsk_object_iterate(v);
		jsk_object_entry *e;
		const char *comma = "";
		while ((e = jsk_object_next(&it))) {
			jsk_printf(h, 0, "%s\"%s\":", comma, e->key);
			jsk_to_string_internal(h, e->value);
			comma = ",";
		}

		jsk_printf(h, 0, "}");
		return;
	}

	case JSK_ARRAY: {
		jsk_printf(h, 0, "[");

		const unsigned len = jsk_array_length(v);
		if (len) {
			jsk_to_string_internal(h, jsk_array_at(v, 0));
			for (unsigned i = 1; i < len; i++) {
				jsk_printf(h, 0, ",");
				jsk_to_string_internal(h, jsk_array_at(v, i));
			}
		}

		jsk_printf(h, 0, "]");
		return;
	}

	case JSK_STRING:
		jsk_print_unescaped_string(h, 0, (char *)v.value);
		return;

	case JSK_INT:
		jsk_printf(h, 0, "%lld", jsk_get_int(v));
		return;

	case JSK_FLOAT:
		jsk_printf(h, 0, "%lf", jsk_get_float(v));
		return;

	case JSK_BOOL:
		jsk_printf(h, 0, v.value ? "true" : "false");
		return;

	case JSK_NULL:
		jsk_printf(h, 0, "null");
		return;
	}
}

JSK_EXPORT char *jsk_to_string(jsk_heap *heap, jsk_value v)
{
	jsk_heap *string_heap = jsk_heap_new(heap->ctx);
	jsk_to_string_internal(string_heap, v);
	jsk_printf(heap, 1, "");
	char *s = (char *)jsk_heap_unify(string_heap, 1);
	jsk_heap_free(string_heap);
	return s;
}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif /* JSKOROST_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif

#endif /* JSKOROST_H */
