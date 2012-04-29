#include "test.h"

/**
  * Initialize libftdic and the chip.
  * @returns *ftdic_context or NULL
  */
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

	unsigned char tmp = 0;

	unsigned int i = 0;

	return(ftdic);
}

int main(int argc, char *argv[]) {

	struct ftdi_context *ftdic = bub_init(57600, 1, 0, 0);

	if(ftdic == NULL)
		abort();
	
	printf("Ready.\n");
	
	unsigned char buf[512];
	memset(&buf, 0, 512);

	int offs = 0;
	long rxb = -1;

	for(;;) {

		unsigned char c = 0;
		rxb = ftdi_read_data(ftdic, &c, 1);
		if(rxb == 1 && c != 0) {
			buf[offs] = c;
			offs++;
			if(c == 10) {
				buf[offs] = buf[offs - 1] = 0;
				//printf("Buf, %d bytes: %s", offs, buf);
				puts(buf);
				memset(&buf, 0, 512);
				offs = 0;
			}
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
