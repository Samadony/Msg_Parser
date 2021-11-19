#ifndef PARSER_H
#define PARSER_H

#include "stdint.h"

//	DEFINITIONS
#define TEST	1
#if(TEST == 1)
#define PRIVATE
#else
#define PRIVATE			static
#endif

#define PARSER_START_LEN			1U
#define PARSER_END_LEN				1U
#define PARSER_CMD_LEN				1U
#define PARSER_DATA_LEN_RAW_FOR_ONE_DATA			2U //ie, unit = 2 bytes
#define PARSER_DATA_LOGICAL_LEN				9
#define PARSER_DATA_TOT_LENGTH  	PARSER_DATA_LEN_RAW_FOR_ONE_DATA * PARSER_DATA_LOGICAL_LEN
#define PARSER_MSG_LENGTH			PARSER_START_LEN + PARSER_CMD_LEN + \
									PARSER_DATA_TOT_LENGTH + PARSER_END_LEN


//	TYPES
typedef enum{
	PARSER_OK = 0x00,
	PARSER_ERROR,
}Parser_Status_t;

typedef enum{
	POLLING_START_STATE = 0X00,
	POLLING_CMD_STATE,
	POLLING_LENGTH_STATE,
	PARSING_DATA_STATE,
	POLLING_END_CHARACTER,
	NUM_OF_PARSER_STATES
}Parser_State_t;

typedef enum{
	MSG_NOT_READY = 0x00,
	MSG_READY = 0x01,
	MSG_STATUS_NUM
}Parser_MsgStatus_t;

typedef struct {
	Parser_State_t parser_state;
	Parser_MsgStatus_t parser_msg_status;
	uint8_t parser_expected_raw_data_length;
	uint8_t parser_recieved_raw_data_length;
	uint8_t* parser_msg_buffer_ptr;
}Parser_t;

//template to create a parser object on the client
#define PARSER_CREATE(COMP_ID)	PARSER_CONC_CREATE_HELPER(COMP_ID)
#define PARSER_CONC_CREATE_HELPER(COMP_ID)		\
	PRIVATE uint8_t parser_##COMP_ID##_msg_buffer[PARSER_MSG_LENGTH];\
	PRIVATE Parser_t parser_##COMP_ID = {.parser_msg_buffer_ptr = parser_##COMP_ID##_msg_buffer};\

Parser_Status_t Parser_Init(Parser_t* const self);
void Parse_Message( Parser_t* const self, const uint8_t cpy_character);

#endif //PARSER_H
