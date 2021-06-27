#define JSKOROST_IMPLEMENTATION
#include "jskorost.h"
#include <time.h>

#include "rapidjson/include/rapidjson/document.h"
#include "rapidjson/include/rapidjson/writer.h"
#include "rapidjson/include/rapidjson/stringbuffer.h"

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

	char *buffer = (char *)malloc(length);
	if (!buffer) {
		fprintf(stderr, "Can't allocate file buffer\n");
		return 1;
	}

	fread(buffer, 1, length, f);
	fclose(f);

	jsk_heap *h = jsk_heap_new(NULL);

	clock_t start, end;
	double time_used;
	jsk_result res;

	std::string s(buffer, length);
	const char *cs = s.c_str();

	start = clock();

	rapidjson::Document d;
	d.Parse(cs);

	end = clock();

	time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

	printf("RapidJSON successfully parsed: %f\n", time_used);

	start = clock();

	res = jsk_parse(h, buffer, length);
	if (res.status != JSK_OK) {
		fprintf(stderr, "JSkorost failed to parse JSON: %s\n", res.data.error);
		return 1;
	}

	end = clock();

	jsk_heap_free(h);

	time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

	printf("JSkorost successfully parsed: %f (%f/sec)\n", time_used,
			length / time_used);

	return 0;
}
