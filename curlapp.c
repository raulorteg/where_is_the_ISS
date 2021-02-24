#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <json-c/json.h>
#include <time.h>
#include <string.h>
#include <math.h>

// memory block to store reqests response
struct memory {
	char *memory;
	size_t size;
};

// functions for request writebacks
const char* getApiFile(char *jsonKey);
static size_t writecallback(char *contents, size_t size, size_t nmemb, void *userp);

// curl functions http get requests
struct memory curl_request_iss(void);
struct memory curl_request_location(const char *placename);

// json parsers for http get request responses
void json_parser_iss(const char *buffer, float *issLon, float *issLat, float *issAlt);
void json_parser_getCoordinates(const char *buffer, float *meanLat, float *meanLon, const char* placename);

// trigonometry and other math functions
float degreesToRadians(float angleDeg);
float radiansToDegrees(float angleRad);
float geodesicDistance(float LatA, float LonA, float LatB, float LonB);
float chordLength(float geoDist);
float distanceToIss(float chordlen, float height);
float computeElevation(float chordLen, float height);
void ComputeRelativeAngles(float issLat, float issLon, float issAlt, // Iss coordinates
							float Lat, float Long, float Alt,  // Your position
							float *horzAngle, float *elevAngle); // pointers to angles

int main(int argc, char *argv[])
{
	time_t begin = time(NULL);
	float issLon, issLat, issAlt;
	float bearing, elevAngle;
	float latitude, longitude, Alt = 0.0;
	const char *placename = "Copenhaguen";

	// Api request for location coords based on location name
	struct memory LocChunk = curl_request_location(placename);
	json_parser_getCoordinates(LocChunk.memory, &latitude, &longitude, placename);
	printf("Latitude: %f , Longitude: %f \n\n", latitude, longitude);

	// api request for ISS position (Lat, Lon, Alt)
	struct memory chunk = curl_request_iss();
	json_parser_iss(chunk.memory, &issLon, &issLat, &issAlt);
	printf("Space Station position: \nLat: %f, Long: %f, Alt: %f \n\n", issLat, issLon, issAlt);

	// do the math for the angles (bearing, elevation)
	ComputeRelativeAngles(issLat, issLon, issAlt, latitude, longitude, Alt, &bearing, &elevAngle);
	printf("Bearing: %f, Elevation: %f \n\n", bearing, elevAngle);

	free(chunk.memory);
	free(LocChunk.memory);
	printf("--finished %f s--\n", difftime(time(NULL), begin));
	return 0;
}

struct memory curl_request_iss(void)
{
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
			fprintf(stderr, "curl_easy_perform() %s\n", curl_easy_strerror(res));
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

struct memory curl_request_location(const char *placename)
{
	const char* ApiKey = getApiFile("API_KEY"); // load .env api key

	CURL *curl_handle;
	CURLcode res;

	struct memory LocChunk;
	LocChunk.memory = NULL;
	LocChunk.size = 0;

	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();

	if (curl_handle) {
		char urlRequestLoc[300];
		sprintf(urlRequestLoc, "https://api.opencagedata.com/geocode/v1/json?q=%s&key=%s", placename, ApiKey);
		curl_easy_setopt(curl_handle, CURLOPT_URL, urlRequestLoc);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writecallback);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &LocChunk);

		res = curl_easy_perform(curl_handle);
		if (res != CURLE_OK) {
			fprintf(stderr, "curl easy perform() %s\n", curl_easy_strerror(res));
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
	return LocChunk;
}

static size_t writecallback(char *contents, size_t size, size_t nmemb, void *userp)
{
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

void json_parser_iss(const char *buffer, float *issLon, float *issLat, float *issAlt)
{
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

	*issLon = json_object_get_double(longitude);
	*issLat = json_object_get_double(latitude);
	*issAlt = json_object_get_double(altitude);
}

void json_parser_getCoordinates(const char *buffer, float *meanLat, float *meanLon, const char* placename)
{
	size_t n_results;

	struct json_object *parsed_json;
	struct json_object *status;
	struct json_object *status_code;
	struct json_object *status_message;
	struct json_object *results;
	struct json_object *result;
	struct json_object *bounds;
	struct json_object *northeast;
	struct json_object *southwest;
	struct json_object *latNortheast;
	struct json_object *lonNortheast;
	struct json_object *latSouthwest;
	struct json_object *lonSouthwest;

	parsed_json = json_tokener_parse(buffer);

	json_object_object_get_ex(parsed_json, "status", &status);
	json_object_object_get_ex(status, "code", &status_code);
	json_object_object_get_ex(status, "message", &status_message);

	json_object_object_get_ex(parsed_json, "results", &results);
	n_results = json_object_array_length(results);
	printf("Found %lu matching results for (%s).\n", n_results, placename);

	result = json_object_array_get_idx(results, 0);
	json_object_object_get_ex(result, "bounds", &bounds);
	json_object_object_get_ex(bounds, "northeast", &northeast);
	json_object_object_get_ex(bounds, "southwest", &southwest);
	json_object_object_get_ex(northeast, "lat", &latNortheast);
	json_object_object_get_ex(northeast, "lng", &lonNortheast);
	json_object_object_get_ex(southwest, "lat", &latSouthwest);
	json_object_object_get_ex(southwest, "lng", &lonSouthwest);

	float latitudeNortheast = json_object_get_double(latNortheast);
	float longitudeNortheast = json_object_get_double(lonNortheast);
	float latitudeSouthwest = json_object_get_double(latSouthwest);
	float longitudeSouthwest = json_object_get_double(lonSouthwest);

	*meanLat = 0.5 * (latitudeNortheast + latitudeSouthwest);
	*meanLon = 0.5 * (longitudeNortheast + longitudeSouthwest);

	int StatusCodeResponse = json_object_get_int(status_code);
	const char *StatusMessageResponse = json_object_get_string(status_message);
	printf("Status response: %d %s \n", StatusCodeResponse, StatusMessageResponse);
}

const char* getApiFile(char *jsonKey)
{
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

float degreesToRadians(float angleDeg)
{
	return angleDeg * (float)M_PI / 180.0;
}

float radiansToDegrees(float angleRad)
{
	return angleRad * 180.0 / (float)M_PI;
}

float geodesicDistance(float LatA, float LonA, float LatB, float LonB)
{
	float R = 6372.795477598; // km (radius quadric medium)
	return R * acos(sin(LatA) * sin(LatB) + cos(LatA) * cos(LatB) * cos(LonA-LonB)); 
}

float chordLength(float geoDist)
{
	float R = 6372.795477598;
	float angDistance = geoDist / R;
	return 2 * R * sin(angDistance/2.0);
}

void ComputeRelativeAngles(float issLat, float issLon, float issAlt, float Lat, float Lon, float Alt, float *bearing, float *elevAngle)
{
	issLat = degreesToRadians(issLat);
	issLon = degreesToRadians(issLon);
	Lat = degreesToRadians(Lat);
	Lon = degreesToRadians(Lon);

	float varPhi = log(tan(issLat/2. + (float)M_PI/4.) / tan(Lat/2. + (float)M_PI/4.));
	float varLon = abs(Lon - issLon);
	float geoDist = geodesicDistance(issLat, issLon, Lat, Lon);
	float chordLen= chordLength(geoDist);
	float trueDistance = distanceToIss(chordLen, issAlt);

	printf("Distance (flat projection): %f  (km)\n", geoDist);
	printf("Distance (chord lenght): %f  (km)\n", chordLen);
	printf("Total Distance: %f  (km)\n", trueDistance);


	*bearing = atan2(varLon, varPhi);
	*elevAngle = computeElevation(chordLen, issAlt);
}

float distanceToIss(float chordLen, float height)
{
	float R = 6372.795477598; // km (radius quadric medium)
	float beta2 = asin(sqrt(1 - 0.25 * pow(chordLen,2) * pow(R,-2)));
	float beta1 = 0.5 * (M_PI - beta2);
	float alpha2 = M_PI - beta1;
	return sqrt(pow(chordLen,2) + pow(height,2) - 2 * chordLen * height * cos(alpha2));
}

float computeElevation(float chordLen, float height)
{
	float R = 6372.795477598; // km (radius quadric medium)
	float beta2 = asin(sqrt(1 - 0.25 * pow(chordLen,2) * pow(R,-2)));
	float beta1 = 0.5 * (M_PI - beta2);
	float alpha2 = M_PI - beta1;
	float trueDistance =  sqrt(pow(chordLen,2) + pow(height,2) - 2 * chordLen * height * cos(alpha2));
	return asin(0.5 * (R + height) * sin(beta2) / trueDistance);
}