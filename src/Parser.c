#include "Parser.h"
#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include "string.h"



#define START_CHARACTER		'['
#define END_CHARACTER		']'

inline static bool IsValid_Data(const uint8_t data);
inline static bool IsValid_Cmd(const uint8_t cmd);
inline static uint8_t Convert_ASCII_Numerical_to_Numbers(const uint8_t cpy_character);

static uint8_t Convert_ASCII_Numerical_to_DataNumofBytes(const uint8_t cpy_character)
{
	return (PARSER_DATA_LEN_RAW_FOR_ONE_DATA * (cpy_character - '0'));
}

inline static bool IsValid_Cmd(const uint8_t cmd)
{
	return ((('A' <= cmd) && ('Z' >= cmd)) ? true : false );
}

inline static bool IsValid_Data(const uint8_t data)
{
	bool IsValid = false;

	if(('A' <= data && 'Z' >= data) || ('a' <= data && 'z' >= data) ||
		('0' <= data && '9' >= data))
	{
		IsValid = true;
	}
	return IsValid;
}

inline static void Append_Data(const uint8_t* const character, uint8_t* const buffer_array)
{

}
/*
 * number of instances shall be equal to number of serial interfaces, the access
 * shall be FIFO
 */
PRIVATE Parser_t Parsers;

Parser_Status_t Parser_Init(Parser_t* const self)
{
	Parser_Status_t parser_status = PARSER_ERROR;
	if(NULL != self)
	{
		if(NULL != self->parser_msg_buffer_ptr)
		{
			(void)memset(self->parser_msg_buffer_ptr, 0, (PARSER_MSG_LENGTH * sizeof(self->parser_msg_buffer_ptr[0])));
			self->parser_state = POLLING_START_STATE;
			self->parser_msg_status = MSG_NOT_READY;
			self->parser_expected_raw_data_length = 0;
			self->parser_recieved_raw_data_length = 0;
			parser_status = PARSER_OK;
		}
	}
	return parser_status;
}

void Parse_Message( Parser_t* const self, const uint8_t cpy_character)
{
//add client notification and return pointer to client type (check the mediator/ server - client pattern(i think thats an observer))
	switch(self->parser_state)
	{
		case POLLING_START_STATE:
			self->parser_msg_status = MSG_NOT_READY;
			if(START_CHARACTER == cpy_character)
			{
				self->parser_recieved_raw_data_length = 0;
				self->parser_msg_buffer_ptr[self->parser_recieved_raw_data_length];
				self->parser_recieved_raw_data_length++;
				self->parser_state = POLLING_CMD_STATE;
			}
			break;
		case POLLING_CMD_STATE:
			self->parser_msg_status = MSG_NOT_READY;

			if(IsValid_Cmd(cpy_character) )
			{
				self->parser_recieved_raw_data_length = 0;
				self->parser_state = POLLING_LENGTH_STATE;
			}
			else
			{
				self->parser_state = POLLING_START_STATE;
			}

			break;
		case POLLING_LENGTH_STATE:
			if('0' == cpy_character )
			{
				self->parser_state = POLLING_END_CHARACTER;
				self->parser_expected_raw_data_length = 0;
			}
			else if(('1' <= cpy_character) && ('9' >= cpy_character))
			{
				self->parser_state = PARSING_DATA_STATE;
				self->parser_expected_raw_data_length =
						Convert_ASCII_Numerical_to_DataNumofBytes(cpy_character);
			}
			else if(START_CHARACTER == cpy_character)
			{
				self->parser_state = POLLING_CMD_STATE;
			}
			else//other invalid length
			{
				self->parser_state = POLLING_START_STATE;
			}

			break;
		case PARSING_DATA_STATE:
			if(IsValid_Data(cpy_character))
			{
				if(self->parser_recieved_raw_data_length <
						self->parser_expected_raw_data_length)
				{
					//append the data to the data buffer
					self->parser_msg_buffer_ptr[self->parser_recieved_raw_data_length];
					self->parser_recieved_raw_data_length++;
				}
				else //data was filled, transfer to Polling END charcter to validate the message
				{

					self->parser_state = POLLING_END_CHARACTER;
				}
			}
			else if(START_CHARACTER == cpy_character)
			{
				self->parser_state = POLLING_CMD_STATE;
			}
			else//any other invalid data
			{
				self->parser_state = POLLING_START_STATE;
			}
			break;
		case POLLING_END_CHARACTER:
			if(END_CHARACTER == cpy_character)
			{
				//notify the user that there a message
				self->parser_state = POLLING_START_STATE;
			}
			else if(START_CHARACTER == cpy_character)
			{
				//discard the data
				self->parser_state = POLLING_CMD_STATE;
			}
			else //any not valid character
			{
				//discard the data
				self->parser_state = POLLING_START_STATE;
			}
			break;
		case NUM_OF_PARSER_STATES:
		default:
			//TODO:" Report Error
			break;
	}
}
