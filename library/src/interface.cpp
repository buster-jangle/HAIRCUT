//
// Created by sean on 7/9/23.
//

#include "HAIRCUT.h"


int HAIRCUT::BlockInterface::initInterface(int inSamples, int outSamples){
    inBuffer.buffer = (f32_complex*)malloc(sizeof(f32_complex) * inSamples);
    outBuffer.buffer = (f32_complex*)malloc(sizeof(f32_complex) * outSamples);

    inBuffer.occupation = 0;
    inBuffer.size = inSamples;

    outBuffer.occupation = 0;
    outBuffer.size = outSamples;
}
int HAIRCUT::BlockInterface::push(f32_complex* src, int count){

    int samplesToPush = count;

//    if(samplesToPush <= getBufferSpaceAvailable() ){ // if there is enough room in the buffer for all samples requested, memcpy the samples
//        PLOGD.printf("Object %016X: Pushing %i samples. Dest buffer occupation: %i", this, samplesToPush, inBuffer.occupation);
//        memcpy(inBuffer.buffer + inBuffer.occupation, src, samplesToPush);
//        inBuffer.occupation += samplesToPush;
//        return 1;
//    }

    // Reach here if there is not enough room for all samples in the buffer
    while(samplesToPush > 0){
        int batchSize = this->getBufferSpaceAvailable();
        if(batchSize > samplesToPush){
            batchSize = samplesToPush;
        }
        PLOGD.printf("Object %016X: Pushing %i samples. of %i remaining", this, batchSize, samplesToPush);
        memcpy(inBuffer.buffer + inBuffer.occupation, src, batchSize); // completley fill input buffer
        src += batchSize;
        inBuffer.occupation += batchSize;
        samplesToPush -= batchSize;

        if(getBufferSpaceAvailable() == 0) {
            PLOGD.printf("Object %016X: Input buffer full. Executing process.", this);
            if (!this->execute()) { // execute processing of samples to make room for the next batch
                PLOGE.printf("Object %016X: process execution failed.");
                return 0;// return fail if execute processing failed
            }
        }

    }
    return 1;
}