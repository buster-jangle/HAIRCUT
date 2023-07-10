/*

*/

#include "main.h" // include main header
#include "HAIRCUT.h" // include the default library

// include common headers by default
#include "stdio.h"
#include "stdlib.h"
#include <errno.h> // Error integer and strerror() function
#include <unistd.h> // write(), read(), close()
#include <fcntl.h> // Contains file controls like O_RDWR
#include <string>
#include <limits.h>
#include <chrono>
#include "stdbool.h"
#include <iostream>
#include <thread>
#include <mutex>
#include "plog/Log.h"
#include "plog/Initializers/RollingFileInitializer.h"
#include "plog/Appenders/ColorConsoleAppender.h"
#include "CLI/App.hpp"
#include "CLI/Formatter.hpp"
#include "CLI/Config.hpp"
#include "cmath"

#include <fftw3.h>
#include "lime/LimeSuite.h"

using namespace std; // default namespace

#define clear() printf("\033[H\033[J")
#define gotoxy(x,y) printf("\033[%d;%dH", (y), (x))

#define MAX_DEVICES 8
#define MASK_12BIT ((1<<12)-1)

#define LMS_FIFO_SIZE (1024*1024)

float baseFrequency = 915.0e6;
float samplesPerSecond =1e6; // Glitches occur in TX signal at less than 3e6 for unknown reasons
float txGain = 30; // in decibels
int codeLength = (1024*16);
int interpolationFactor = 1;
uint32_t seed = 0x1BADA551;

typedef struct int16_complex{
    volatile int16_t real;
    volatile int16_t imag;
} int16_complex;
//typedef struct f32_complex{
//    volatile float real;
//    volatile float imag;
//} f32_complex;


lms_stream_t txStreamGlobal;


uint64_t getTimeStamp() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}
uint64_t lastTransmission = 0;

int lmsInit(lms_device_t* devHandle);

int main(int argc, char *argv[]) {
    CLI::App cli{"-HAIRCUT: LimeSdr-based transmitter-"};

    std::string logfilePath = "HAIRCUT.log";
    cli.add_option("-l,--logfile", logfilePath, "Logfile destination. ex: logs/HAIRCUT.log");
    CLI11_PARSE(cli);

    static plog::RollingFileAppender<plog::CsvFormatter> fileAppender(logfilePath.c_str(), 8000,3); // Create the 1st appender.
    static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender; // Create the 2nd appender.
    plog::init(plog::verbose, &fileAppender).addAppender(&consoleAppender); // Initialize the logger with the both appenders.    PLOGI << "Started logging.";

//    HAIRCUT object;
//    object.init(plog::info, plog::get()); // Initialize a library with a PLOG appender.

    PLOGI.printf("LimeSuite library version: %s", LMS_GetLibraryVersion());
    PLOGV << "Obtaining available device list";
    lms_info_str_t deviceInfo[MAX_DEVICES]; // Allocate info stype for up to 16 devices
    int devCount = LMS_GetDeviceList(deviceInfo);
    if (devCount < 0) {
        PLOGE << "Failed to get device list";
        return EXIT_FAILURE;
    }
    if (devCount == 0) {
        PLOGE << "No LimeSDR device found";
        return EXIT_FAILURE;
    }
    PLOGI.printf("Devices found: %i",devCount);
    for (int i = 0; i < devCount; ++i)
        PLOGI.printf("\tIndex %i: %s", i, deviceInfo[i]);

    lms_device_t* deviceHandle[MAX_DEVICES];
    for(int i = 0; i < devCount; ++i){
        PLOGI.printf("Opening device %i", i);
        if(LMS_Open(&deviceHandle[i], deviceInfo[i], nullptr)) {
            PLOGE.printf("Failed to open device of index %i", i);
            return EXIT_FAILURE;
        }
        else{
            PLOGI.printf("Initializing device %i", i);
            if(lmsInit(deviceHandle[0]))
                return EXIT_FAILURE;
        }
    }

    // Generate code sequence as a lookup table
    PLOGI.printf("Generating code sequence of length %i with code %08X...", codeLength, seed);
    srand(seed); /// seed the PRNG


    //int16_complex codeSequence[codeLength];
    //int16_complex* codeSequence = (int16_complex*)malloc(codeLength * sizeof(int16_complex));
    int16_complex* codeSequence = (int16_complex*)malloc(codeLength * sizeof(int16_complex));
    int16_complex* deadAir = (int16_complex*)malloc(codeLength * sizeof(int16_complex));

    memset(deadAir, 0, codeLength * sizeof(int16_complex));
    //memset(codeSequence, 0, codeLength * sizeof(f32_complex));

    //fftwf_complex floatCodeSequenceDiscrete[codeLength * interpolationFactor]; // Discrete values are first loaded into this array, then transformed to the followng array, then zero-padded, then inverse-transformed back into this array to acheieve interpolaton.
    fftwf_complex* floatCodeSequenceDiscrete = (fftwf_complex*) malloc(codeLength * interpolationFactor * sizeof(fftwf_complex));

    //fftwf_complex floatCodeSequenceFrequency[codeLength];
    fftwf_complex* floatCodeSequenceFrequency = (fftwf_complex*) malloc(codeLength * sizeof(fftwf_complex));
    // not needed //memset(floatCodeSequenceDiscrete, 0, sizeof(fftw_complex) * codeLength * interpolationFactor); // Zero out the whole array. Part of the array will be overwritten anyway, but this ensures zero padding works correctly

    float freq = 10000;
    int16_complex* codeSeqPtr = codeSequence;

    printf("startSeq\r\n");
    for(int i = 0; i < codeLength; ++i){
//        float sweep = (((float)i / (float)codeLength) * 2.0f) - 1.0f;
//        float phase = sweep * M_PI * (freq * 1.0);
//
//        codeSeqPtr->real = (int16_t)round(sin(phase) * 1047.0);
//        codeSeqPtr->imag = (int16_t)round(cos(phase) * 1047.0);
//        printf("%i: %i, %i\r\n", i, codeSeqPtr->real, codeSeqPtr->imag);
//        ++codeSeqPtr;
//        codeSequence[i].real = 1024;
//        codeSequence[i].imag = 1024;

        uint32_t randVal = rand();
        codeSequence[i].real = (uint16_t)(randVal & MASK_12BIT)-2048;
        codeSequence[i].imag = (uint16_t)((randVal >> 16) & MASK_12BIT)-2048;

        floatCodeSequenceDiscrete[i][0] = codeSequence[i].real;
        floatCodeSequenceDiscrete[i][1] = codeSequence[i].imag;
    }
    PLOGI.printf("Done");

////////// Configure FFT plans
    fftwf_plan fftForward = fftwf_plan_dft_1d(
            codeLength, // number of fft bins
            floatCodeSequenceDiscrete,
            floatCodeSequenceFrequency,
            FFTW_FORWARD,
            FFTW_ESTIMATE
            );
    fftwf_plan fftInverse = fftwf_plan_dft_1d(
            codeLength, // number of fft bins
            floatCodeSequenceFrequency,
            floatCodeSequenceDiscrete,
            FFTW_BACKWARD,
            FFTW_ESTIMATE
    );
    /// apply windowing function
    for(int i = 0; i < codeLength; ++i){
//        float sweep = (float)i / (float)codeLength;
//        float window = (cos(sweep * 2.0f * M_PIf) + 1) * 0.25f;
        float window = 0.5f;
        floatCodeSequenceDiscrete[i][0] *= window;
        floatCodeSequenceDiscrete[i][1] *= window;
    }


    /// calculate forward fft
    fftwf_execute(fftForward);

    //// Apply filter by cutting off bins
    float lowpassCutoff = 0.5f;
    int binCutoff = round((float)codeLength * lowpassCutoff * 0.5f);
    int n2bin = codeLength / 2;
    for(int i = 0; i < binCutoff; ++i){
        int indexPos = n2bin + i;
        int indexNeg = n2bin - i;
        floatCodeSequenceFrequency[indexPos][0] = 0; //real // cut positive frequencies
        floatCodeSequenceFrequency[indexPos][1] = 0; //imag

        floatCodeSequenceFrequency[indexNeg][0] = 0; //real // cut negativeFrequencies
        floatCodeSequenceFrequency[indexNeg][1] = 0; //imag
    }
    // calculate inverse fft
    fftwf_execute(fftInverse);
    // put results in int16 datatypes
    float scalingFactor = 0.5f / ( (float)codeLength); // calculate a scale factor that preserves magnitude after FFT and IFFT

    for(int i = 0; i < codeLength; ++i){
        floatCodeSequenceDiscrete[i][0] *= scalingFactor;// values must be rescaled after fft
        floatCodeSequenceDiscrete[i][1] *= scalingFactor;
        codeSequence[i].real = round(floatCodeSequenceDiscrete[i][0]);
        codeSequence[i].imag = round(floatCodeSequenceDiscrete[i][1]);
    }

    for(int i = 0; i < codeLength; ++i){
        if(codeSequence[i].real > 2047 || codeSequence[i].real < -2048){
            PLOGE.printf("Code sequence value out of range: %i, %ir", i, codeSequence[i].real);
        }
        if(codeSequence[i].imag > 2047 || codeSequence[i].imag < -2048){
            PLOGE.printf("Code sequence value out of range: %i, %ii", i, codeSequence[i].imag);
        }
    }
    // free fft intermediates
    free(floatCodeSequenceDiscrete);
    free(floatCodeSequenceFrequency);

    while(true){
        lms_stream_status_t streamStatus;
        if(LMS_GetStreamStatus(&txStreamGlobal, &streamStatus) != 0){
            PLOGE.printf("Failed to get global TX 0 stream status: %s", LMS_GetLastErrorMessage());
            return EXIT_FAILURE;
        }

        clear();
        gotoxy(0, 0);
        printf("TX Stream status:\n");
        if(streamStatus.active)
            printf("\tState: ACTIVE\n");
        else
            printf("\tState: NOT ACTIVE\n");
        printf("\tLink rate:%3.2fMB/S\n", streamStatus.linkRate/(float)(1024*1024));
        printf("\tUnderruns:%i\n", streamStatus.underrun);
        printf("\tOverruns:%i\n", streamStatus.overrun);
        printf("\tDropped packets:%i\n", streamStatus.droppedPackets);
        printf("\tBuffer: %3.1f%\n", 100.0f*(float)streamStatus.fifoFilledCount/(float)streamStatus.fifoSize);
        printf("\tFifo filled: %i\n", streamStatus.fifoFilledCount);
        printf("\r\n");
        //printf("rollingPhase: %f, cos: %f, sin:%f\r\n", rollingPhase, (float)(sinf(rollingPhase) * 0.2f), (float)(cosf(rollingPhase) * 0.2f));

//        lms_stream_meta_t streamMetadata;
//        streamMetadata.flushPartialPacket = false;
//        streamMetadata.timestamp = 0;
//        streamMetadata.waitForTimestamp = false;

        lms_stream_meta_t tx_metadata; //Use metadata for additional control over sample send function behavior
        tx_metadata.flushPartialPacket = true; //do not force sending of incomplete packet
        tx_metadata.waitForTimestamp = false; //Enable synchronization to HW timestamp
        tx_metadata.timestamp = 0;

        uint32_t bufferSpace = streamStatus.fifoSize - streamStatus.fifoFilledCount;
        while(bufferSpace > codeLength){

            int16_complex* bufPtr = deadAir;
            if(getTimeStamp() > lastTransmission + 1000000){
                lastTransmission = getTimeStamp();
                bufPtr = codeSequence;
            }
            if(LMS_SendStream(&txStreamGlobal, bufPtr, codeLength, &tx_metadata, 100) != codeLength){
                PLOGE.printf("Failed to send all samples to TX stream: %s", LMS_GetLastErrorMessage());
                return EXIT_FAILURE;
            }
            bufferSpace -= codeLength;
        }

        usleep(5000);
	}
}

int lmsTxChannelInit(lms_device_t* devHandle, int channel){
    /// channel-specific config
    int status = 0;
    status = LMS_EnableChannel(devHandle, LMS_CH_TX, channel, true); // enable tx
    if(status){
        PLOGE.printf("Failed to enable channel CH%i_TX: %s", channel, LMS_GetLastErrorMessage());
        return EXIT_FAILURE;
    }
    status = LMS_SetLOFrequency(devHandle, LMS_CH_TX, channel, baseFrequency);
    if(status){
        PLOGE.printf("Failed to set LO frequency on CH%i_TX: %s", channel, LMS_GetLastErrorMessage());
        return EXIT_FAILURE;
    }

    status = LMS_SetAntenna(devHandle, LMS_CH_TX, channel, 1); // set TX to antenna 1 (0 = no path, 1 = TX1, 2 = TX2)
    if(status){
        PLOGE.printf("Failed to set antenna on CH%i_TX: %s", channel, LMS_GetLastErrorMessage());
        return EXIT_FAILURE;
    }
    status = LMS_SetLPFBW(devHandle, LMS_CH_TX, channel, samplesPerSecond * 1.0); // set the analog lowpass filter bandwidth to samples per second, with a bit of margin
    if(status){
        PLOGE.printf("Failed to set LPF bandwidth on CH%i_TX: %s", channel, LMS_GetLastErrorMessage());
        return EXIT_FAILURE;
    }
    status = LMS_SetGaindB(devHandle, LMS_CH_TX, channel, txGain);
    if(status){
        PLOGE.printf("Failed to set TX gain on CH%i_TX: %s", channel, LMS_GetLastErrorMessage());
        return EXIT_FAILURE;
    }
    status = LMS_Calibrate(devHandle, LMS_CH_TX, channel, samplesPerSecond * 1.0, 0);
    if(status){
        PLOGE.printf("Failed to calibrate on CH%i_TX: %s", channel, LMS_GetLastErrorMessage());
        return EXIT_FAILURE;
    }
//    status = LMS_GetGaindB(devHandle, LMS_CH_TX, channel, samplesPerSecond * 1.2, 0);
//    if(status){
//        PLOGE.printf("Failed to calibrate on CH%i_TX: %s", channel, LMS_GetLastErrorMessage());
//        return EXIT_FAILURE;
//    }
    return EXIT_SUCCESS;
}

int lmsInit(lms_device_t* devHandle){
    int status = 0;
    PLOGI.printf("Initializing...");
//    status = LMS_Reset(devHandle); //
//    if(status){
//        PLOGE.printf("Failed to reset: %s", LMS_GetLastErrorMessage());
//        return EXIT_FAILURE;
//    }
    if(LMS_Init(devHandle)){
        PLOGE.printf("Failed to initialize.");
        return EXIT_FAILURE;
    }
    else{
        PLOGI.printf("Initialized.");
    }


    ///// device-wide config
    LMS_SetClockFreq(devHandle, LMS_CLOCK_EXTREF, 10e6); // use external clock

    status = LMS_SetSampleRate(devHandle, samplesPerSecond, 0); // Set samplerate with no oversampling
    if(status){
        PLOGE.printf("Failed to set samplerate: %s", LMS_GetLastErrorMessage());
        return EXIT_FAILURE;
    }
    // channel specific configs
    if(lmsTxChannelInit(devHandle, 1) == EXIT_FAILURE) // configure channel 0
        return EXIT_FAILURE;
    // Configure TX stream for channel 0
    lms_stream_t* txStream = &txStreamGlobal;
    txStream->fifoSize = LMS_FIFO_SIZE; // 256KSample fifo
    txStream->throughputVsLatency = 0.5; // value is 0 to 1.0. Higher gives better throughput, but more latency.
    txStream->dataFmt = lms_stream_t::LMS_FMT_I12; // data input format is 12-bit data stored in int16
    //txStream->linkFmt = lms_stream_t::LMS_LINK_FMT_I16; // Keep link 12-bit, which achievs maximum throughput
    txStream->channel = 1; // associate stream with channel TX 0
    txStream->isTx = true; //

    status = LMS_SetupStream(devHandle, txStream);
    if(status){
        PLOGE.printf("Failed to set up TX stream: %s", LMS_GetLastErrorMessage());
        return EXIT_FAILURE;
    }
    status = LMS_StartStream(txStream);
    if(status){
        PLOGE.printf("Failed to start TX stream: %s", LMS_GetLastErrorMessage());
        return EXIT_FAILURE;
    }

    PLOGI.printf("Finished device setup");
    return EXIT_SUCCESS;
}





