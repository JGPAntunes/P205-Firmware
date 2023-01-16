/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file  circular_queue.c
 * @brief Circular buffer code, to save any type of data.
 * One byte of the total size is sacrificed to distinguish between an
 * empty and a full queue. Start pointer => pointer to write;
 * end pointer => pointer to read; [ | | | | | | | | ]: S = E queue is 
 * empty; S + 1 = E queue is full.
 */

/********************************** Includes ***********************************/

/* Interface */
#include "circular_queue.h"

/* Utils */
#include "atomic.h"
#include "debug.h"

/********************************** Private ************************************/
/**
 * Checks whether possible to copy/write sequentially.
 * @param[in] me Circular queue structure pointer.
 * @param[in] data_size Circular queue size.
 * @return True if possible or False otherwise.
 */
bool _circular_queue_can_copy_sequencialy(circular_queue_t* me, size_t data_size) {
  if (me->write_index >= me->read_index) {
    return (circular_queue_capacity(me) - me->write_index) >= data_size;
  } else {
    return (me->read_index - me->write_index - 1) >= data_size;
  }
}

/**
 * Checks whether possible to read sequentially.
 * @param[in] me Circular queue structure pointer.
 * @param[in] data_size Circular queue size.
 * @return True if possible or False otherwise (e.g., when queue is empty).
 */
bool _circular_queue_can_read_sequencialy(circular_queue_t* me, size_t data_size) {
  if (circular_queue_is_empty(me))
    return false;
  if (me->write_index > me->read_index) {
    return (me->write_index - me->read_index) >= data_size;
  } else {
    return (circular_queue_capacity(me) - me->read_index) >= data_size;
  }
}

/********************************** Public *************************************/
/**
 * Initializes the circular queue.
 * @param[in] me Circular queue structure pointer.
 * @param[in] size Circular queue size.
 * @param[in] circular_queue_pointer Points to buffer where data is stored.
 */
void circular_queue_init(circular_queue_t* me, size_t size, uint8_t* circular_queue_pointer) {
  /* Initialize circular_queue structure */
  me->size = size;
  me->reserved = false;
  me->write_index = me->read_index = 0;

  me->circular_queue_pointer = circular_queue_pointer;

  /* Clear data, just because... */
  memset(me->circular_queue_pointer, 0x00, me->size);
}

/**
 * Initializes the flash circular queue.
 * @param[in] me Circular queue structure pointer.
 * @param[in] queue_size Circular queue size.
 */
void circular_flash_queue_init(circular_queue_t* me, size_t queue_size) {
  (void)me;
  (void)queue_size;
#ifdef USE_FLASH
  /* Initialize circular_queue structure */
  me->size = queue_size;
  me->reserved = false;
  // TODO_TODO: no write_index not read_index initialization ????
#endif
}

/**
 * Reset queue pointer (write and read pointers).
 * @param[in] me Circular queue structure pointer.
 */
void circular_queue_reset_pointers(circular_queue_t* me) {
  me->write_index = me->read_index = 0;
}

/**
 * Reserve space in the queue.
 * @param[in] me Circular queue structure pointer.
 * @param[in] size Size to reserve.
 * @return True if reserve happened, false otherwise.
 */
bool circular_queue_reserve(circular_queue_t* me, size_t size) {
  bool b_return = false;
  atomic(
    if (!me->reserved && (circular_queue_free_space(me) >= size)) {
    /* We can reserve the space needed.
     * Set reservation flag active */
    me->reserved = true;
    /* Return to the free space beginning */
    b_return = true; }
  );
  
  return b_return;
}

/**
 * Stores data in the flash queue.
 * This method is no longer public, but once was! Now only called by circular_queue_copy_and_commit_data
 * @param[in] me Circular queue structure pointer.
 * @param[in] data Data to save.
 * @param[in] data_size Size of the data.
 * @return True if store happened, false otherwise.
 */
bool circular_flash_queue_copy_and_commit_data(circular_queue_t* me, uint8_t* data, size_t data_size) {
  (void)me;
  (void)data;
  (void)data_size;

  bool b_return = false;
#ifdef USE_FLASH
  atomic(
    if (me->reserved && (circular_queue_free_space(me) >= data_size)) {
      /* We can copy the data.
       * Copy data */
      if(_circular_queue_can_copy_sequencialy(me, data_size)) {
        /* Copy data sequentially */
        flash_manager_write_on_address(EXT_FLASH_GAMA_QUEUE_APP_ID, me->write_index, data, data_size);
        me->write_index += data_size;

      } else {
        /* Copy data partially */
        //memcpy(p,data, me->data_size - me->p_start);
        flash_manager_write_on_address(EXT_FLASH_GAMA_QUEUE_APP_ID, me->write_index , data, me->size - me->write_index);
        flash_manager_write_on_address(EXT_FLASH_GAMA_QUEUE_APP_ID, 0, &data[me->size - me->write_index], data_size-(me->size - me->write_index));
        //memcpy(me->circular_queue_pointer,&data[me->size - me->p_start],data_size-(me->data_size - me->p_start));
        me->write_index = data_size - (me->size - me->write_index);
      }
      b_return = true;
    }

    /* Free reservation */
    me->reserved = false;
  );
#endif
  return b_return;
}

/**
 * Stores data in the queue.
 * @param[in] me Circular queue structure pointer.
 * @param[in] data Data to save.
 * @param[in] data_size Size of the data.
 * @return True if store happened, false otherwise.
 */
bool circular_queue_copy_and_commit_data(circular_queue_t* me, uint8_t* data, size_t size) {
#ifdef USE_FLASH
  if(me == flash_gama_get_queue()) {
    return circular_flash_queue_copy_and_commit_data(me,data,size);
  }
#endif

  bool b_return = false;

  atomic(
    if(me->reserved && (circular_queue_free_space(me) >= size)) {
      /* We can copy the data */
      void *p = (void*) (me->circular_queue_pointer + me->write_index);
      /* Copy data */
      if(_circular_queue_can_copy_sequencialy(me,size)) {
        /* Copy data sequentially */
        memcpy(p,data,size); 
        me->write_index += size; 
      } else {
        /* Copy data partially */
        memcpy(p, data, me->size - me->write_index); 
        memcpy(me->circular_queue_pointer, &data[me->size - me->write_index], size-(me->size - me->write_index)); 
        me->write_index = size - (me->size - me->write_index);
      }
      b_return = true;
    }
    /* Free reservation */
    me->reserved = false;
  );
  		
  return b_return;
}

/**
 * Reads one packet from flash queue and delete it after that.
 * @param[in] me Circular queue structure pointer.
 * @param[in] copy_buffer Buffer to fill with the message.
 * @param[in] buffer_size Buffer size.
 * @return True if the operation was done, false otherwise.
 */
bool circular_flash_queue_read_and_delete(circular_queue_t* me, uint8_t* copy_buffer, size_t buffer_size) {
  (void)me;
  (void)buffer_size;
  (void)copy_buffer;

  bool b_return = false;
#ifdef USE_FLASH
  atomic(
    if(circular_queue_space_used(me) >= buffer_size) {
      /* We have data to read.
       * Copy data */
      if(_circular_queue_can_read_sequencialy(me,buffer_size)) {
        /* Copy/read data sequentially*/
        flash_manager_read_c_queue(EXT_FLASH_GAMA_QUEUE_APP_ID, me->read_index, copy_buffer, buffer_size);
        //memcpy(copy_buffer,(void*) (me->circular_queue_pointer + me->p_end), size);
        me->read_index += buffer_size;
      } else {
        /* Copy/read data partially */
        flash_manager_read_c_queue(EXT_FLASH_GAMA_QUEUE_APP_ID,  me->read_index, copy_buffer, me->size - me->read_index);
        //memcpy(copy_buffer, (void*)me->circular_queue_pointer + me->p_end, me->size - me->p_end);
        flash_manager_read_c_queue(EXT_FLASH_GAMA_QUEUE_APP_ID, 0, &copy_buffer[me->size - me->read_index], buffer_size-(me->size - me->read_index));
        //memcpy(&copy_buffer[me->size - me->p_end], (void*) me->circular_queue_pointer, size-(me->size - me->p_end));
        me->read_index = buffer_size-(me->size - me->read_index);
      }
    }

    if(circular_queue_is_empty(me)) {
      circular_queue_reset_pointers(me);
    }

    b_return = true;
  );
#endif

  return b_return;
}

/**
 * Reads one packet from queue and delete it after that.
 * @param[in] me Circular queue structure pointer.
 * @param[in] copy_buffer Buffer to fill with the message.
 * @param[in] read_size Buffer size.
 * @return True if the operation was done, false otherwise.
 */
bool circular_queue_read_and_delete(circular_queue_t* me, uint8_t* copy_buffer, size_t read_size) {
#ifdef USE_FLASH
  if(me == flash_gama_get_queue()) {
    return circular_flash_queue_read_and_delete(me,copy_buffer,read_size);
  }
#endif

  bool b_return = false;

  atomic(
    if(circular_queue_space_used(me) >= read_size) {
      /* We have data to read.
       * Copy data */
      if(_circular_queue_can_read_sequencialy(me,read_size)) {
        /* Copy/read data sequentially*/
        memcpy(copy_buffer,(void*) (me->circular_queue_pointer + me->read_index), read_size);
        
        me->read_index += read_size;
      } else {
        /* Copy/read data partially */
        memcpy(copy_buffer, (void*)me->circular_queue_pointer + me->read_index, me->size - me->read_index);
        memcpy(&copy_buffer[me->size - me->read_index], (void*) me->circular_queue_pointer, read_size-(me->size - me->read_index));

        me->read_index = read_size-(me->size - me->read_index);
      }
    }
    if(circular_queue_is_empty(me)) {
      circular_queue_reset_pointers(me);
    }

    b_return = true;
  );

  return b_return;
}

/**
 * Reads the next packet in flash queue.
 * @param[in] me Circular queue structure pointer.
 * @param[in] copy_buffer Buffer to fill with the message.
 * @param[in] read_size Buffer size.
 * @return True if the read was done, false otherwise.
 */
bool circular_flash_queue_read(circular_queue_t* me, uint8_t* copy_buffer, size_t read_size) {
  (void)me;
  (void)read_size;
  (void)copy_buffer;

  bool b_return = false;

#ifdef USE_FLASH
  atomic(
    if(circular_queue_space_used(me) >= read_size) {
      /* We have data to read.
       * Copy data */
      if(_circular_queue_can_read_sequencialy(me,read_size)) {
        /* Copy/read data sequentially*/
        flash_manager_read_c_queue(EXT_FLASH_GAMA_QUEUE_APP_ID, me->read_index, copy_buffer, read_size);
        //memcpy(copy_buffer,(void*) (me->circular_queue_pointer + me->p_end), size);
      } else {
        /* Copy/read data partially */
        flash_manager_read_c_queue(EXT_FLASH_GAMA_QUEUE_APP_ID,  me->read_index, copy_buffer, me->size - me->read_index);
        //memcpy(copy_buffer, (void*)me->circular_queue_pointer + me->p_end, me->size - me->p_end);
        flash_manager_read_c_queue(EXT_FLASH_GAMA_QUEUE_APP_ID, 0, &copy_buffer[me->size - me->read_index], read_size - (me->size - me->read_index));
        //memcpy(&copy_buffer[me->size - me->p_end], (void*) me->circular_queue_pointer, size-(me->size - me->p_end));
      }
    }
    b_return = true;
  );
#endif

  return b_return;
}

/**
 * Reads the next packet.
 * @param[in] me Circular queue structure pointer.
 * @param[in] copy_buffer Buffer to fill with the message.
 * @param[in] read_size Buffer size.
 * @return True if the read was done, false otherwise.
 */
bool circular_queue_read(circular_queue_t* me, uint8_t* copy_buffer, size_t read_size) {
#ifdef USE_FLASH
  if(me == flash_gama_get_queue()) {
    return circular_flash_queue_read(me,copy_buffer,read_size);
  }
#endif

  bool b_return = false;

  atomic(
    if(circular_queue_space_used(me) >= read_size) {
      /* We have data to read.
       * Copy data */
      if(_circular_queue_can_read_sequencialy(me,read_size)) {
        /* Copy/read data sequentially*/
        memcpy(copy_buffer,(void*) (me->circular_queue_pointer + me->read_index), read_size); 
      } else {
        /* Copy/read data partially */
        memcpy(copy_buffer, (void*)me->circular_queue_pointer + me->read_index, me->size - me->read_index);
        memcpy(&copy_buffer[me->size - me->read_index], (void*) me->circular_queue_pointer, read_size-(me->size - me->read_index));
      }
    }
    b_return = true;
  );

  return b_return;
}

/**
 * Deletes the next packet from flash.
 * @param[in] me Circular queue structure pointer.
 * @param[in] size Next message (to be deleted) size.
 * @return True if the message was deleted, false otherwise.
 */
bool circular_flash_queue_delete(circular_queue_t* me, size_t size) {
  (void)me;
  (void)size;

  bool b_return = false;

#ifdef USE_FLASH
  /* Try to clean flash memory */
  atomic(
    if(circular_queue_space_used(me) >= size) {
      /* Confirm we have data */
      if(_circular_queue_can_read_sequencialy(me,size)) {
        flash_manager_delete_c_queue(EXT_FLASH_GAMA_QUEUE_APP_ID, me->read_index, size, me);
        me->read_index += size;
      } else {
        flash_manager_delete_c_queue(EXT_FLASH_GAMA_QUEUE_APP_ID,  me->read_index, me->size - me->read_index, me);
        flash_manager_delete_c_queue(EXT_FLASH_GAMA_QUEUE_APP_ID, 0, size-(me->size - me->read_index), me);
        me->read_index = size-(me->size - me->read_index);
      }
    }

    b_return = true;
  );
#endif

  return b_return;
}

/**
 * Deletes the next packet.
 * @param[in] me Circular queue structure pointer.
 * @param[in] size Next message (to be deleted) size.
 * @return True if the message was deleted, false otherwise.
 */
bool circular_queue_delete(circular_queue_t* me, size_t delete_size) {
#ifdef USE_FLASH
  if(me == flash_gama_get_queue()) {
    return circular_flash_queue_delete(me,delete_size);
  }
#endif

  bool b_return = false;

  atomic(
    if(circular_queue_space_used(me)>=delete_size) {
      /* We have data to read.
       * Just update pointers */
      if(_circular_queue_can_read_sequencialy(me,delete_size)) {
        me->read_index += delete_size;
      } else {
        me->read_index = delete_size -(me->size - me->read_index);
      }
    }
    if(circular_queue_is_empty(me)) {
      circular_queue_reset_pointers(me);
    }
    b_return = true;
  );

  return b_return;
}

/**
 * Deletes the packet from the queue beginning.
 * @param[in] me Circular queue structure pointer.
 * @param[in] size Next message (to be deleted) size.
 * @return True if the message was deleted, false otherwise.
 */
bool circular_queue_delete_start(circular_queue_t* me, size_t size) {
  bool b_return = false;

  atomic(
    if(circular_queue_space_used(me) >= size) {
      /* We have data to read.
       * Just update pointers */
      if(me->write_index >= size) {
        me->write_index -= size;
      } else {
        me->write_index = me->size - (size - me->write_index);
      }
    }
    if(circular_queue_is_empty(me)) {
      circular_queue_reset_pointers(me);
    }
    b_return = true;
  );

  return b_return;
}

/**
 * Deletes the packet in the middle of the queue.
 * @param[in] me Circular queue structure pointer.
 * @param[in] position Position in the queue to be erased.
 * @param[in] size Next message (to be deleted) size.
 * @return True if the message was deleted, false otherwise.
 */
bool circular_queue_delete_middle(circular_queue_t* me, size_t position, size_t size) {
  size_t data_to_copy;

  if(position < me->write_index) {
    data_to_copy = (me->write_index - position) - size;
  } else { 
    data_to_copy = (me->size-(position+size)) + me->write_index;
  }

  size_t data_block = 100;
  size_t copy_size;
  size_t copy_to_p = position;
  size_t copy_from_p;

  if((position + size) >= me->size) {
    copy_from_p = (position + size) - me->size;
  }
  else {
    copy_from_p = position + size;
  }

  void *p_to;
  void *p_from;

  atomic(
    while(data_to_copy > 0) {
      /* Copy limmit to data block*/
      if(data_to_copy >= data_block) {
        copy_size = data_block;
      } else {
        copy_size = data_to_copy;
      }

      /* Copy  cannot excised queue size*/
      if(copy_to_p + copy_size > me->size) {
        copy_size = me->size-copy_to_p;
      }
      if(copy_from_p + copy_size > me->size) {
        copy_size = me->size-copy_from_p;
      }

      /* Setup copy pointers */
      p_to = (void*) (me->circular_queue_pointer + copy_to_p);
      p_from = (void*) (me->circular_queue_pointer + copy_from_p);

      /* Copy */
      memcpy(p_to,p_from ,copy_size);

      /* Update pointers */
      copy_to_p += copy_size;
      copy_from_p += copy_size;

      data_to_copy -= copy_size;

      if(data_to_copy == 0) {
        me->write_index = copy_to_p;
      }

      if(copy_to_p == me->size) {
        copy_to_p = 0;
      }
      if(copy_from_p == me->size) {
        copy_from_p = 0;
      }
    }
  );

  return true;
}

/**
 * Discards prior message reservation.
 * @param[in] me Circular queue structure pointer.
 */
void circular_queue_discard(circular_queue_t* me) {
  atomic(
    if (me->reserved) {
      me->reserved = false;
    }
  );
}

/**
 * Gets the size of the next packet in flash.
 * @param[in] me Circular queue structure pointer.
 * @return Size of the next packet.
 */
size_t circular_flash_queue_peek_size(circular_queue_t* me) {
  (void)me;

#ifdef USE_FLASH
  uint8_t aux[1] = {0x00};
  atomic(
    if (!circular_queue_is_empty(me)) {
      flash_manager_read_on_address(EXT_FLASH_GAMA_QUEUE_APP_ID,  me->read_index, aux, 1);
    }
  );
  return aux[0];
#else
  return 0;
#endif
}

/**
 * Gets the size of the next packet.
 * @param[in] me Circular queue structure pointer.
 * @return Size of the next packet.
 */
void* circular_queue_peek(circular_queue_t* me) {
#ifdef USE_FLASH
  if(me == flash_gama_get_queue()) {
    return NULL;
  }
#endif

  void* p = NULL;
  atomic(
    if (!circular_queue_is_empty(me)) {
      p = &me->circular_queue_pointer[me->read_index];
    }
  );

  return p;
}

/**
 * Asks about queue capacity.
 * @param[in] me Circular queue structure pointer.
 * @return Queue capacity.
 */
size_t circular_queue_capacity(circular_queue_t* me) {
  return me->size - 1;
}

/**
 * Asks about current queue free space.
 * @param[in] me Circular queue structure pointer.
 * @return Queue free space.
 */
size_t circular_queue_free_space(circular_queue_t* me) {
  if (me->write_index >= me->read_index) {
    return circular_queue_capacity(me) - (me->write_index - me->read_index);
  } else {
    return me->read_index - me->write_index - 1;
  }
}

/**
 * Asks about current queue used space.
 * @param[in] me Circular queue structure pointer.
 * @return Queue used space.
 */
size_t circular_queue_space_used(circular_queue_t* me) {
  return circular_queue_capacity(me) - circular_queue_free_space(me);
}

/**
 * Asks if queue is full.
 * @param[in] me Circular queue structure pointer.
 * @return True if queue is full, false otherwise.
 */
bool circular_queue_is_full(circular_queue_t* me) {
  return circular_queue_free_space(me) == 0;
}

/**
 * Asks if queue is empty.
 * @param[in] me Circular queue structure pointer.
 * @return True if queue is empty, false otherwise.
 */
bool circular_queue_is_empty(circular_queue_t* me) {
  return circular_queue_free_space(me) == circular_queue_capacity(me);
}