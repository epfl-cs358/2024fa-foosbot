#include <stdio.h>
#include <csignal>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <curl/curl.h>
using namespace std;
using namespace cv;

#define ERR_CURL_INIT -1
#define ERR_NO_MEMORY -2

#define URL_DEF "http://192.168.7.15/bmp"

#define MSG_START ':'
#define MSG_SEP   ';'
#define MSG_END   "\r\n"

#define FLAGS_SIZE 2

typedef uint8_t Byte;

char url_def[] = URL_DEF;

string flags[FLAGS_SIZE*2] = {
	"-u", "--url", "-d", "--debug"
};

typedef struct memory_s {
	Byte *memory;
	size_t   size;
} memory;

CURL    *curl_handle;
CURLcode res;

// TODO: Find a better way than having it as a global variable without clashing
//       with the SGINT handler
// Initialise memory chunk for getting image
memory chunk;

static size_t write_memory(void *content, size_t size,
		                   size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	memory *chunk = (memory*)userp; // TODO: Not very safe

	// Reallocate the chunk with the size of the image received
	Byte *ptr = (Byte*)realloc(chunk->memory, realsize);
	if (not ptr) return ERR_NO_MEMORY;
	chunk->memory = ptr;

	memcpy(&(chunk->memory), content, realsize);
	chunk->size = realsize;

	return realsize;
}

int init_curl(char url[] = url_def)
{
	curl_handle = curl_easy_init();
	if (curl_handle) {
		// Specify URL
		curl_easy_setopt(curl_handle, CURLOPT_URL, url);
		// Set the callback function to pass the data to
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_memory);
		// Pass the memory chunk to the callback function
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)&chunk);
		// Some servers do not like requests without user-agent field, so we add
		// one
		curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
	} else {
		return ERR_CURL_INIT;
	}

	return 0;
}

void cleanup(memory *chunk)
{
	curl_easy_cleanup(curl_handle);
	free(chunk->memory);
	curl_global_cleanup();
}

void sgint_handler(int signal) { cleanup(&chunk); }

int main(int argc, char *argv[])
{
	for (int i = 0; i < argc; i++) {
		for (int j = 0; j < FLAGS_SIZE; j++) {
			// Call corresponding function
		}
	}

	// It will be resized in `init_curl()`
	chunk.memory = (Byte*)calloc(1, sizeof(char));
	// No data yet
	chunk.size   = 0;

	signal(SIGINT, sgint_handler);

	int init = init_curl();
	if (init) return init;

	while (true) {
		res = curl_easy_perform(curl_handle);

		if (res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));
		} else {
			// Process
			vector<Byte> buf(chunk.memory, chunk.memory + chunk.size);

			if (buf.size()) {
				Mat img = imdecode(buf, -1);

				Mat filt;
				cvtColor(img, filt, COLOR_BGR2GRAY);
				medianBlur(filt, filt, 5);

				vector<Vec3f> circles;
				HoughCircles(filt, circles, CV_HOUGH_GRADIENT, 1,
						     filt.rows/16, /* For distance between circles */
							 100, 30, 1 /* minRadius */, 100 /* maxRadius */);

				int x, y = -1;

				if (circles.size()) {
					for (size_t i = 0; i < circles.size(); i++) {
						Vec3i c = circles[i];
						Point center = Point(c[0], c[1]);
						// Circle center
						circle(img, center, 1, Scalar(0, 100, 100), 3);
						// Circle outline
						int radius = c[2];
						circle(img, center, radius, Scalar(255, 0, 255), 3);
					}

					// TODO: That is assuming we only detect the ball
					x = circles[0][0];
					y = circles[0][1];

					int time_stmp = 0; // TODO
				}



				imshow("Detected circles", img);
				waitKey();
			}

			// Send
		}
	}

	return 0;
}
