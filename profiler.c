#define JSKOROST_IMPLEMENTATION
#include "jskorost.h"
#include <time.h>

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "Expected a filename\n");
		return 1;
	}

	const char *const filename = argv[1];
	FILE *f = fopen(filename, "rb");
	if (!f) {
		fprintf(stderr, "Couldn't open file '%s'\n", filename);
		return 1;
	}

	fseek(f, 0, SEEK_END);
	const long length = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *buffer = malloc(length);
	if (!buffer) {
		fprintf(stderr, "Can't allocate file buffer\n");
		return 1;
	}

	fread(buffer, 1, length, f);
	fclose(f);

	jsk_heap *h = jsk_heap_new(NULL);

	clock_t start = clock();

	jsk_result res = jsk_parse(h, buffer, length);
	if (res.status != JSK_OK) {
		fprintf(stderr, "Failed to parse JSON");
		return 1;
	}

	clock_t end = clock();

	jsk_heap_free(h);

	double time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

	printf("Successfully parsed: %f\n", time_used);

	return 0;
}
