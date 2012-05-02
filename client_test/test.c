#include "test.h"

#define GAM_USE_LIBPNG
#define GAM_USE_SDLTTF
#include "libgam.h"

#define TAIL_COUNT 10

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

	if(gam_init(GAM_ALL, 1) != EXIT_SUCCESS)
		abort();

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


	TTF_Font *ttf = gam_init_sdlttf("StayPuft.ttf", 22);
	if(ttf == NULL)
		abort();
	Uint32 text_color = SDL_MapRGB(screen.surface->format, 0, 0, 0);
	
	gam_state->print_mode = GAM_PRINT_BLEND;
	gam_state->alpha = SDL_MapRGB(screen.surface->format, 140, 140, 160);
	/*********************************************************************/ 

	typedef struct { 
		int x, y, z;
	} point_t;
	
	point_t point[TAIL_COUNT];

	int i;
	for(i = 0; i < TAIL_COUNT; i++) {
		point[i].x = (.5 * screen.w - .5 * spot->w);
		point[i].y = (.5 * screen.h - .5 * spot->h);
		point[i].z = 0;
	}

	memset(&buf, 0, BUF_SIZE);
	ftdi_usb_purge_rx_buffer(ftdic);

	SDL_Event event;
	SDL_Rect rect;
	int busy = 1;
	while(busy) {

		while(SDL_PollEvent(&event))
			if(event.type == SDL_KEYDOWN)
				if(event.key.keysym.sym == SDLK_ESCAPE)
					busy = 0;
	
		SDL_BlitSurface(backdrop, NULL, screen.surface, NULL);

		rxb = ftdi_read_data(ftdic, buf, BUF_SIZE);

		if(rxb > 0) {
			if(rxb == BUF_SIZE) {

				if(buf[BUF_SIZE - 1] == 10 && 
				   buf[BUF_SIZE - 2] == 13) {

						int16_t x, y, z;

						x = (buf[1] << 8) ^ buf[0];
						y = (buf[3] << 8) ^ buf[2];
						z = (buf[5] << 8) ^ buf[4];


						printf("\e[32mGREAT SUCCESS\e[0m %4d %4d %4d\n", x, y, z);

						//gam_print_core(screen.surface, 0, 0, text_color, "X: %d", x);
						gam_print(screen.surface, 0, 0, ttf, text_color, "X: %1.2f", x / 256.0);
						gam_print(screen.surface, 0, 23, ttf, text_color, "Y: %1.2f", y / 256.0);
						gam_print(screen.surface, 0, 46, ttf, text_color, "Z: %1.2f", z / 256.0);

						//printf("%1.3f %1.3f %1.3f\n", 
						//       x / 256.0, 
						//       y / 256.0, 
						//       z / 256.0);

						point[0].x = (.5 * screen.w - .5 * spot->w) + x;
						point[0].y = (.5 * screen.h - .5 * spot->h) + y;

						for(i = TAIL_COUNT - 1; i > 0; i--) {
							point[i].x = point[i - 1].x;
							point[i].y = point[i - 1].y;
							point[i].z = point[i - 1].z;
						}

						for(i = TAIL_COUNT - 1; i >= 0; i--) {
							rect.x = point[i].x;
							rect.y = point[i].y;
							SDL_SetAlpha(spot, 
								     SDL_SRCALPHA, 
								     (255 / TAIL_COUNT) * (TAIL_COUNT - i));
							SDL_BlitSurface(spot, NULL, screen.surface, &rect);
						}

						SDL_Flip(screen.surface);

				} else {
					fprintf(stderr, "Packet of right length but wrong signature\n");
				}
			} else if(rxb < BUF_SIZE) { // Underrun? 
				fprintf(stderr, "Packet of unknown size %ld\n", rxb);
			} else {
				abort();
			}
		} else if(rxb < 0){
			fprintf(stderr, "RX Error: %ld: %s\n", rxb, ftdi_get_error_string(ftdic));
		}

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
