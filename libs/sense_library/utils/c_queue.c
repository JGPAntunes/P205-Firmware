/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file  c_queue.c
 * @brief Use of the circular queue to save messages. First byte of the data saved 
 * indicates the next message size of the data this way we can save any type of 
 * messages and don´t use special code to know their size. Messages are saved in a 
 * circular queue, were the first byte indicates the next message size. Example:
 * [2|X|X|3|E|E|E|1|A|4|W|W|W|W| | | | | | |].
 */

/********************************** Includes ***********************************/

/* Interface */
#include "c_queue.h"

/* Circular queue utility */
#include "circular_queue.h"

/* Utils */
#include "debug.h"
#include "atomic.h"

/********************************** Private ************************************/



/********************************** Public *************************************/

/**
 * Initializes the circular queue.
 * @param[in] me Circular queue structure pointer.
 * @param[in] size Circular queue size.
 * @param[in] circular_queue_pointer Points to buffer where data is stored.
 */
void c_queue_init(circular_queue_t* me, size_t size, uint8_t* circular_queue_pointer) {
  circular_queue_init(me, size, circular_queue_pointer);
}

/**
 * Initializes the flash circular queue.
 * @param[in] me Circular queue structure pointer.
 * @param[in] queue_size Circular queue size.
 */
void c_queue_flash_init(circular_queue_t* me, size_t queue_size) {
  /* Initializes the circular queue */
  circular_flash_queue_init(me, queue_size);
}

/**
 * Reserves space in the queue, considering the payload size and the byte to store the size
 * @param[in] me Circular queue structure pointer.
 * @param[in] reserve_size Size to reserve.
 * @return True if reserve happened, false otherwise.
 */
bool c_queue_reserve(circular_queue_t* me, size_t reserve_size) {
  /* Try reserve space for the data plus one byte indicating the size */
  return circular_queue_reserve(me, reserve_size + 1);
}

/**
 * Stores the data in the queue.
 * @param[in] me Circular queue structure pointer.
 * @param[in] data Data to save.
 * @param[in] data_size Size of the data.
 * @return True if store happened, false otherwise.
 */
bool c_queue_copy_and_commit_data(circular_queue_t* me, uint8_t* data, size_t data_size) {
  /* Copy the data and one byte indicating their size */
  uint8_t aux_data[data_size + 1];
  aux_data[0] = (uint8_t) data_size;
  memcpy(&aux_data[1], data, data_size);
  
  return circular_queue_copy_and_commit_data(me, aux_data, data_size + 1);
}

/**
 * Gets the size of the next packet.
 * @param[in] me Circular queue structure pointer.
 * @return Size of the next packet.
 */
uint8_t c_queue_peek(circular_queue_t* me) {
#ifdef USE_FLASH
  if(me == flash_gama_get_queue()) {
    return circular_flash_queue_peek_size(me);
  }
#endif

  /* Try read the next data size */
  if(circular_queue_peek(me) != NULL) {
    uint8_t *p;
    p = (uint8_t*)circular_queue_peek(me);
    return *p;
  }
  else {
    return 0;
  }
}

/**
 * Gets the pointer to the next message
 * Note: function not used in Evian nor (if ever) used even by Dandelion.
 * @param[in] me Circular queue structure pointer.
 * @param[in] msg_size Message's size.
 * @return The pointer to the next message.
 */
uint8_t* c_queue_message_peek(circular_queue_t* me, size_t msg_size) {
#ifdef USE_FLASH
  if(me == flash_gama_get_queue()) {
    return NULL;
  }
#endif

  if(msg_size != c_queue_peek(me))
    return NULL;
  else {
    return (uint8_t *) (circular_queue_peek(me) + 1);
  }
}

/**
 * Reads the next packet.
 * @param[in] me Circular queue structure pointer.
 * @param[in] write_buffer Buffer to fill with the message.
 * @param[in] buffer_size Buffer size.
 * @return True if the write was done, false otherwise.
 */
bool c_queue_read(circular_queue_t* me, uint8_t* write_buffer, size_t buffer_size) {
  /* Valid read? */
  if(buffer_size != c_queue_peek(me)) {
    return false;
  }

  /* Read the data and the first byte indicating the size */
  uint8_t aux_data[buffer_size + 1];
  if(circular_queue_read(me, aux_data, buffer_size + 1)) {
    /* Just "return" the data */
    memcpy(write_buffer, &aux_data[1], buffer_size);
    return true;
  } else {
    return false;
  }
}

/**
 * Deletes the next packet.
 * @param[in] me Circular queue structure pointer.
 * @param[in] size Next message (to be deleted) size.
 * @return True if the message was deleted, false otherwise.
 */
bool c_queue_delete(circular_queue_t* me, size_t size) {
  /* Valid delete? */
  if(size != c_queue_peek(me)) {
    return false;
  }
  
  return circular_queue_delete(me, size + 1);
}

/**
 * Discards prior message reservation.
 * @param[in] me Circular queue structure pointer.
 */
void c_queue_discard(circular_queue_t* me) {
  /* Discard reservation */
  circular_queue_discard(me);
}

/**
 * Asks how many messages are inside the queue.
 * Note: not to be used for queues stored at the external flash.
 * @param[in] me Circular queue structure pointer.
 * @return Message's number.
 */
size_t c_queue_how_many_msgs(circular_queue_t* me) {
  size_t number_of_msgs = 0, pointer = 0;

#ifdef USE_FLASH
  if(me == flash_gama_get_queue()) {
    return 0;
  }
#endif

  /* Execute this block atomically to avoid race conditions and hence inconsistencies. */
  atomic(
    pointer = me->read_index;

    if(pointer > me->write_index) {
      while (pointer < me->size) {
        number_of_msgs++;
        pointer += (size_t)me->circular_queue_pointer[pointer] + 1;
      }
      pointer -= me->size;
    }

    while (pointer < me->write_index) {
      number_of_msgs++;
      pointer += (size_t)me->circular_queue_pointer[pointer] + 1;
    }
  );

  return number_of_msgs;
}

/**
 * Asks how many messages are inside the queue that fill the indicated size.
 * I.e., how many messages not yet read can be fetched fitting a given size
 * This is useful for uplink transfer, whose message size is constraint.
 * Note: not to be used by queues stored at external flash
 * @param[in] me Circular queue structure pointer.
 * @param[in] size Defines until what size to search.
 * @return The number of messages.
 */
size_t c_queue_how_many_msgs_until_size(circular_queue_t* me, size_t size) {
  size_t number_of_msgs = 0, pointer = 0, current_size = 0;
  bool count_stop = false;

#ifdef USE_FLASH
  if(me == flash_gama_get_queue()) {
    return 0;
  }
#endif

  if(size == 0) {
    return number_of_msgs;
  }

  /* Execute this block atomically to avoid race conditions and hence inconsistencies. */
  atomic(
    pointer = me->read_index;

    if(pointer > me->write_index) {
      while (pointer < me->size) {
        if(!count_stop && ((current_size + (size_t)me->circular_queue_pointer[pointer]) <= size)) {
          current_size += (size_t)me->circular_queue_pointer[pointer];
          number_of_msgs++;
        }
        else {
          count_stop = true;
        }
        pointer += (size_t)me->circular_queue_pointer[pointer] + 1;
      }
      pointer -= me->size;
    }

    while (pointer < me->write_index) {
      if(!count_stop && ((current_size + (size_t)me->circular_queue_pointer[pointer]) <= size)) {
        current_size += (size_t)me->circular_queue_pointer[pointer];
        number_of_msgs++;
      }
      else {
        count_stop = true;
      }
      pointer += (size_t)me->circular_queue_pointer[pointer] + 1;
    }
  );

  return number_of_msgs;
}

/**
 * Asks the total payload of the specified message's number
 * Note: not to be used by queues stored at external flash
 * @param[in] me Circular queue structure pointer.
 * @param[in] msg_number Message number.
 * @return The payload size.
 */
size_t c_queue_get_msgs_payload_size(circular_queue_t* me, uint16_t msg_number) {
  size_t number_of_packets = 0, pointer = 0, payload_size = 0;

#ifdef USE_FLASH
  if(me == flash_gama_get_queue()) {
    return 0;
  }
#endif

  /* Valid? */
  if(msg_number > c_queue_how_many_msgs(me)) {
    return payload_size;
  }

  /* Execute this block atomically to avoid race conditions and hence inconsistencies. */
  atomic(
    pointer = me->read_index;

    if(pointer > me->write_index) {
      while (pointer < me->size) {
        number_of_packets++;

        if(number_of_packets <= msg_number) {
          payload_size += (size_t)me->circular_queue_pointer[pointer];
        }
        pointer += (size_t)me->circular_queue_pointer[pointer] + 1;
      }
      pointer -= me->size;
    }

    while (pointer < me->write_index) {
      number_of_packets++;

      if(number_of_packets <= msg_number) {
        payload_size += (size_t)me->circular_queue_pointer[pointer];
      }
      pointer += (size_t)me->circular_queue_pointer[pointer] + 1;
    }
  );

  return payload_size;
}

/**
 * Gets the size of the specified packet.
 * Note: not to be used by queues stored at external flash
 * @param[in] me Circular queue structure pointer.
 * @param[in] msg_number Message's index inside the queue.
 * @return Size of the packet.
 */
size_t c_queue_peek_by_number(circular_queue_t* me, uint16_t msg_number) {
  size_t number_of_packets = 0, pointer = 0, peek_return = 0;

#ifdef USE_FLASH
  if(me == flash_gama_get_queue()) {
    return 0;
  }
#endif

  /* Valid? I.e., the index inside the queue must not be bigger than the total amount of messages! */
  if(msg_number > c_queue_how_many_msgs(me)) {
    return peek_return;
  }

  /* Execute this block atomically to avoid race conditions and hence inconsistencies. */
  atomic(
    pointer = me->read_index;

    if(pointer > me->write_index) {
      while (pointer < me->size) {
        number_of_packets++;

        if(number_of_packets == msg_number) {
          peek_return = (size_t)me->circular_queue_pointer[pointer];
        }
        pointer += (size_t)me->circular_queue_pointer[pointer] + 1;
      }
      pointer -= me->size;
    }

    while (pointer < me->write_index) {
      number_of_packets++;

      if(number_of_packets == msg_number) {
        peek_return = (size_t)me->circular_queue_pointer[pointer];
      }
      pointer += (size_t)me->circular_queue_pointer[pointer] + 1;
    }
  );
  
  return peek_return;
}

/**
 * Gets the pointer of the specified packet.
 * Note: not to be used by queues stored at external flash
 * @param[in] me Circular queue structure pointer.
 * @param[in] msg_number Message's index inside the queue.
 * @return Pointer to the packet.
 */
size_t c_queue_get_s_position_by_index(circular_queue_t* me, uint16_t msg_number) {
  size_t current_msg_number = 0, pointer = 0, peek_return = 0;

#ifdef USE_FLASH
  if(me == flash_gama_get_queue()) {
    return 0;
  }
#endif

  /* Valid? */
  if(msg_number >= c_queue_how_many_msgs(me)) {
    return peek_return;
  }

  /* Execute this block atomically to avoid race conditions and hence inconsistencies. */
  atomic(
    pointer = me->read_index;

    if(pointer > me->write_index) {
      while (pointer < me->size) {
        current_msg_number++;

        if(current_msg_number == msg_number) {
          peek_return = pointer;
          break;
        }
        pointer += (size_t)me->circular_queue_pointer[pointer] + 1;
      }
      pointer -= me->size;
    }

    while (pointer < me->write_index) {
      current_msg_number++;

      if(current_msg_number == msg_number) {
        peek_return= pointer;
        break;
      }
      pointer += (size_t)me->circular_queue_pointer[pointer] + 1;
    }
  );

  return peek_return;
}

/**
 * Reads the specified packet by packet number.
 * Note: not to be used by queues stored at external flash
 * @param[in] me Circular queue structure pointer.
 * @param[in] packet_number Message's index inside the queue.
 * @param[in] write_buffer Buffer to fill with the message.
 * @param[in] buffer_size Buffer size.
 * @return True if the write was done, false otherwise.
 */
bool c_queue_read_packet_by_number(circular_queue_t* me, uint16_t packet_number, uint8_t* copy_buffer, size_t size) {
  size_t number_of_packets = 0, pointer = 0;
  bool bool_return = false;

#ifdef USE_FLASH
  if(me == flash_gama_get_queue()) {
    return bool_return;
  }
#endif

  /* Valid read? */
  if(size != c_queue_peek_by_number(me, packet_number)) {
    return bool_return;
  }

  /* Execute this block atomically to avoid race conditions and hence inconsistencies. */
  atomic(
    pointer = me->read_index;

    if(pointer > me->write_index) {
      while (pointer < me->size) {
        number_of_packets++;

        if(number_of_packets == packet_number) {
          if((pointer + size) >= me->size) {
            /* Copy not sequentially */
            memcpy(copy_buffer, &me->circular_queue_pointer[pointer + 1], me->size - (pointer + 1));
            memcpy(&copy_buffer[me->size - (pointer + 1)], me->circular_queue_pointer, size - (me->size - (pointer + 1)));
          }
          else {
            /* Copy sequentially */
            memcpy(copy_buffer, &me->circular_queue_pointer[pointer + 1], size);
          }
          bool_return = true;
        }
        /* Update pointer to the next message */
        pointer += (size_t)me->circular_queue_pointer[pointer] + 1;
      }
      /* Pointer > me-> size, update to the new message begin */
      pointer -= me->size;
    }

    while (pointer < me->write_index) {
      number_of_packets++;

      if(number_of_packets == packet_number) {
        /* Copy sequentially */
        memcpy(copy_buffer, &me->circular_queue_pointer[pointer + 1], size);
        bool_return = true;
      }
      /* Update pointer to the next message */
      pointer += (size_t)me->circular_queue_pointer[pointer] + 1;
    }
  );

  return bool_return;
}

/**
 * Deletes the specified packet from the queue.
 * @param[in] me Circular queue structure pointer.
 * @param[in] packet_number Message's index inside the queue.
 * @param[in] size Next message (to be deleted) size.
 * @return True if the message was deleted, false otherwise.
 */
bool c_queue_delete_by_index(circular_queue_t* me, size_t index, size_t size) {
  /* if the index is the very first one */
  if(index == 1) {
    return c_queue_delete(me, size);
  }
  /* if the index is the last one */
  size_t l_index = c_queue_how_many_msgs(me);
  if(l_index == index) {
    /* Delete last one */
    return circular_queue_delete_start(me, size + 1);
  }
  else {
    /* Delete in the middle */
    size_t position = c_queue_get_s_position_by_index(me,index);
    return circular_queue_delete_middle(me, position, (size + 1));
  }
}

/**
 * Asks about queue capacity.
 * @param[in] me Circular queue structure pointer.
 * @return Queue capacity.
 */
size_t c_queue_capacity(circular_queue_t* me) {
  return circular_queue_capacity(me);
}

/**
 * Asks about current queue free space.
 * @param[in] me Circular queue structure pointer.
 * @return Queue free space.
 */
size_t c_queue_free_space(circular_queue_t* me) {
  return circular_queue_free_space(me);
}

/**
 * Asks about current queue used space.
 * @param[in] me Circular queue structure pointer.
 * @return Queue used space.
 */
size_t c_queue_space_used(circular_queue_t* me) {
  return circular_queue_space_used(me);
}

/**
 * Asks if queue is full.
 * @param[in] me Circular queue structure pointer.
 * @return True if queue is full, false otherwise.
 */
bool c_queue_is_full(circular_queue_t* me) {
  return circular_queue_is_full(me);
}

/**
 * Asks if queue is empty.
 * @param[in] me Circular queue structure pointer.
 * @return True if queue is empty, false otherwise.
 */
bool c_queue_is_empty(circular_queue_t* me) {
  return circular_queue_is_empty(me);
}

/**
 * Asks if queue is reserved.
 * @param[in] me Circular queue structure pointer.
 * @return True if queue is reserved, false otherwise.
 */
bool c_queue_is_reserved(circular_queue_t* me) {
  return me->reserved;
}