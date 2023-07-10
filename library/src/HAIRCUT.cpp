/*

*/

#include "HAIRCUT.h" // include header

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


using namespace std; // default namespace

//HAIRCUT::HAIRCUT(){
//    status = 0;
//}
//
bool HAIRCUT::init(plog::IAppender* appender, plog::Severity severity){
    PLOGI << "initializing";

    if(!fftw_init_threads()){
        PLOGE.printf("Failed to initialize FFTW multithreading");
    }
    return true;
}

