#define JSKOROST_IMPLEMENTATION
#define JSK_DEBUG
#include "jskorost.h"
#include <stdlib.h>
#include <setjmp.h>
#include <cmocka.h>

static void test_heap(void **state)
{
	(void)state;

	jsk_heap *h = jsk_heap_new(NULL);
	assert_non_null(h);
	assert_ptr_equal(h->head, h->tail);

	const size_t size = 128;

	char *a = jsk_heap_alloc(h, size, 8);
	assert_non_null(a);
	assert_ptr_equal(a, h->chunk);

	char *b = jsk_heap_alloc(h, size, 8);
	assert_non_null(b);
	assert_ptr_equal(b, a + size);

	for (size_t allocated = 2 * size;
			allocated + size < JSK_HEAP_CHUNK_SIZE;
			allocated += size)
		b = jsk_heap_alloc(h, size, 8);

	b = jsk_heap_alloc(h, 2 * size, 8);
	assert_non_null(b);
	assert_ptr_not_equal(h->head, h->tail);
	assert_ptr_equal(b, h->tail->chunk);

	b = jsk_heap_alloc(h, 1, 8);
	b = jsk_heap_alloc(h, 8, 8);
	assert_int_equal((long long)b % 8, 0);

	jsk_heap_free(h);
}

static void test_heap_oversized(void **state)
{
	(void)state;

	jsk_heap *h = jsk_heap_new(NULL);

	void *a = jsk_heap_alloc(h, JSK_HEAP_MIN_OVERSIZED, 8);
	assert_ptr_equal(a, h->oversized->data);

	jsk_heap_free(h);
}

static void test_simple_values(void **state)
{
	(void)state;

	const jsk_value t = jsk_new_bool(1);
	assert_int_equal(t.type, JSK_BOOL);
	assert_true(jsk_get_bool(t));

	const jsk_value f = jsk_new_bool(0);
	assert_int_equal(f.type, JSK_BOOL);
	assert_false(jsk_get_bool(f));

	const jsk_value n = jsk_new_null();
	assert_int_equal(n.type, JSK_NULL);
	assert_null(n.value);

	const jsk_value i = jsk_new_int(123456);
	assert_int_equal(i.type, JSK_INT);
	assert_int_equal(jsk_get_int(i), 123456);

	const jsk_value d = jsk_new_float(123.456);
	assert_int_equal(d.type, JSK_FLOAT);
	assert_int_equal(jsk_get_float(d), 123.456);
}

static void test_strings(void **state)
{
	(void)state;

	jsk_heap *h = jsk_heap_new(NULL);

	const jsk_value a = jsk_new_string(h, "Hello World");
	assert_int_equal(a.type, JSK_STRING);
	assert_string_equal(a.value, "Hello World");

	const jsk_value b = jsk_new_string_len(h, "Hello World", 5);
	assert_int_equal(b.type, JSK_STRING);
	assert_string_equal(b.value, "Hello");

	const char *escaped = "Hello\\nWorld";
	const jsk_value c = jsk_new_string_escaped(h, escaped, strlen(escaped));
	assert_int_equal(c.type, JSK_STRING);
	assert_string_equal(c.value, "Hello\nWorld");

	assert_string_equal(a.value, "Hello World");
	assert_string_equal(b.value, "Hello");
	assert_string_equal(c.value, "Hello\nWorld");

	jsk_heap_free(h);
}

static void test_arrays(void **state)
{
	(void)state;

	jsk_value a = jsk_new_array();
	assert_int_equal(a.type, JSK_ARRAY);
	assert_int_equal(jsk_array_length(a), 0);

	jsk_heap *h = jsk_heap_new(NULL);

	jsk_array_push(h, &a, jsk_new_null());
	assert_int_equal(jsk_array_length(a), 1);

	jsk_value b = jsk_array_at(a, 0);
	assert_int_equal(b.type, JSK_NULL);

	jsk_array_push(h, &a, jsk_new_bool(1));
	assert_int_equal(jsk_array_length(a), 2);

	b = jsk_array_at(a, 1);
	assert_int_equal(b.type, JSK_BOOL);
	assert_true(jsk_get_bool(b));

	jsk_heap_free(h);
}

static void test_objects(void **state)
{
	(void)state;

	jsk_heap *h = jsk_heap_new(NULL);

	jsk_value a = jsk_new_object(h);
	jsk_object *o = jsk_get_object(a);
	assert_non_null(a.value);
	assert_non_null(o);
	assert_int_equal(o->count, 0);

	for (size_t i = 0; i < o->allocated; i++)
		assert_int_equal(o->entries[i].hash, 0);

	jsk_object_insert(&a, "test", jsk_new_bool(1));
	assert_int_equal(o->count, 1);

	int num_non_zero = 0;
	for (size_t i = 0; i < o->allocated; i++)
		if (o->entries[i].hash)
			num_non_zero++;
	assert_int_equal(num_non_zero, 1);

	jsk_value *v = jsk_object_get(a, "test");
	assert_non_null(v);
	assert_int_equal(v->type, JSK_BOOL);
	assert_true(jsk_get_bool_p(v));

	jsk_heap_free(h);
}

static void test_parse_simple_values(void **state)
{
	(void)state;

	char *json;
	jsk_result res;
	jsk_heap *h = jsk_heap_new(NULL);

	json = "null";
	res = jsk_parse(h, json, strlen(json));
	assert_int_equal(res.status, JSK_OK);
	assert_int_equal(res.data.value.type, JSK_NULL);
	assert_null(res.data.value.value);

	json = "true";
	res = jsk_parse(h, json, strlen(json));
	assert_int_equal(res.status, JSK_OK);
	assert_int_equal(res.data.value.type, JSK_BOOL);
	assert_true(jsk_get_bool(res.data.value));

	json = "false";
	res = jsk_parse(h, json, strlen(json));
	assert_int_equal(res.status, JSK_OK);
	assert_int_equal(res.data.value.type, JSK_BOOL);
	assert_false(jsk_get_bool(res.data.value));

	json = "123456";
	res = jsk_parse(h, json, strlen(json));
	assert_int_equal(res.status, JSK_OK);
	assert_int_equal(res.data.value.type, JSK_INT);
	assert_int_equal(jsk_get_int(res.data.value), 123456);

	json = "123.456";
	res = jsk_parse(h, json, strlen(json));
	assert_int_equal(res.status, JSK_OK);
	assert_int_equal(res.data.value.type, JSK_FLOAT);
	assert_int_equal(jsk_get_float(res.data.value), 123.456);

	json = "123e5";
	res = jsk_parse(h, json, strlen(json));
	assert_int_equal(res.status, JSK_OK);
	assert_int_equal(res.data.value.type, JSK_FLOAT);
	assert_int_equal(jsk_get_float(res.data.value), 12300000.f);

	json = "123e+5";
	res = jsk_parse(h, json, strlen(json));
	assert_int_equal(res.status, JSK_OK);
	assert_int_equal(res.data.value.type, JSK_FLOAT);
	assert_int_equal(jsk_get_float(res.data.value), 12300000.f);

	json = "123e-7";
	res = jsk_parse(h, json, strlen(json));
	assert_int_equal(res.status, JSK_OK);
	assert_int_equal(res.data.value.type, JSK_FLOAT);
	assert_int_equal(jsk_get_float(res.data.value), 0.0000123f);

	json = "1.23e3";
	res = jsk_parse(h, json, strlen(json));
	assert_int_equal(res.status, JSK_OK);
	assert_int_equal(res.data.value.type, JSK_FLOAT);
	assert_int_equal(jsk_get_float(res.data.value), 1230.f);

	jsk_heap_free(h);
}

static void test_parse_strings(void **state)
{
	(void)state;

	char *json;
	jsk_result res;
	jsk_heap *h = jsk_heap_new(NULL);

	json = "\"Hello World\"";
	res = jsk_parse(h, json, strlen(json));
	assert_int_equal(res.status, JSK_OK);
	assert_int_equal(res.data.value.type, JSK_STRING);
	assert_string_equal(jsk_get_string(res.data.value), "Hello World");

	json = "\"Hello\\nWorld\"";
	res = jsk_parse(h, json, strlen(json));
	assert_int_equal(res.status, JSK_OK);
	assert_int_equal(res.data.value.type, JSK_STRING);
	assert_string_equal(jsk_get_string(res.data.value), "Hello\nWorld");

	json = "\"Hello\\\"World\"";
	res = jsk_parse(h, json, strlen(json));
	assert_int_equal(res.status, JSK_OK);
	assert_int_equal(res.data.value.type, JSK_STRING);
	assert_string_equal(jsk_get_string(res.data.value), "Hello\"World");

	jsk_heap_free(h);
}

static void test_parse_arrays(void **state)
{
	(void)state;

	char *json;
	jsk_result res;
	jsk_value a, b;
	jsk_heap *h = jsk_heap_new(NULL);

	json = "[ true,  6, \"foo\", [], [null] ]";
	res = jsk_parse(h, json, strlen(json));
	assert_int_equal(res.status, JSK_OK);
	a = res.data.value;
	assert_int_equal(a.type, JSK_ARRAY);
	assert_int_equal(jsk_array_length(a), 5);
	b = jsk_array_at(a, 0);
	assert_int_equal(b.type, JSK_BOOL);
	assert_true(jsk_get_bool(b));
	b = jsk_array_at(a, 1);
	assert_int_equal(b.type, JSK_INT);
	assert_int_equal(jsk_get_int(b), 6);
	b = jsk_array_at(a, 2);
	assert_int_equal(b.type, JSK_STRING);
	assert_string_equal(jsk_get_string(b), "foo");
	b = jsk_array_at(a, 3);
	assert_int_equal(b.type, JSK_ARRAY);
	assert_int_equal(jsk_array_length(b), 0);
	b = jsk_array_at(a, 4);
	assert_int_equal(b.type, JSK_ARRAY);
	assert_int_equal(jsk_array_length(b), 1);
	b = jsk_array_at(b, 0);
	assert_int_equal(b.type, JSK_NULL);
	assert_null(b.value);

	jsk_heap_free(h);
}

static void test_parse_objects(void **state)
{
	(void)state;

	jsk_value *v, *w;
	jsk_heap *h = jsk_heap_new(NULL);

	const char *json =
		"{"
		"	\"hello\": \"world\","
		"	\"subobject\": {"
		"		\"foo\": \"bar\","
		"		\"property\": 123"
		"	}"
		"}";

	const jsk_result res = jsk_parse(h, json, strlen(json));
	assert_int_equal(res.status, JSK_OK);

	const jsk_value a = res.data.value;
	assert_int_equal(a.type, JSK_OBJECT);
	assert_int_equal(jsk_object_count(a), 2);

	v = jsk_object_get(a, "aMissingKey");
	assert_null(v);

	v = jsk_object_get(a, "hello");
	assert_non_null(v);
	assert_int_equal(v->type, JSK_STRING);
	assert_string_equal(jsk_get_string_p(v), "world");

	v = jsk_object_get(a, "subobject");
	assert_non_null(v);
	assert_int_equal(v->type, JSK_OBJECT);
	assert_int_equal(jsk_object_count(*v), 2);

	w = jsk_object_get(*v, "anotherMissingKey");
	assert_null(w);

	w = jsk_object_get(*v, "foo");
	assert_non_null(w);
	assert_int_equal(w->type, JSK_STRING);
	assert_string_equal(jsk_get_string_p(w), "bar");

	w = jsk_object_get(*v, "property");
	assert_non_null(w);
	assert_int_equal(w->type, JSK_INT);
	assert_int_equal(jsk_get_int_p(w), 123);

	jsk_heap_free(h);
}

static void test_to_string_simple_values(void **state)
{
	(void)state;

	char *s;
	jsk_value v;
	jsk_heap *h = jsk_heap_new(NULL);

	v = jsk_new_bool(1);
	s = jsk_to_string(h, v);
	assert_string_equal(s, "true");
	free(s);

	v = jsk_new_bool(0);
	s = jsk_to_string(h, v);
	assert_string_equal(s, "false");
	free(s);

	v = jsk_new_null();
	s = jsk_to_string(h, v);
	assert_string_equal(s, "null");
	free(s);

	v = jsk_new_int(123456);
	s = jsk_to_string(h, v);
	assert_string_equal(s, "123456");
	free(s);

	v = jsk_new_float(123.456);
	s = jsk_to_string(h, v);
	assert_string_equal(s, "123.456000");
	free(s);

	jsk_heap_free(h);
}

static void test_to_string_strings(void **state)
{
	(void)state;

	char *s;
	jsk_value v;
	jsk_heap *h = jsk_heap_new(NULL);

	v = jsk_new_string(h, "Hello World");
	s = jsk_to_string(h, v);
	assert_string_equal(s, "\"Hello World\"");
	free(s);

	v = jsk_new_string(h, "Hello\n\b\"World");
	s = jsk_to_string(h, v);
	assert_string_equal(s, "\"Hello\\n\\b\\\"World\"");
	free(s);

	jsk_heap_free(h);
}

static void test_to_string_arrays(void **state)
{
	(void)state;

	char *s;
	jsk_value v;
	jsk_heap *h = jsk_heap_new(NULL);

	v = jsk_new_array();
	jsk_array_push(h, &v, jsk_new_null());
	jsk_array_push(h, &v, jsk_new_bool(0));
	jsk_array_push(h, &v, jsk_new_bool(1));
	jsk_array_push(h, &v, jsk_new_array());
	s = jsk_to_string(h, v);
	assert_string_equal(s, "[null,false,true,[]]");
	free(s);

	jsk_heap_free(h);
}

static void test_to_string_objects(void **state)
{
	(void)state;

	const char *json =
		"{"
		"	\"hello\": \"world\","
		"	\"subobject\": {"
		"		\"foo\": \"bar\","
		"		\"property\": 123"
		"	}"
		"}";

	jsk_heap *h = jsk_heap_new(NULL);

	jsk_result res = jsk_parse(h, json, strlen(json));
	jsk_value v = res.data.value;

	char *s0 = jsk_to_string(h, v);

	res = jsk_parse(h, s0, strlen(s0));
	v = res.data.value;

	char *s1 = jsk_to_string(h, v);

	assert_string_equal(s0, s1);

	free(s0);
	free(s1);

	jsk_heap_free(h);
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_heap),
		cmocka_unit_test(test_heap_oversized),
		cmocka_unit_test(test_simple_values),
		cmocka_unit_test(test_strings),
		cmocka_unit_test(test_arrays),
		cmocka_unit_test(test_objects),
		cmocka_unit_test(test_parse_simple_values),
		cmocka_unit_test(test_parse_strings),
		cmocka_unit_test(test_parse_arrays),
		cmocka_unit_test(test_parse_objects),
		cmocka_unit_test(test_to_string_simple_values),
		cmocka_unit_test(test_to_string_strings),
		cmocka_unit_test(test_to_string_arrays),
		cmocka_unit_test(test_to_string_objects),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
