#include "audio.h"


// I/O flags
volatile u8 audioRxDone = 0;
volatile u8 audioTxDone = 0;

volatile u8 processAudio = 0;


// I/O buffers
__attribute__((l1_data_A))
__attribute__((aligned(32)))
buffer_t inputChannels0;

__attribute__((l1_data_A))
__attribute__((aligned(32)))
buffer_t inputChannels1;

__attribute__((l1_data_B))
__attribute__((aligned(32)))
buffer_t outputChannels0;

__attribute__((l1_data_B))
__attribute__((aligned(32)))
buffer_t outputChannels1;
