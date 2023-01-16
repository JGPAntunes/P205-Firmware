/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file  circular_queue.h
 * @brief Circular buffer code, to save any type of data.
 * One byte of the total size is sacrificed to distinguish between an
 * empty and a full queue. Start pointer, pointer to write;
 * end pointer, pointer to read; [ | | | | | | | | ]: S = E queue is 
 * empty; - S + 1 = E queue is full.
 */

#ifndef CIRCULAR_QUEUE_H
#define CIRCULAR_QUEUE_H

/********************************** Includes ***********************************/

/* Standard C library */
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Circular queue utilities */
#include "c_queue.h"

/********************************** Private ************************************/



/********************************** Public *************************************/

extern circular_queue_t* flash_gama_get_queue(void);

extern size_t flash_manager_write_on_address(uint8_t app_id, uint32_t address, uint8_t* buffer, size_t size);

extern size_t flash_manager_read_c_queue(uint8_t app_id, uint32_t address, uint8_t* buffer, size_t size);

extern size_t flash_manager_read_on_address(uint8_t app_id, uint32_t address, uint8_t* buffer, size_t size);

extern void flash_manager_delete_c_queue(uint8_t app_id, uint32_t address, size_t size, circular_queue_t* cq);

void circular_queue_save_flash_queue_reference(circular_queue_t* me);

void circular_queue_init(circular_queue_t* me, size_t size, uint8_t* circular_queue_pointer);

void circular_flash_queue_init(circular_queue_t* me, size_t size);

void circular_queue_reset_pointers(circular_queue_t* me);

bool circular_queue_reserve(circular_queue_t* me, size_t size);

bool circular_queue_copy_and_commit_data(circular_queue_t* me, uint8_t* data, size_t size);

bool circular_queue_read_and_delete(circular_queue_t* me, uint8_t* copy_buffer, size_t size);

bool circular_queue_read(circular_queue_t* me, uint8_t* copy_buffer, size_t size);

bool circular_queue_delete(circular_queue_t* me, size_t size);

bool circular_queue_delete_start(circular_queue_t* me, size_t size);

bool circular_queue_delete_middle(circular_queue_t* me,size_t position, size_t size);

void circular_queue_discard(circular_queue_t* me);

void* circular_queue_peek(circular_queue_t* me);

size_t circular_flash_queue_peek_size(circular_queue_t* me);

size_t circular_queue_capacity(circular_queue_t* me);

size_t circular_queue_free_space(circular_queue_t* me);

size_t circular_queue_space_used(circular_queue_t* me);

bool circular_queue_is_full(circular_queue_t* me);

bool circular_queue_is_empty(circular_queue_t* me);

#endif  /* CIRCULAR_QUEUE_H */

