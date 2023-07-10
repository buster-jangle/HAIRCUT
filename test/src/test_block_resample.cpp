/*
  ___   _   _   ___   _   _ _____ ___   _   __
 / _ \ | \ | | / _ \ | \ | |_   _/ _ \ | | / /
/ /_\ \|  \| |/ /_\ \|  \| | | |/ /_\ \| |/ /
|  _  || . ` ||  _  || . ` | | ||  _  ||    \
| | | || |\  || | | || |\  | | || | | || |\  \
\_| |_/\_| \_/\_| |_/\_| \_/ \_/\_| |_/\_| \_/

Project $PROJECT_NAME main.cpp
Initial file automatically generated by PROGENY on 04/18/23 12:42:56
*/

#include "test.h" // include main header
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
#include <plog/Log.h>
#include "plog/Initializers/RollingFileInitializer.h"
#include "math.h"
using namespace std; // default namespace

#define SAMPLE_COUNT (1024*8)
f32_complex testDataset[SAMPLE_COUNT];

HAIRCUT pipeline;
HAIRCUT::resample interpolate;
void fail(){
    PLOG_ERROR << "Test Failed";
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]){

    static plog::RollingFileAppender<plog::CsvFormatter> fileAppender("test_block_fft.log", 1048576, 3); // Create the file appender, up to 1MB per file, rolling over 3 files
    static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender; // Create the console appender
    plog::init(plog::debug,&fileAppender).addAppender(&consoleAppender); // Step2: initialize the logger

    PLOG_INFO << "Starting test.";

    for(int i = 0; i < SAMPLE_COUNT; ++i){
        testDataset->real = sinf((float)i);
        testDataset->imag = cosf((float)i);
    }
    bool testStatus = false;

    if(!pipeline.init(plog::get())){
        fail();
    }

    if(!interpolate.init(256, 4096)){
        fail();
    }

    /// Push in an oversized batch
    interpolate.push(testDataset, SAMPLE_COUNT);

    /// push in randomly sized small batches
    int i = 0;
    while( i < SAMPLE_COUNT) {
        interpolate.push(testDataset, rand() % 1024);
        i += 9;
    }



    PLOG_INFO << "Test Passed";
    return EXIT_SUCCESS;
}