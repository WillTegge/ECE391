#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "paging.h"
#include "lib.h"
#include "rtc.h"
#include "filesystem.h"
#include "terminal.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}

static inline void sys_call(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $0x80");
}

/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
// /*------------------------------------------------------------------------------------------------
// int idt_test(){
// 	TEST_HEADER;

// 	int i;
// 	int result = PASS;
// 	for (i = 0; i < 10; ++i){
// 		if ((idt[i].offset_15_00 == NULL) && 
// 			(idt[i].offset_31_16 == NULL)){
// 			assertion_failure();
// 			result = FAIL;
// 		}
// 	}

// 	return result;
// }

// /* PAGING TESTS*/

// /*
// 	inside_4mb_test: Checks to see if there is allocated memory
// 	for the 4mb chunk 
// 	Inputs:	NONE
// 	OUTPUTS: PASS
// 	Side Effects: None
// 	Coverage: Paging
// */
// int inside_4mb_test(){
// 	TEST_HEADER;
// 	int* addy = (int*)0x600000;	//6MB
// 	int test=0;
// 	test=*(addy);
// 	return PASS;
// }

// /*
// 	inside_8mb_test: checks to see if there is allocated memory 
// 	past the 4MB chunk for kernal
// 	Inputs:	NONE
// 	OUTPUTS: NONE
// 	Side Effects: Throw a page fault
// 	Coverage: Paging
// */
// int past_8mb_test(){
// 	TEST_HEADER;
// 	int* addy = (int*)0x800000 + 200; //A little past 8MB
// 	int test=0;
// 	test=*(addy);
// 	return 1;
// }

// /*
// 	past_video_mem_test: Checks to see if there is allocated memory
// 	past the video memory 
// 	Inputs:	NONE
// 	OUTPUTS: Throws Page Fault
// 	Side Effects: None
// 	Coverage: Paging
// */
// int past_video_mem_test(){
// 	TEST_HEADER;
// 	int* addy = (int*)0xb9000;	//next 1kb chunk after video mem
// 	int test=0;
// 	test=*(addy);
// 	return 1;
// }

// /*
// 	div_zero: Checks to see if the divide by 0 interrupt works
// 	Inputs:	NONE
// 	OUTPUTS: PASS
// 	Side Effects: Throws a Divide by 0 exception
// 	Coverage: IDT
// */
// int div_zero(){
// 	int test = 12/0;
// 	return test;
// }

// /*
// 	sys_call_test: Checks to see if system call gets called
// 	Inputs:	NONE
// 	OUTPUTS: NONE
// 	Side Effects: Throws system call 
// 	Coverage: IDT
// */
// void sys_call_test(){
// 	TEST_HEADER;
// 	sys_call();
// }


// /*
// 	rtc_test: Checks to see if the RTC works
// 	Inputs:	NONE
// 	OUTPUTS: PASS
// 	Side Effects: None
// 	Coverage: RTC
// */
// int rtc_test(){
// 	TEST_HEADER;
// 	rtc_count = 70;
// 	while(1){
// 		if(rtc_count > 0){
// 			continue;
// 		}else{
// 			break;
// 		}
// 	}
// 	clear();
// 	printf("RTC Passed");
// 	return PASS;
// }
// // add more tests here

// /* Checkpoint 2 tests */

// /*
// 	rtc_read_write_demo(): Checks to see if RTC read and write functions work
// 	Inputs:	NONE
// 	OUTPUTS: PASS
// 	Side Effects: None
// 	Coverage: RTC
// */
// int rtc_read_write_demo(){
//     TEST_HEADER;
//     int32_t freq = 2;
//     int i;
//     rtc_count = 1;

//     rtc_open(NULL);         // testing rtc_open

//     // printf("Testing rtc_open Here \n");

//     while(freq <= 1024){
       
//         printf("Testing Frequency: %d Hz\n", freq);

//         for(i = 0; i < freq; i++){
//             rtc_read(NULL, NULL, NULL);
//             // printf("Testing rtc_read Here \n");
   
//             rtc_write(NULL, &freq, sizeof(int32_t));
//             // printf("Testing Frequency: %d Hz\n", freq);
           
//             printf("1");

//         }

       
//         printf("\n");

//         // clear();

//         freq = freq * 2;

//     }

// 	return PASS;


// }

// /*
// 	rtc_open_close_demo(): Checks to see if RTC open and close functions work
// 	Inputs:	NONE
// 	OUTPUTS: PASS
// 	Side Effects: None
// 	Coverage: RTC
// */
// int rtc_open_close_demo(){
//     TEST_HEADER;

//     int32_t temp_1 = 0;

//     int32_t temp_2 = 0;

//     open_frequency = 0;

//     temp_1 = rtc_open(NULL);

//     printf("%d \n", open_frequency);

//     temp_2 = rtc_close(NULL);

//     if((temp_1 == 0) & (temp_2 == 0) & (open_frequency == 2)){
 
//         return PASS;
 
//     }
//     else{
 
//         return FAIL;
 
//     }

// }



// // need a test that checks for invalid inputs to the rtc
// // tell it to write an invalid frequency
// // have it recognize that it is an invalid frequency by printing out invalid or something like that

// /*
// 	rtc_test_for_invalid_inputs(): Checks to see if RTC catches invalid inputs and returns an error
// 	Inputs:	NONE
// 	OUTPUTS: PASS
// 	Side Effects: None
// 	Coverage: RTC
// */
// int rtc_test_for_invalid_inputs(){
// 	TEST_HEADER;

// 	int32_t invalid_freq = 73;

// 	int32_t temp_1 = 0;

// 	rtc_open(NULL);       

// 	printf("Testing Non Power of Two Frequency: %d \n", invalid_freq);
// 	temp_1 = rtc_write(NULL, &invalid_freq, sizeof(int32_t));

// 	if(temp_1 != -1){			// rtc_write should fail and return -1 since frequency is invalid
// 		return FAIL;
// 	}
// 	else{
// 		printf("Invalid Frequency \n");
// 	}
	
// 	invalid_freq = -1;
// 	printf("Testing Frequency Less than 2: %d \n", invalid_freq);
// 	temp_1 = rtc_write(NULL, &invalid_freq, sizeof(int32_t));

// 	if(temp_1 != -1){			// rtc_write should fail and return -1 since frequency is invalid
// 		return FAIL;
// 	}
// 	else{
// 		printf("Invalid Frequency \n");
// 	}

// 	invalid_freq = 3000;
// 	printf("Testing Frequency Greater than 1024: %d \n", invalid_freq);
// 	temp_1 = rtc_write(NULL, &invalid_freq, sizeof(int32_t));

// 	if(temp_1 != -1){			// rtc_write should fail and return -1 since frequency is invalid
// 		return FAIL;
// 	}
// 	else{
// 		printf("Invalid Frequency \n");
// 	}

// 	return PASS;

// }


// /*
// 	terminal_read_write_test(): Checks to see if Terminal read and write functions are working
// 	Inputs:	NONE
// 	OUTPUTS: PASS
// 	Side Effects: None
// 	Coverage: Terminal
// */
// int terminal_read_write_test(){
// 	TEST_HEADER;


// 	int32_t nbytes = 128;
// 	int i;
// 	char* test_buf[128] ;//= {3};
// 	char current_char;
// 	int32_t ret_val;

// 	ret_val = terminal_init();
// 	// printf("keyboard terminal after initialization:");
// 	// for(i = 0; i < 128; i++){

// 	// 	putc(keyboard_buffer[i]);

// 	// }


// 	// if(ret_val != 0){
// 	// 	printf("terminal open failed\n");
// 	// 	return -1;
// 	// }


// 	while(1){

// 		// testing terminal read 
// 		ret_val = terminal_read(1, test_buf, nbytes);	
// 		// printf("number of chars to terminal buf is: %d\n", ret_val);	

// 		// testing terminal write
// 		terminal_write(1, test_buf, nbytes);
// 		// printf("\n");

// 		// nbytes = nbytes / 2;

// 	}

// 	return 0;

// }



// /*
// 	terminal_open_close_test(): Checks to see if Terminal open and close functions are working
// 	Inputs:	NONE
// 	OUTPUTS: PASS
// 	Side Effects: None
// 	Coverage: Terminal
// */
// int terminal_open_close_test(){
// 	TEST_HEADER;

// 	int32_t return_value = 0;

// 	printf("Testing Terminal Open/Init \n");

// 	return_value = terminal_init();

// 	if(return_value != 0){

// 		return FAIL;

// 	}

// 	printf("Testing Terminal Close \n");

// 	return_value = terminal_close(0);

// 	if(return_value != 0){

// 		return FAIL;

// 	}


// 	return PASS;

// }


// /*
// 	terminal_buffer_overflow_test(): Checks to see if Terminal stops buffer overflow
// 	Inputs:	NONE
// 	OUTPUTS: PASS
// 	Side Effects: None
// 	Coverage: Terminal
// */
// int terminal_buffer_overflow_test(){
// 	TEST_HEADER;

// 	int32_t invalid_nbytes = 200;
// 	char* test_buf[128] = {3};
	
// 	printf("Testing Buffer of Size 200 \n");
// 	int32_t ret_val = terminal_read(1, test_buf, invalid_nbytes);	

// 	if(ret_val != -1){

// 		return FAIL;

// 	}
// 	else{

// 		printf("Too many bytes to copy! \n");
// 		return PASS;

// 	}

// 	return PASS;


// }


// /*
// 	terminal_write_test(): Checks to see if Terminal writes from a buffer to the screen
// 	Inputs:	NONE
// 	OUTPUTS: none
// 	Side Effects: None
// 	Coverage: Terminal
// */
// void terminal_write_test(){
// 	TEST_HEADER;

// 	char* buf = "319OS> ";
// 	terminal_write(1, buf, 7);

// }


// /*
// 	testing_keyboard_buffer(): Checks to see if our keyboard buffer stores values we want
// 	Inputs:	NONE
// 	OUTPUTS: none
// 	Side Effects: None
// 	Coverage: Terminal
// */
// void testing_keyboard_buffer(){

// 	int i;

// 	char* buf = "319OS> ";

// 	memcpy(keyboard_buffer, buf, 7);

// 	for(i = 0; i < 8; i++){

// 		putc(keyboard_buffer[i]);

// 	}


// }




// /*
// 	dir_test(): prints out the file system directory 
// 	Inputs:	NONE
// 	OUTPUTS: none
// 	Side Effects: None
// 	Coverage: Filesystem
// */
// int dir_test(){
// 	uint8_t buf[2000][32];
// 	//printf("Looping through directories:\n");
// 	int index;
// 	for(index = 0; index < bbg->num_dir_entries; index++){
// 		//printf("Accessing dir_read function:\n");
// 		dir_read(index, buf[index], 32);
// 		printf("File Name: %s\n", (int8_t*)buf[index]);
// 	}
// 	printf("\n");
// 	return PASS;
// }


// /*
// 	file(): prints read data from a file
// 	Inputs:	NONE
// 	OUTPUTS: none
// 	Side Effects: None
// 	Coverage: Filesystem
// */
// int file_test(){
// 	/*
// 	6 = fish
// 	9 = cat
// 	10 = frame0.txt
// 	11 = verylong...
// 	12 = ls
// 	16 = hello
// */
// 	(bbg->dir_entries)[10];
// 	int8_t buf[10000];
// 	file_read((bbg->dir_entries)[10].inode_num, buf, (((inode_t*)bbg + 1+ ((bbg->dir_entries)[10].inode_num))->file_size));
// 	printf("%s", (int8_t*)buf);
// 	printf("DONE");
// 	return PASS;
// }

// /* Checkpoint 3 tests */
// /* Checkpoint 4 tests */
// /* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	clear();

	/* IDT TESTS */
	// TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("page_test", page_test());
	// div_zero(); 
	// sys_call_test();

	/* RTC TESTS */
	// rtc_test();
	// TEST_OUTPUT("rtc_read_write_demo", rtc_read_write_demo());
	// TEST_OUTPUT("rtc_open_close_demo", rtc_open_close_demo());
	// TEST_OUTPUT("rtc_test_for_invalid_inputs", rtc_test_for_invalid_inputs());



	/* TERMINAL TESTS */
	// TEST_OUTPUT("terminal_open_close_test", terminal_open_close_test());
	// terminal_read_write_test();
	// // TEST_OUTPUT("terminal_buffer_overflow_test", terminal_buffer_overflow_test());
	// terminal_write_test();
	// testing_keyboard_buffer();

	

	/* PAGING TESTS */
	// inside_4mb_test();
	// past_video_mem_test();
	// past_8mb_test();

	/*FILESYSTEM TEST*/
	// dir_test();
	// file_test();

}
