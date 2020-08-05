#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <stdint.h>

#ifdef USE_CAN
#define DELIMITER_LEN (0)
#define HEAD_LEN (0)
#define CRC_LEN (0)
#define PROTOCOL_MAX_PAYLOAD_LEN (8)
#else
#define DELIMITER_LEN (1)
#define HEAD_LEN (1)
#define CRC_LEN (2)
#define PROTOCOL_MAX_PAYLOAD_LEN (8)
#endif

#define PROTOCOL_MAX_PKT_LEN (DELIMITER_LEN+\
                              HEAD_LEN+\
                              (PROTOCOL_MAX_PAYLOAD_LEN*2)+\
                              CRC_LEN)

// use to identify the start of a frame 
#define PROTOCOL_SPECIAL_BYTE_DELIMITER (0x7E)

typedef void    (*recv_message_t)(uint8_t* data, uint8_t size);
typedef uint8_t (*write_byte_t)(uint8_t byte);

typedef enum {
  PROTOCOL_STATE_NORMAL = 0x00,
  PROTOCOL_STATE_ESC
} protocol_state_t;

typedef enum {
  RECV_STATE_DEFAULT = 0x00,
  RECV_STATE_HEAD,
  RECV_STATE_DATA,
  RECV_STATE_CRC
} recv_state_t;

typedef enum {
  PROTOCOL_NO_ERR = 0x00,
  PROTOCOL_ERR_BUFF_OVERFLOW,
  PROTOCOL_ERR_UNKNOWN_ESCAPED_BYTE,
  PROTOCOL_ERR_CRC
} protocol_err_t;

typedef struct {
  uint8_t* buff; // receive buffer pointer
  uint8_t  buff_size; // receive buffer size

  recv_message_t recv_message; // pointer to the function to be called when the packet was received successfully
  write_byte_t write_byte; // a pointer to the function called when the byte is sent
} protocol_descriptor_t;

typedef struct {
  protocol_state_t state;  //TODO: 
  recv_state_t recv_state; // state machine receiver
  uint8_t w_size; // expected number of bytes
  uint8_t size; // package length
  uint16_t crc; // packet checksum
  uint8_t pr_byte; // previous packet byte
  const protocol_descriptor_t *desc;
} protocol_handler_t;

/*
initialization of the head structure, setting default values of states
*/
protocol_err_t protocol_init(protocol_handler_t* protocol, const protocol_descriptor_t* desc);

/*
called every time one byte needs to be transferred to the protocol
*/
protocol_err_t protocol_read_byte(protocol_handler_t* protocol, uint8_t byte);

/*
a function protocol_read_byte called in a loop (size)
*/
protocol_err_t protocol_read(protocol_handler_t* protocol, uint8_t* data, uint8_t size);

/*
message sending function
*/
protocol_err_t protocol_send_message(protocol_handler_t* protocol, uint8_t* data, uint8_t size);

#endif /* __PROTOCOL_H__ */