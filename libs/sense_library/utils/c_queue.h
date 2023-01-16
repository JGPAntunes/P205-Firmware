/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file  c_queue.h
 * @brief This is an utility of circular_queue extending its support for the
 * use of the circular queue to save messages. First byte of the data saved 
 * indicates the next message size of the data, this way we can save any type of 
 * messages and don´t use special code to know their size. Messages are saved in a 
 * circular queue, were the first byte indicates the next message size. Example:
 * [2|X|X|3|E|E|E|1|A|4|W|W|W|W| | | | | | |].
 */

#ifndef C_QUEUE_H
#define C_QUEUE_H

/********************************** Includes ***********************************/

/* Standard C library */
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/********************************** Definitions ********************************/

/**
 * Circular queue data structure.
 */
typedef struct {
  size_t size;                     ///< Indicates queue total size.
  bool reserved;                   ///< Reverse exclusive access to queue.
  size_t write_index;              ///< Write pointer that indicates next byte location to be written.
  size_t read_index;               ///< Read pointer that indicates next byte location to be read.
  uint8_t* circular_queue_pointer; ///< Buffer where data is stored.
} circular_queue_t;

/********************************** Prototypes *********************************/

void c_queue_init(circular_queue_t* me, size_t size, uint8_t* circular_queue_pointer);

void c_queue_flash_init(circular_queue_t* me, size_t queue_size);

bool c_queue_reserve(circular_queue_t* me, size_t size);

bool c_queue_copy_and_commit_data(circular_queue_t* me, uint8_t* data, size_t size);

bool c_queue_read(circular_queue_t* me, uint8_t* copy_buffer, size_t size);

bool c_queue_delete(circular_queue_t* me, size_t size);

void c_queue_discard(circular_queue_t* me);

uint8_t c_queue_peek(circular_queue_t* me);

uint8_t* c_queue_message_peek(circular_queue_t* me, size_t msg_size);

size_t c_queue_how_many_msgs(circular_queue_t *me);

size_t c_queue_how_many_msgs_until_size(circular_queue_t *me, size_t size);

size_t c_queue_get_msgs_payload_size(circular_queue_t *me, uint16_t packet_number);

size_t c_queue_peek_by_number(circular_queue_t *me, uint16_t packet_number);

bool c_queue_read_packet_by_number(circular_queue_t *me, uint16_t packet_number, uint8_t *copy_buffer, size_t size);

bool c_queue_delete_by_index(circular_queue_t* me, size_t index ,size_t size);

size_t c_queue_capacity(circular_queue_t* me);

size_t c_queue_free_space(circular_queue_t* me);

size_t c_queue_space_used(circular_queue_t* me);

bool c_queue_is_full(circular_queue_t* me);

bool c_queue_is_empty(circular_queue_t* me);

bool c_queue_is_reserved(circular_queue_t* me);

#endif  /* C_QUEUE_H */
