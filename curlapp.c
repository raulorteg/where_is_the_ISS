#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>
#include <time.h>

/* lets get all the data into a memory block */
struct memory {
	char *memory;
	size_t size;
};

void json_parser_iss(const char *buffer);
void json_parser_location(const char *buffer); // TODO:

static size_t writecallback(char *contents, size_t size, size_t nmemb, void *userp);
struct memory curl_request_iss(void);
struct memory curl_request_location(void); // TODO:

int main(void) {
	time_t begin, end;
	begin = time(NULL);

	struct memory chunk = curl_request_iss();
	json_parser_iss(chunk.memory);

	free(chunk.memory);
	end = time(NULL);
	printf("--finished %f s--\n", difftime(end, begin));
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

		// send all data to the writeback
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writecallback);
		// save data in memory
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
		// print contents gotten from http get request
		// printf("%s\n", chunk.memory);

		// call json parser
		//json_parser_iss(chunk.memory);

		//free(chunk.memory);
		curl_easy_cleanup(curl);
	}

	curl_global_cleanup();
	return chunk;
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

	/*
	parsed_name = json_object_get_string(name);
	parsed_id = json_object_get_int(id);
	parsed_latitude = json_object_get_double(latitude);
	parsed_longitude = json_object_get_double(longitude);
	parsed_altitude = json_object_get_double(altitude);
	parsed_timestamp = json_object_get_int(timestamp);
	*/

}