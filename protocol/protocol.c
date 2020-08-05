#include <assert.h>
#include <stddef.h>
#include "protocol.h"
#include "crc16.h"



static void reset_rx(protocol_handler_t* protocol);
static protocol_err_t push_byte_to_buffer(protocol_handler_t* protocol, uint8_t byte);
#ifndef USE_CAN
static uint8_t write_enc_byte(protocol_handler_t* protocol, uint8_t byte);
#endif



static void reset_rx(protocol_handler_t* protocol)
{
  assert(protocol != NULL);
   
  //set default state
  protocol->recv_state = RECV_STATE_DEFAULT;
  protocol->state = PROTOCOL_STATE_NORMAL;
  protocol->size = 0;
  protocol->w_size = 0;
  protocol->crc = CRC_INIT;
  protocol->pr_byte = 0x00;
}

protocol_err_t protocol_init(protocol_handler_t* protocol, const protocol_descriptor_t* desc)
{
  assert(protocol != NULL);
  assert(desc != NULL);
  assert(desc->buff != NULL);
  assert(desc->recv_message != NULL);
  assert(desc->write_byte != NULL);

  protocol->desc = desc;
  reset_rx(protocol);
        
  return PROTOCOL_NO_ERR;
}

static protocol_err_t push_byte_to_buffer(protocol_handler_t* protocol, uint8_t byte)
{
  protocol_err_t error = PROTOCOL_NO_ERR;
        
  assert(protocol != NULL);

  if (protocol->size >= protocol->desc->buff_size) {
    error = PROTOCOL_ERR_BUFF_OVERFLOW;
    reset_rx(protocol);
  } else {
    protocol->desc->buff[(protocol->size)++] = byte;
    protocol->crc = step_crc16_ccitt(byte, protocol->crc);
    protocol->state = PROTOCOL_STATE_NORMAL;
  }

  return error;
}

protocol_err_t protocol_read_byte(protocol_handler_t* protocol, uint8_t byte)
{
  
  protocol_err_t err = PROTOCOL_NO_ERR;
        
  assert(protocol != NULL);
  
#ifdef USE_CAN  
  err = push_byte_to_buffer(protocol,byte);
#else   
  
  //TODO: if a single character delimiter came
  
  //remove duplicate character delimiter
  if ((byte == PROTOCOL_SPECIAL_BYTE_DELIMITER) && \
     (protocol->pr_byte == PROTOCOL_SPECIAL_BYTE_DELIMITER)) {
    return err;
  }
  protocol->pr_byte = byte;
  
  //state machine
  switch (protocol->recv_state) {
    
    //wait delimiter byte (0x7E)
    case RECV_STATE_DEFAULT:
      if (byte == PROTOCOL_SPECIAL_BYTE_DELIMITER)
          protocol->recv_state = RECV_STATE_HEAD;
      break;
  
    //wait packet length byte 
    case RECV_STATE_HEAD:
      protocol->w_size = byte;
      protocol->recv_state = RECV_STATE_DATA;
      break;
    
    //wait data byte
    case RECV_STATE_DATA:
      err = push_byte_to_buffer(protocol,byte);
      if (--(protocol->w_size) == 0) {
        protocol->recv_state = RECV_STATE_CRC;
        protocol->w_size = CRC_LEN;
      }
      break;
      
    //wait crc byte  
    case RECV_STATE_CRC:
      //chech_crc
      if ((uint8_t)((protocol->crc)>>((--protocol->w_size)*8)) != byte) {
        reset_rx(protocol);
        err = PROTOCOL_ERR_CRC;
        break;
      }
      
      //call recv_message
      if (protocol->w_size == 0) {
          protocol->desc->recv_message(protocol->desc->buff, protocol->size);
          reset_rx(protocol);
      }

      break;
      
    default:
      reset_rx(protocol);
      break;
  
  }
#endif
  return err;
}

protocol_err_t protocol_read(protocol_handler_t* protocol, uint8_t* data, uint8_t size)
{
  protocol_err_t err = PROTOCOL_NO_ERR;
  

  for (uint8_t i = 0; i<size; i++) {
    
    err = protocol_read_byte(protocol,data[i]);

    if (err != PROTOCOL_NO_ERR)
      break;
  }
  
#ifdef USE_CAN
  if (err == PROTOCOL_NO_ERR)
    protocol->desc->recv_message(protocol->desc->buff, protocol->size);
  
  reset_rx(protocol);
#endif  

  return err;
}

#ifndef USE_CAN
static uint8_t write_enc_byte(protocol_handler_t* protocol, uint8_t byte)
{
  assert(protocol != NULL);

  if (byte == PROTOCOL_SPECIAL_BYTE_DELIMITER) {
    if (protocol->desc->write_byte(PROTOCOL_SPECIAL_BYTE_DELIMITER) == 0)
      return 0;
  }
  
  if (protocol->desc->write_byte(byte) == 0)
    return 0;
        
  return 1;
}
#endif

protocol_err_t protocol_send_message(protocol_handler_t* protocol, uint8_t *data, uint8_t size)
{    
  assert(data != NULL);
  assert(protocol != NULL);
  
    //check max len
  if (size > PROTOCOL_MAX_PAYLOAD_LEN)
    return PROTOCOL_ERR_BUFF_OVERFLOW;
  
#ifdef USE_CAN
  for (uint8_t i = 0; i<size; i++) {
    if (protocol->desc->write_byte(data[i]) == 0)
      return PROTOCOL_ERR_BUFF_OVERFLOW;
  }
#else  
  uint8_t byte;
  uint8_t crc_buf[2];
  uint16_t crc = CRC_INIT;
    
  //head
  if (protocol->desc->write_byte(PROTOCOL_SPECIAL_BYTE_DELIMITER) == 0)
    return PROTOCOL_ERR_BUFF_OVERFLOW;

  //len
  if (write_enc_byte(protocol, size) == 0)
    return PROTOCOL_ERR_BUFF_OVERFLOW;
  
  
  //payload
  for (uint8_t i = 0; i < size; i++) {
    byte = data[i];
    crc = step_crc16_ccitt(byte, crc);
  
    if (write_enc_byte(protocol, byte) == 0)
      return PROTOCOL_ERR_BUFF_OVERFLOW;
  }

  //crc
  crc_buf[0] = (uint8_t)(crc >> 8);
  crc_buf[1] = (uint8_t)(crc & 0xFF);

  for (uint8_t i = 0; i < 2; i++) {
    byte = crc_buf[i];
   
    if (write_enc_byte(protocol, byte) == 0)
      return PROTOCOL_ERR_BUFF_OVERFLOW;
  }
#endif
  return PROTOCOL_NO_ERR;
}
