//
// Created by sean on 7/9/23.
//

#include "HAIRCUT.h"


int HAIRCUT::BlockInterface::initInterface(int inSamples, int outSamples){
    inBuffer = new sampleBuffer_t();
    outBuffer = new sampleBuffer_t();

    for(int i = 0; i < inChannels; ++i) {
        inBuffer[i].buffer = (f32_complex *) malloc(sizeof(f32_complex) * inSamples);
        inBuffer[i].occupation = 0;
        inBuffer[i].size = inSamples;
    }
    for(int i = 0; i < outChannels; ++i) {
        outBuffer[i].buffer = (f32_complex *) malloc(sizeof(f32_complex) * outSamples);
        outBuffer[i].occupation = 0;
        outBuffer[i].size = outSamples;
    }
    return 1;
}

// push samples through the DSP pipeline
int HAIRCUT::BlockInterface::push(f32_complex* src, int count, int channel){

    int samplesToPush = count;
    std::vector<std::thread> threads; // recursive calls will later be spllit off into new threads

    while(samplesToPush > 0){
        int batchSize = this->getBufferSpaceAvailable();
        if(batchSize > samplesToPush){
            batchSize = samplesToPush;
        }
        PLOGD.printf("Object %016X: Pushing %i samples. of %i remaining", this, batchSize, samplesToPush);
        memcpy(inBuffer[channel].buffer + inBuffer[channel].occupation, src, batchSize); // completely fill input buffer
        src += batchSize;
        inBuffer[channel].occupation += batchSize;
        samplesToPush -= batchSize;

        if(getBufferSpaceAvailable() == 0) {
            PLOGD.printf("Object %016X: Input buffer full. Executing process.", this);
            /// Before execution, check that there is room in the output buffer(s). If not, execute the next stages
            for(int ch = 0; ch < outChannels; ++ch) {
                if (getBufferSpaceAvailable() < outBuffer[ch].size) {
                    PLOGD.printf("Object %016X: Output buffer full. Pushing to next stage. %i downstream blocks", this, outBuffer[ch].connection.size());
                    for (int i = 0; i < outBuffer[ch].connection.size(); ++i) { // for each connection downstream of each channel, push samples to it
                        /// not working yet // threads.emplace_back(&BlockInterface::push, outBuffer[ch].connection[i], outBuffer[ch].buffer, outBuffer[i].size);
                        if (!outBuffer[ch].connection[i]->push(outBuffer[ch].buffer, outBuffer[i].size)) {
                            PLOGE.printf("Object %016X: Pushing samples failed.");
                            return 0;
                        }
                    }
                    outBuffer[ch].occupation -= outBuffer[ch].size;
                }
            }
            if (!this->execute()) { // execute processing of samples to make room for the next batch
                PLOGE.printf("Object %016X: process execution failed.");
                return 0;// return fail if execute processing failed
            }
            else{

            }
        }

    }
    // Wait for all threads to finish
    for (auto& t : threads) {
        t.join();
    }
    return 1;
}

//int HAIRCUT::BlockInterface::pull(f32_complex* src, int count) {
//    PLOGD.printf("Object %016X: Pulling %i samples", this, count);
//
//    int samplesToPull = count;
//
//    while (samplesToPull > 0) {
//        if(this->getNumSamplesAvailable() == 0){ // if there are no samples available, then proceed execute this block
//            PLOGD.printf("Object %016X: No samples available. Attempting to execute block", this);
//            if(getBufferSpaceAvailable() > 0){ // Before this block can execute, the input buffer must be full. Check if it isnt
//                PLOGD.printf("Object %016X: Input buffer not filled. Attempting pull from upstream", this);
//
//            }
//        }
//
//
//        PLOGD.printf("Object %016X: Pulling %i samples. of %i remaining", this, batchSize, samplesToPull);
//        memcpy(inBuffer.buffer + inBuffer.occupation, src, batchSize); // completley fill input buffer
//        src += batchSize;
//        inBuffer.occupation += batchSize;
//        samplesToPush -= batchSize;
//
//    }
//}