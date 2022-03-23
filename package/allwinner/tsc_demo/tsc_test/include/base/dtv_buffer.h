/*
 * dtv_buffer.h
 *
 *  Created on: 2014-3-24
 *      Author: wei
 */

#ifndef DTV_BUFFER_H_
#define DTV_BUFFER_H_

/*
 * Notice:
 * Must hold a lock before calling these functions. *
 */

typedef struct DTV_BUFFER {
	uint8_t *buf;//virtual address, differ with tsc.
	int32_t buf_size;//total buffer size
	int32_t rd_pos;//read position
	int32_t wr_pos;//write position
	int32_t free_size;//free buffer size
} dtv_buffer_t;

dtv_buffer_t *dtv_buffer_create(int size);

int dtv_buffer_destroy(dtv_buffer_t *buffer);

void dtv_buffer_reset(dtv_buffer_t *buffer);

int dtv_buffer_write(dtv_buffer_t *buffer, void *data, int size);

int dtv_buffer_read(dtv_buffer_t *buffer, void *data, int size);

int dtv_buffer_request(dtv_buffer_t *buffer, int request_size, uint8_t **data,
		int *size, uint8_t **ring_data, int *ring_size);

int dtv_buffer_update(dtv_buffer_t *buffer, int size);

//@return:valid data size
int dtv_buffer_get_size(dtv_buffer_t *buffer);

//@return:free buffer size
int dtv_buffer_get_free_size(dtv_buffer_t *buffer);

//@return:total buffer size
int dtv_buffer_get_capacity(dtv_buffer_t *buffer);

#endif /* DTV_BUFFER_H_ */
