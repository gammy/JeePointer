#include "test.h"

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

	struct ftdi_context *ftdic = bub_init(57600, 1, 0, BUF_SIZE);

	if(ftdic == NULL)
		abort();
	
	printf("Ready.\n");
	
	uint8_t buf[BUF_SIZE];
	memset(&buf, 0, BUF_SIZE);

	long rxb = -1;

	for(;;) {

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
			}
		} else {
			if(rxb != 0)
				printf("Nope, got %ld\n", rxb);
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
