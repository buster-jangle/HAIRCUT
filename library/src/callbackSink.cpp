//
// Created by sean on 7/9/23.
//
#include "HAIRCUT.h" // include header

int HAIRCUT::callbackSink::init(void (*callback_i)(f32_complex*, int count), int sampleCount_i) {

    sampleCount = sampleCount_i;
    PLOGD.printf("Object %016X: Creating callbackSink block with size: %i, batchsize: %i, inversion: %i", this, sampleCount);
    callback = callback_i;
    initInterface(sampleCount, 1);

    /// Perform sanity checks. Return false if any created pointer is null
    if(!inBuffer[0].buffer){
        PLOGE.printf("Created inBuffer failed");
        return 0;
    }

    return 1;
}
int HAIRCUT::callbackSink::execute()  {
    PLOGD.printf("Object %016X: Execute callback sink", this);

    callback(inBuffer[0].buffer, inBuffer->size);
    inBuffer[0].occupation -= sampleCount; // udate buffer occupation according to how much data the operaton serviced
    return 1;
}