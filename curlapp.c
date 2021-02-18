#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>
#include <time.h>
#include <string.h>

/* lets get all the data into a memory block */
struct memory {
	char *memory;
	size_t size;
};

void json_parser_iss(const char *buffer);
// TODO: void json_parser_location(const char *buffer);

static size_t writecallback(char *contents, size_t size, size_t nmemb, void *userp);
struct memory curl_request_iss(void);
struct memory curl_request_location(void);
const char* getApiFile(char *jsonKey);

int main(void) {
	time_t begin = time(NULL);

	struct memory chunk = curl_request_iss();
	json_parser_iss(chunk.memory);

	struct memory LocChunk = curl_request_location();
	// TODO: void json_parser_location(const char *buffer);

	free(chunk.memory);
	free(LocChunk.memory);
	printf("--finished %f s--\n", difftime(time(NULL), begin));
	return 0;
}

struct memory curl_request_iss(void) {
	CURL *curl;
	CURLcode res;
	struct memory chunk;

	chunk.memory = NULL;
	chunk.size = 0;
	curl_global_init(CURL_GLOBAL_ALL);

	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "https://api.wheretheiss.at/v1/satellites/25544"); // 25544 is ISS' id
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writecallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);

		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() returned %s\n", curl_easy_strerror(res));
		}

		else {
			char *domain = NULL;
			// printf("We got %d bytes to our callback in memory %p\n", (int)chunk.size, chunk.memory);
			domain = strstr(chunk.memory, "Domain");

			if (domain) {
				// printf("Found 'domain' at index %d\n", (domain - chunk.memory));
			}
		}
		curl_easy_cleanup(curl);
	}

	curl_global_cleanup();
	return chunk;
}

struct memory curl_request_location(void) {

	// load .env api key
	const char* ApiKey = getApiFile("API_KEY");

	CURL *curl_handle;
	CURLcode res;

	struct memory LocChunk;
	LocChunk.memory = NULL;
	LocChunk.size = 0;

	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();

	if (curl_handle) {
		char *placename = "Copenhaguen"; // temp hardcoded
		char urlRequestLoc[50];
		sprintf(urlRequestLoc, "https://api.opencagedata.com/geocode/v1/json?q=%s&key=%s", placename, ApiKey);

		curl_easy_setopt(curl_handle, CURLOPT_URL, urlRequestLoc);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writecallback);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &LocChunk);

		res = curl_easy_perform(curl_handle);
		if (res != CURLE_OK) {
			fprintf(stderr, "curl easy perform() returned %s\n", curl_easy_strerror(res));
		}

		else {
			char *domain = NULL;
			domain = strstr(LocChunk.memory, "Domain");

			if (domain) {
				// printf("Found 'domain' at index %d\n", (domain - chunk.memory));
			}
		}
		curl_easy_cleanup(curl_handle);
	}
	curl_global_cleanup();
	printf("%s \n", LocChunk.memory);
	return LocChunk;
}

/* if received data doesnt match expected size then terminates program
this function is used to jump over that condition*/
static size_t writecallback(char *contents, size_t size, size_t nmemb, void *userp) {
	size_t realsize = size * nmemb;
	struct memory *mem = (struct memory *)userp;

	char *ptr = realloc(mem->memory, mem->size + realsize + 1);
	if (ptr == NULL)
		return 0; // then will return different value than realsize which acts as error

	mem->memory = ptr;
	memcpy(&mem->memory[mem->size], contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0; // to terminate the data
	return realsize;
}

// json parser for satellite api query
void json_parser_iss(const char *buffer) {
	struct json_object *parsed_json;
	struct json_object *name;
	struct json_object *id;
	struct json_object *latitude;
	struct json_object *longitude;
	struct json_object *altitude;
	struct json_object *velocity;
	struct json_object *timestamp;

	parsed_json = json_tokener_parse(buffer);

	json_object_object_get_ex(parsed_json, "name", &name);
	json_object_object_get_ex(parsed_json, "id", &id);
	json_object_object_get_ex(parsed_json, "latitude", &latitude);
	json_object_object_get_ex(parsed_json, "longitude", &longitude);
	json_object_object_get_ex(parsed_json, "altitude", &altitude);
	json_object_object_get_ex(parsed_json, "velocity", &velocity);
	json_object_object_get_ex(parsed_json, "timestamp", &timestamp);

	printf("Name: %s\n", json_object_get_string(name));
	printf("Id: %d\n", json_object_get_int(id));
	printf("Latitude: %lf\n", json_object_get_double(latitude));
	printf("Longitude: %lf\n", json_object_get_double(longitude));
	printf("Altitude: %lf\n", json_object_get_double(altitude));
	printf("Timestamp: %d\n", json_object_get_int(timestamp));
}


const char* getApiFile(char *jsonKey) {
	FILE *fp;
	char buffer[1024];
	fp = fopen("env.json", "r");
	if (!fp) {
		printf("Error: could open env.json file.\n");
		exit(1);
	}
	fread(buffer, 1024, 1, fp);
	fclose(fp);

	struct json_object *parsed_json;
	struct json_object *ApiKey;

	parsed_json = json_tokener_parse(buffer);
	json_object_object_get_ex(parsed_json, jsonKey, &ApiKey);

	const char* ApiKeyString = json_object_get_string(ApiKey);
	return  ApiKeyString;
}