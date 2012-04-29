#include "test.h"

#define GAM_USE_LIBPNG
#include "libgam.h"


struct ftdi_context *
bub_init(unsigned int baud_rate,
	 unsigned char latency,
	 unsigned int tx_buf_size,
	 unsigned int rx_buf_size) {

	int ret = 0;

	struct ftdi_context *ftdic;
	
	ftdic = malloc(sizeof(struct ftdi_context));
   
	if(ftdic == NULL) {
		perror("malloc");
		return(NULL);
	}

	ret = ftdi_init(ftdic);
	if (ret < 0) {
		fprintf(stderr, "ftdi_init failed: %d.\n", ret);
		return(NULL);
	}

	ftdi_set_interface(ftdic, INTERFACE_ANY);

	ret = ftdi_usb_open(ftdic, 0x0403, 0x6001); // FIXME make nice defines
	if(ret < 0) {
		fprintf(stderr, "unable to open ftdi device: %d (%s)\n", 
			ret, ftdi_get_error_string(ftdic));
		return(NULL);
	}

	if(ftdi_usb_reset(ftdic) != 0)
		fprintf(stderr, "WARN: ftdi_usb_reset failed!\n");

	ftdi_disable_bitbang(ftdic);

	if (ftdi_set_baudrate(ftdic, baud_rate) < 0) {
		fprintf(stderr, "Unable to set baudrate: (%s)\n", 
			ftdi_get_error_string(ftdic));
		return(NULL);
	} 

	ftdi_set_latency_timer(ftdic, latency);

	if(tx_buf_size > 0)
		ftdi_write_data_set_chunksize(ftdic, tx_buf_size);
	if(rx_buf_size > 0)
		ftdi_read_data_set_chunksize(ftdic, rx_buf_size);

	return(ftdic);
}

#define BUF_SIZE	8

int main(int argc, char *argv[]) {

	/** COMMS ************************************************************/ 

	struct ftdi_context *ftdic = bub_init(57600, 1, 0, BUF_SIZE);

	if(ftdic == NULL)
		abort(); // FIXME
	
	printf("Ready.\n");
	
	uint8_t buf[BUF_SIZE];
	memset(&buf, 0, BUF_SIZE);

	long rxb = -1;

	/** GFX **************************************************************/ 

	gam_screen screen = {
		.w       = 512,
		.h       = 512,
		.depth   = 32,
		.flags   = SDL_HWACCEL | SDL_DOUBLEBUF,
		.info    = 0,
		.surface = NULL
	};

	if(gam_video_init(&screen) != 0)
		return(EXIT_FAILURE);
	
	SDL_Surface *backdrop = 
			gam_get_surface_from_png_file("backdrop.png", SDL_HWSURFACE);
	if(backdrop == NULL){
		fprintf(stderr, "Can't allocate surface: %s\n", SDL_GetError());
		return(EXIT_FAILURE);
	}

	
	SDL_Surface *spot = gam_get_surface_from_png_file("spot.png", SDL_HWSURFACE);
	//SDL_Surface *spot = gam_get_surface_sphere(screen.surface, 100);
	if(spot == NULL){
		fprintf(stderr, "Can't allocate surface: %s\n", SDL_GetError());
		return(EXIT_FAILURE);
	}

	SDL_SetColorKey(spot, 
			SDL_SRCCOLORKEY,
			SDL_MapRGB(spot->format, 255, 255, 255));

	if(spot == NULL){
		fprintf(stderr, "Can't allocate surface: %s\n", SDL_GetError());
		return(EXIT_FAILURE);
	}

	/*********************************************************************/ 

	struct { 
		int x, y, z;
	} point = {.5 * screen.w, .5 * screen.w, 0};
	//} point = {0, 0, 0};

	SDL_Event event;
	SDL_Rect rect;
	int busy = 1;
	while(busy) {

		while(SDL_PollEvent(&event))
			if(event.type == SDL_KEYDOWN)
				if(event.key.keysym.sym == SDLK_ESCAPE)
					busy = 0;
	
		SDL_BlitSurface(backdrop, NULL, screen.surface, NULL);

		memset(&buf, 0, BUF_SIZE);
		rxb = ftdi_read_data(ftdic, buf, BUF_SIZE);

		if(rxb == BUF_SIZE) {

			if(buf[BUF_SIZE - 1] == 10 && 
			   buf[BUF_SIZE - 2] == 13) {

					int16_t x, y, z;

					x = (buf[1] << 8) ^ buf[0];
					y = (buf[3] << 8) ^ buf[2];
					z = (buf[5] << 8) ^ buf[4];

					printf("%4d %4d %4d\n", x, y, z);

					rect.x = point.x - (.5 * spot->w);
					rect.y = point.y - (.5 * spot->h);
			}
		} else {
			if(rxb != 0)
				printf("Nope, got %ld\n", rxb);
		}

		SDL_BlitSurface(spot, NULL, screen.surface, &rect);
		SDL_Flip(screen.surface);
	}

	int ret = 0;
	if ((ret = ftdi_usb_close(ftdic)) < 0) {
		fprintf(stderr, "Unable to close ftdi device: %d (%s)\n", 
			ret,
		       	ftdi_get_error_string(ftdic));
		return(EXIT_FAILURE);
	}

	ftdi_deinit(ftdic);

	return(EXIT_SUCCESS);
}
