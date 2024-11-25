#include <stdio.h>
#include <string>
#include <fcntl.h>   // Contains file controls like O_RDWR
#include <errno.h>   // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control defintions
#include <unistd.h>  // write(), read(), close()
#include <csignal>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <curl/curl.h>
using namespace std;
using namespace cv;

#define ERR_CURL_INIT -1
#define ERR_NO_MEMORY -2
#define ERR_SER_OPEN  -3
#define ERR_MISC      -4

#define DEF_URL  "http://192.168.7.15/bmp"
#define DEF_PORT "/dev/ttyUSB0"

#define MSG_START ':'
#define MSG_SEP   ';'
#define MSG_END   '\r'

#define FLAGS_SIZE 3

typedef uint8_t Byte;

char def_url[] = DEF_URL;

// Opening serial port
int serial_port = open(DEF_PORT, O_RDWR);

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

int init_curl(char url[] = def_url)
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

void cleanup()
{
	curl_easy_cleanup(curl_handle);
	free(chunk.memory);
	curl_global_cleanup();
    close(serial_port);
}

void sgint_handler(int signal) { cleanup(); }

int main(int argc, char *argv[])
{
	// It will be resized in `init_curl()`
	chunk.memory = (Byte*)calloc(1, sizeof(char));
	// No data yet
	chunk.size   = 0;

    if (serial_port < 0) {
        printf("Error %i from open: %s\n", errno, strerror(errno));
        return ERR_SER_OPEN;
    }

    // Access termios struct in order to configure serial port
    struct termios tty;

    // Write existing configuration of the serial port to tty and set our own
    // configuration
    if (tcgetattr(serial_port, &tty) != 0) {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        return ERR_MISC;
    }
    tty.c_cflag &= ~PARENB; // Clear parity bit
    tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in
                            // communication (most common)
    tty.c_cflag &= ~CSIZE; // Clear all the size bits, then use one of the
                           // statements below
    tty.c_cflag |= CS8; // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines
    tty.c_lflag &= ~ICANON; // Disable canonical mode
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    // Disable any special handling of received bytes
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

    tty.c_cc[VTIME] = 0; // read() does not block (we don't read anyway)
    tty.c_cc[VMIN] =  0;

    // Set in/out baud rate
    cfsetispeed(&tty, B0);    // Input :    0
    cfsetospeed(&tty, B9600); // Output: 9600

    // Save tty settings
    if (tcsetattr(serial_port, TCSANOW, &tty)) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        return ERR_MISC;
    }

    signal(SIGINT, sgint_handler);

    int init = init_curl();
    if (init)
      return init;

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
                       filt.rows / 16, /* For distance between circles */
                       100, 30, 1 /* minRadius */, 100 /* maxRadius */);

          int x, y, time_stmp = -1;

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

          // Writing to serial
          if (x>0 and y>0 and time_stmp>0) {
            unsigned char x_char = static_cast<unsigned char>(x);
            unsigned char y_char = static_cast<unsigned char>(y);
            unsigned char time_stmp_char = static_cast<unsigned char>(time_stmp);
            unsigned char msg[] = {
                MSG_START,
                x_char, MSG_SEP,
                y_char, MSG_SEP,
                time_stmp_char,
                MSG_END
            };
            write(serial_port, msg, sizeof(msg));
          }

          imshow("Detected circles", img);
          waitKey();
        }
      }
	}

	return 0;
}
