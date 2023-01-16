/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file  simcom.c
 * @brief SIMCom drivers and module interaction (includes internal buffers, etc.).
 */

/* Interface */
#include "sense_library/sensors/simcom.h"

/********************************** Private ***********************************/

/**
 * Callback to send new string through UART to SIMCom module.
 */
static uart_send_string_callback_def _uart_send_string_callback = NULL;

/**
 * It is a circular buffer that will be filled with the
 * messages received from the UART module.
 */
static uint8_t _rx_uart_buffer[RX_UART_SIMCOM_BUFFER_SIZE];

/**
 * The rx buffer write pointer.
 */
static uint16_t _write_pointer = 0;

/**
 * The rx buffer read pointer.
 */
static uint16_t _read_pointer = 0;

/**
 * Indicates the number of responses buffered.
 */
static uint16_t _how_many_responses = 0;

/**
 * Indicates the beggining of a new transaction.
 */
static uint16_t _beggining_last_command_pointer = 0;

/**
 * Indicates if the SIMCOM is ready to receive a new command.
 */
static bool _ready_to_rx_new_command = true;

/**
 * When new line char received record that in this variable.
 * Just used for print purposes.
 */
static bool _new_line_char_received = false;

/********************************** Public ***********************************/

/**
 * SIMCom initial setup.
 * @param[in] uart_send_string_callback Function callback in that will be used to transmit new strings to module.
 */
void simcom_setup(uart_send_string_callback_def uart_send_string_callback) {
  _uart_send_string_callback = uart_send_string_callback;
}

/**
 * Initializes all the internal variables.
 */
void simcom_reset_variables(void) {
  memset(_rx_uart_buffer, '\0', RX_UART_SIMCOM_BUFFER_SIZE);
  _ready_to_rx_new_command = true;
  _how_many_responses = 0;
  _write_pointer = 0;
  _read_pointer = 0;
  _beggining_last_command_pointer = 0;
}

/**
 * Simply returns a boolean that indicates if the module is ready to receive
 * more commands.
 * return SIMCom status according with its availability to receive more commands.
 */
bool simcom_ready_to_rx_new_command(void) {
  return _ready_to_rx_new_command;
}

/**
 * Simply put the command string in the UART tx bus to be sent.
 * @param[in] command
 * @param[in] command_size
 * @param[in] force Tells if we want to force the transmission. It could be relevant if module stays unavailable to receive
 * more commands for a long time.
 * @return    True if command was sent, false otherwise.
 */
bool simcom_send_command(char *command, uint16_t command_size, bool force) {
  if (_ready_to_rx_new_command || force) {
    _ready_to_rx_new_command = false;
    _beggining_last_command_pointer = _write_pointer;

    _uart_send_string_callback(command, command_size);

    return true;
  } else {
    return false;
  }
}

/**
 * Simply returns the number if responses to be read.
 * @return The number of respondes to be read (delimited with new lines).
 */
uint16_t simcom_how_many_new_responses(void) {
  return _how_many_responses;
}

/**
 * Returns the last responses.
 * @param[in] responses Pointer to the responses data structure.
 */
void simcom_read_responses(struct simcom_responses *responses) {
  memset(responses->responses, '\0', RX_UART_SIMCOM_BUFFER_SIZE);
  responses->responses_size = 0;
  responses->responses_number = 0;

  if (!_how_many_responses)
    return;

  uint16_t responses_write_pointer = 0;

  /* Copy all the completed received responses. Remember that _rx_uart_buffer
	 * is a circular buffer! */
  while (_how_many_responses) {
    responses->responses[responses_write_pointer] = (char)_rx_uart_buffer[_read_pointer];

    if ((char)_rx_uart_buffer[_read_pointer] != '\r') {
      /* SIMCom module usually sends to consecutive new line chars. Avoid that when printing out */
      if ((char)_rx_uart_buffer[_read_pointer] == '\n' && _new_line_char_received) {
        /* Skip this print */
        ;
      } else {
        if ((char)_rx_uart_buffer[_read_pointer] == '\n') {
          _new_line_char_received = true;
        } else {
          _new_line_char_received = false;
        }

        char test[2];
        test[0] = (char)_rx_uart_buffer[_read_pointer];
        test[1] = '\0';
        debug_print_char(DEBUG_LEVEL_1, test[0]);
      }
    }

    if ((char)responses->responses[responses_write_pointer] == '\r') {
      responses->responses_number++;
      _how_many_responses--;
    }

    if (_read_pointer == (RX_UART_SIMCOM_BUFFER_SIZE - 1)) {
      _read_pointer = 0;
    } else {
      _read_pointer++;
    }

    if (responses_write_pointer < (RX_UART_SIMCOM_BUFFER_SIZE - 1)) {
      responses_write_pointer++;
    }
  }

  responses->responses_size = responses_write_pointer;

  return;
}

/**
 * This is an utility function. It is typically used when we want to see the entire
 * last transaction. So that this function doesn't replace the usage of the
 * simcom_read_responses function. It returns the last responses that are from the
 * last command. These way we can get all the last transaction responses.
 * @param[in] last_command The last command sent.
 * @param[in] responses The pointer to the responses data structure.
 */
void simcom_get_last_completed_transaction(char *last_command,
    struct simcom_responses *responses) {
  memset(responses->responses, '\0', RX_UART_SIMCOM_BUFFER_SIZE);
  responses->responses_size = 0;
  responses->responses_number = 0;

  /* Temporary copy of the uart rx buffer. Remember that it's a circular buffer! */
  char temp_rx_uart_buffer[RX_UART_SIMCOM_BUFFER_SIZE];
  memset(temp_rx_uart_buffer, '\0', RX_UART_SIMCOM_BUFFER_SIZE);
  memcpy(temp_rx_uart_buffer,
      (char *)&_rx_uart_buffer[_beggining_last_command_pointer],
      (RX_UART_SIMCOM_BUFFER_SIZE - _beggining_last_command_pointer));
  memcpy(
      &temp_rx_uart_buffer[(RX_UART_SIMCOM_BUFFER_SIZE - _beggining_last_command_pointer)],
      (char *)_rx_uart_buffer, _beggining_last_command_pointer);

  /* Because there is a possibility of some other chars before my actual command. Locate the beginning of the last command */
  int temp_beggining_last_command_pointer = utils_locate_subtring(
      temp_rx_uart_buffer, last_command);
  if (temp_beggining_last_command_pointer == -1) {
    return;
  }

  memcpy(responses->responses,
      &temp_rx_uart_buffer[temp_beggining_last_command_pointer],
      (RX_UART_SIMCOM_BUFFER_SIZE - temp_beggining_last_command_pointer));

  responses->responses_size = strlen(responses->responses);

  uint16_t i;
  /* Update the responses number variable */
  for (i = 0; i < responses->responses_size; i++) {
    if (responses->responses[i] == '\r') {
      responses->responses_number++;
    }
  }

  return;
}

/**
 * Increments the number of chars received, which corresponds to the
 * response size
 * param[in] new_char Corresponds to the new UART char received.
 */
void simcom_rx_new_char(uint8_t new_char) {
  _rx_uart_buffer[_write_pointer] = new_char;

  /* Ready to send a new command? OK received? */
  if ((char)new_char == 'K') {
    if (_write_pointer != 0) {
      if ((char)_rx_uart_buffer[_write_pointer - 1] == 'O') {
        _ready_to_rx_new_command = true;
      }
    }
    /* My 'K' char is on the first position and remember, I'm a circular buffer man! */
    else {
      if ((char)_rx_uart_buffer[RX_UART_SIMCOM_BUFFER_SIZE - 1] == 'O') {
        _ready_to_rx_new_command = true;
      }
    }
  }

  if ((char)new_char == '\r') {
    _how_many_responses++;
  }

  /* Updates the _write_pointer */
  if (_write_pointer == (RX_UART_SIMCOM_BUFFER_SIZE - 1)) {
    _write_pointer = 0;
  } else {
    _write_pointer++;
  }
}