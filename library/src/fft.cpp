//
// Created by sean on 7/9/23.
//
#include "HAIRCUT.h" // include header

int HAIRCUT::FFT::init(int size, int batchSize, bool inverse) {
    PLOGD.printf("Object %016X: Creating FFT block with size: %i, batchsize: %i, inversion: %i", this, size, batchSize, inverse);

    initInterface(size * batchSize, size * batchSize);

    fftSize = size;
    int fftSign;
    if(inverse){
        fftSign  = FFTW_BACKWARD;
    }
    else{
        fftSign = FFTW_FORWARD;
    }
    fftwPlannerMutex.lock(); // lock the FFTW Planner mutex, so that
    // Create FFT plan with 8 threads
    fftw_plan_with_nthreads(8);
    fftPlan = fftwf_plan_dft_1d(
            size, // number of fft bins
            (fftwf_complex*)inBuffer[0].buffer,
            (fftwf_complex*)outBuffer[0].buffer,
            fftSign,
            FFTW_PATIENT
    );
    fftwPlannerMutex.unlock();

    /// Perform sanity checks. Return false if any created pointer is null
    if(!inBuffer[0].buffer){
        PLOGE.printf("Created inBuffer failed");
        return 0;
    }
    if(!outBuffer[0].buffer){
        PLOGE.printf("Created outBuffer failed");
        return 0;
    }
    if(!fftPlan){
        PLOGE.printf("Created fftwPlan failed");
        return 0;
    }
    return 1;
}
int HAIRCUT::FFT::execute()  {
//    if(getBufferSpaceAvailable() < fftSize){
//        PLOGD.printf("Object %016X: Output buffer full. Pushing to next stage. %i downstream blocks", this, destinationBlock.size());
//        for(int i = 0; i < destinationBlock.size(); ++i){
//            if(!destinationBlock[i]->push(outBuffer.buffer, fftSize)){
//                PLOGE.printf("Object %016X: Pushing samples failed.");
//                return 0;
//            }
//        }
//        outBuffer.occupation -= fftSize;
//    }

    fftwf_execute(fftPlan); // execute fftw has no return
    outBuffer[0].occupation += fftSize;
    inBuffer[0].occupation -= fftSize; // udate buffer occupation according to how much data the operaton serviced
    return 1;
}