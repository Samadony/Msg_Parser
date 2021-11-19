
#include "unity.h"
#include "Parser.h"
#include "MK20DX256.h"
#include "string.h"

#define TEST_ID	TestParser

PARSER_CREATE(TEST_ID);

void setUp(void){
	memset(parser_TestParser.parser_msg_buffer_ptr, 0xFF, PARSER_MSG_LENGTH);
	parser_TestParser.parser_state = NUM_OF_PARSER_STATES;
}
/*
 * Goal: Test the initialization of the Parser object
 * Arrange:
 * Act: Call initialization
 * Assert: Init status shall be okay
 * Assert: Buffer shall be initialized as expected
 *
 * Act: init with input parameter NULL
 * Assert: returned status shall be parser error
 *
 */
void test_The_Init_of_the_parser_object(void)
{
	Parser_Status_t parser_status;
	parser_status = Parser_Init(&parser_TestParser);
	TEST_ASSERT_EQUAL(PARSER_OK, parser_status);
	TEST_ASSERT_EQUAL(0, parser_TestParser.parser_msg_buffer_ptr[PARSER_MSG_LENGTH - 1]);
	TEST_ASSERT_EQUAL(0, parser_TestParser.parser_msg_buffer_ptr[0]);
	TEST_ASSERT_EQUAL(POLLING_START_STATE, parser_TestParser.parser_state);
	TEST_ASSERT_EQUAL(MSG_NOT_READY, parser_TestParser.parser_msg_status);

	parser_status = Parser_Init(NULL);
	TEST_ASSERT_EQUAL(PARSER_ERROR, parser_status);
}

/*
 * Goal: Test state machine transition
 * Strategy: test each trigger including remaining in same state trigger
 *
 *
 * 		State: POLLING_START_STATE
 *
 * Arrange: set parser.parser_state = POLLING_START_STATE
 * Act: call parser with in char = '['
 * Assert: parser.parser_state = POLLING_CMD_STATE
 *
 * Arrange: set parser.parser_state = POLLING_START_STATE
 * Act: call parser with in char != '['
 * Assert: parser.parser_state = POLLING_START_STATE
 *
 *
 *		 State: POLLING_CMD_STATE
 *
 * Arrange: parser.parser_state = POLLING_CMD_STATE
 * Act: call parser with in parameter char = 'A - Z'
 * Assert: state = POLLING_LENGTH_STATE
 *
 * Arrange: parser.parser_state = POLLING_CMD_STATE
 * Act: call parser with in parameter char != 'A - Z'
 * Assert: state = POLLING_START_STATE
 *
 *
 *  	 State: POLLING_LENGTH_STATE
 *
 * Arrange: parser.parser_state = POLLING_LENGTH_STATE
 * Act: call parser with in parameter char = '1 - 9'
 * Assert: state = PARSING_DATA_STATE
 *
 * Arrange: parser.parser_state = POLLING_LENGTH_STATE
 * Act: call parser with in parameter char = '0'
 * Assert: state = POLLING_END_CHARACTER
 *
 * Arrange: parser.parser_state = POLLING_LENGTH_STATE
 * Act: call parser with in parameter char = '['
 * Assert: state = POLLING_CMD_STATE
 *
 * Arrange: parser.parser_state = POLLING_LENGTH_STATE
 * Act: call parser with in parameter char != '0 - 9'
 * Assert: state = POLLING_START_STATE
 *
 *
 *   	 State: PARSING_DATA_STATE
 *
 * Arrange: parser.parser_state = PARSING_DATA_STATE
 * Act: call parser with in parameter char = ('0 - 9' || char = 'A - Z') && (length == Len)
 * Assert: state = POLLING_END_CHARACTER
 *
 * Arrange: parser.parser_state = PARSING_DATA_STATE
 * Act: call parser with in parameter char != ('0 - 9' || char = 'A - Z') && (length == Len)
 * Assert: state = POLLING_START_STATE
 *
 * Arrange: parser.parser_state = PARSING_DATA_STATE
 * Act: call parser with in parameter char = '['
 * Assert: state = POLLING_CMD_STATE
 *
 * Arrange: parser.parser_state = PARSING_DATA_STATE
 * Act: call parser with in parameter char = ('A - Z' || '0- 9')
 * Assert: state = PARSING_DATA_STATE
 *
 * Arrange: parser.parser_state = PARSING_DATA_STATE
 * Act: call parser with in parameter char = length is odd not even"pairs"
 * Assert: state = PARSING_DATA_STATE
 *
 *
 *   	 State: POLLING_END_CHARACTER
 *
 * Arrange: parser.parser_state = POLLING_END_CHARACTER
 * Act: call parser with in parameter char = ']'
 * Assert: state = POLLING_START_STATE
 *
 * Arrange: parser.parser_state = POLLING_END_CHARACTER
 * Act: call parser with in parameter char = '['
 * Assert: state = POLLING_CMD_STATE
 *
 * Arrange: parser.parser_state = POLLING_END_CHARACTER
 * Act: call parser with in parameter char != (('0 - 9' || char = 'A - Z') && (length == Len))
 * Assert: state = POLLING_START_STATE
 *
 *
 */
void test_the_parser_states(void)
{
	uint8_t Albhabet[26] = {'A','B','C','D','E','F','G','H','I','J','K','L',
	'M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};
	uint8_t start_char = '[';
	uint8_t end_char = ']';
	uint8_t num_0_to_9[10] = {'0','1','2','3','4','5','6','7','8','9'};
	uint8_t random_char = '/'; //not '[' || 'A-to-Z' || '0-to-9' || ]

	uint8_t iterator;

	//	State: POLLING_START_STATE
	start_char = '['; // start
	parser_TestParser.parser_state = POLLING_START_STATE;
	Parse_Message(&parser_TestParser, start_char);
	TEST_ASSERT_EQUAL(POLLING_CMD_STATE, parser_TestParser.parser_state);

	random_char = '-'; // any but '['
	parser_TestParser.parser_state = POLLING_START_STATE;
	Parse_Message(&parser_TestParser, random_char);
	TEST_ASSERT_EQUAL(POLLING_START_STATE, parser_TestParser.parser_state);

	//	State: POLLING_CMD_STATE
	parser_TestParser.parser_state = POLLING_CMD_STATE;
	Parse_Message(&parser_TestParser,'C');
	TEST_ASSERT_EQUAL(POLLING_LENGTH_STATE, parser_TestParser.parser_state);
	for(iterator = 0; iterator < 26 ; iterator++)
	{	//A to Z
		parser_TestParser.parser_state = POLLING_CMD_STATE;
		Parse_Message(&parser_TestParser, Albhabet[iterator]);
		TEST_ASSERT_EQUAL(POLLING_LENGTH_STATE, parser_TestParser.parser_state);
	}
	parser_TestParser.parser_state = POLLING_CMD_STATE;
	Parse_Message(&parser_TestParser, random_char);
	TEST_ASSERT_EQUAL(POLLING_START_STATE, parser_TestParser.parser_state);

	//	State: POLLING_LENGTH_STATE

	for(iterator = 1 ; iterator <= 9 ; iterator++)
	{
		parser_TestParser.parser_state = POLLING_LENGTH_STATE;
		Parse_Message(&parser_TestParser, num_0_to_9[iterator]);
		TEST_ASSERT_EQUAL(PARSING_DATA_STATE, parser_TestParser.parser_state);
	}

	parser_TestParser.parser_state = POLLING_LENGTH_STATE;
	Parse_Message(&parser_TestParser, random_char);
	TEST_ASSERT_EQUAL(POLLING_START_STATE, parser_TestParser.parser_state);

	parser_TestParser.parser_state = POLLING_LENGTH_STATE;
	Parse_Message(&parser_TestParser, num_0_to_9[0]); // '0'
	TEST_ASSERT_EQUAL(POLLING_END_CHARACTER, parser_TestParser.parser_state);

	parser_TestParser.parser_state = POLLING_LENGTH_STATE;
	Parse_Message(&parser_TestParser, start_char); // '['
	TEST_ASSERT_EQUAL(POLLING_CMD_STATE, parser_TestParser.parser_state);

	//	State: PARSING_DATA_STATE
//	parser.parser_state = POLLING_LENGTH_STATE;
//	Parse_Message(&parser, start_char); // '['
//	TEST_ASSERT_EQUAL(POLLING_CMD_STATE, parser.parser_state);
//
//	for(iterator = 0; iterator < PARSER_DATA_TOT_LENGTH ; iterator++)
//	{
//		 parser.parser_state = PARSING_DATA_STATE;
//		 Parse_Message(&parser, num_0_to_9[10]);
//		 //char = ('0 - 9' || char = 'A - Z') && (length == Len);
//		TEST_ASSERT_EQUAL(PARSING_DATA_STATE, parser.parser_state);
//	}
//	TEST_ASSERT_EQUAL(POLLING_END_CHARACTER, parser.parser_state);
//
	 parser_TestParser.parser_state = PARSING_DATA_STATE;
	 Parse_Message(&parser_TestParser, start_char);
	 TEST_ASSERT_EQUAL(POLLING_CMD_STATE, parser_TestParser.parser_state);

	 parser_TestParser.parser_state = PARSING_DATA_STATE;
	 Parse_Message(&parser_TestParser, random_char);
	 TEST_ASSERT_EQUAL(POLLING_START_STATE, parser_TestParser.parser_state);

	 parser_TestParser.parser_state = PARSING_DATA_STATE;
	 Parse_Message(&parser_TestParser, end_char);
	 TEST_ASSERT_EQUAL(POLLING_START_STATE, parser_TestParser.parser_state);

//	 Arrange: parser.parser_state = PARSING_DATA_STATE
//	 Act: call parser with in parameter char = length is odd not even"pairs"
//	 Assert: state = PARSING_DATA_STATE

//	   	 State: POLLING_END_CHARACTER

	 parser_TestParser.parser_state = POLLING_END_CHARACTER;
	 Parse_Message(&parser_TestParser, end_char);
	 TEST_ASSERT_EQUAL(POLLING_START_STATE, parser_TestParser.parser_state);

	 parser_TestParser.parser_state = POLLING_END_CHARACTER;
	 Parse_Message(&parser_TestParser, start_char);
	 TEST_ASSERT_EQUAL(POLLING_CMD_STATE, parser_TestParser.parser_state);

	 parser_TestParser.parser_state = POLLING_END_CHARACTER;
	 Parse_Message(&parser_TestParser, random_char);
	 TEST_ASSERT_EQUAL(POLLING_START_STATE, parser_TestParser.parser_state);

}

/*
 * Goal: Test the parsing of data
 * Arrange:
 * Act: Call the parser
 * Assert: Init status shall be okay
 * Assert: Buffer shall be initialized as expected
 *
 * Act: init with input parameter NULL
 * Assert: returned status shall be parser error
 *
 */

uint8_t arr[] = {1, 3, 2, 1, 5, 3, 3, 1, 3, 5, 8};


int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_The_Init_of_the_parser_object);
    RUN_TEST(test_the_parser_states);
    return UNITY_END();
}
