
/// 1802 Program Loader GPIO Pin Assignments
//  - include file for PiLoader 4 PCB
//  - update for 1802load.c release 4.3 and higher


#define data0  20 
#define data1  16
#define data2  12
#define data3  7
#define data4  8 
#define data5  25
#define data6  24 
#define data7  23

#define wait   26
#define clear  19
#define interrupt 6

#define ef3    15
#define ef4    21

#define mux    5

#define shift_register_data   17
#define shift_register_load   27
#define shift_register_clock  22

// Note : data pin 0 to 7 must be first and don't use ef3

int pins[] = { data0, data1, data2, data3, data4, data5, data6, data7, interrupt, ef4, wait, clear, mux } ;

// index in pins[] array of control pins

#define interrupt_pin  8
#define ef4_pin        9
#define wait_pin      10
#define clear_pin     11
#define mux_pin       12
