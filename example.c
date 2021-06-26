#define JSKOROST_IMPLEMENTATION
#include "jskorost.h"

static const char json[] =
"{"
"	\"color\": \"red\","
"	\"value\": true,"
"	\"another\": null,"
"	\"anArray\": [  "
"            true, false, null, \"hel\\nlo\", 1543, -4, 4.552, 543e-2 "
"        ]"
"}";

int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	jsk_heap *heap = jsk_heap_new(NULL);

	jsk_result res = jsk_parse(heap, json, sizeof(json));

	if (res.status != JSK_OK) {
		printf("Error: %s\n", res.data.error);
		return 1;
	}

	char *s = jsk_to_string(heap, res.data.value);
	printf("Success: %s\n", s);
	free(s);

	jsk_heap_free(heap);
	return 0;
}
