//
// Created by sean on 7/10/23.
//
#include "HAIRCUT.h" // include header

int HAIRCUT::resample::init(int inputSize_i, int outputSize_i, int batchSize) {
    inputSize = inputSize_i;
    outputSize = outputSize_i;

    if(inputSize & 1 || outputSize & 1){
        PLOGE.printf("Input and output sizes must be an even integer!");
        return 0;
    }

    PLOGD.printf("Object %016X: Creating interpolation block with input size: %i, batchsize: %i, output size: %i", this, inputSize, batchSize, outputSize);

    initInterface(inputSize * batchSize, outputSize * batchSize);

    /// allocate intermediate frequency domain buffers
    forwardOutBuffer = (f32_complex *) malloc(sizeof(f32_complex) * inputSize);
    inverseInBuffer = (f32_complex *) malloc(sizeof(f32_complex) * outputSize);

    memset(inverseInBuffer, 0x00, sizeof(f32_complex) * outputSize); // zero out intermediate buffer to make zero-padding quicker


    fftwPlannerMutex.lock(); // lock the FFTW Planner mutex, so that
    // Create FFT plan with 8 threads
    fftw_plan_with_nthreads(8);
    fftForward = fftwf_plan_dft_1d(
            inputSize, // number of fft bins
            (fftwf_complex*)inBuffer[0].buffer,
            (fftwf_complex*)forwardOutBuffer,
            FFTW_FORWARD,
            FFTW_PATIENT
    );
    fftInverse = fftwf_plan_dft_1d(
            outputSize, // number of fft bins
            (fftwf_complex*)inverseInBuffer,
            (fftwf_complex*)outBuffer[0].buffer,
            FFTW_BACKWARD,
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
    if(!inverseInBuffer || !forwardOutBuffer){
        PLOGE.printf("Created intermediate buffers failed");
        return 0;
    }
    if(!fftForward){
        PLOGE.printf("Created fftForwardPlan failed");
        return 0;
    }
    if(!fftInverse){
        PLOGE.printf("Created fftInversePlan failed");
        return 0;
    }
    return 1;
}

int HAIRCUT::resample::execute()  {
    PLOGD.printf("Object %016X: Execute interpolation", this);

    fftwf_execute(fftForward); // execute forward fft

    ///// these operations will either leave a zero-padded gap in uper frequency bins for interpolation, or clip upper frequency bins for decmation
    memcpy(inverseInBuffer, forwardOutBuffer, (inputSize/2) * sizeof(f32_complex)); // copy lower half of frequency bins, and copy to the lower half of output
    memcpy(inverseInBuffer + outputSize - (inputSize/2), forwardOutBuffer + inputSize - ( inputSize/2), (inputSize/2) * sizeof(f32_complex)); // grap the upper half of binsand copy to destination

    fftwf_execute(fftInverse); // execute inverse fft
    //memcpy(outBuffer[0].buffer, inverseInBuffer, outputSize * sizeof(f32_complex)); // debug show fft

    float scalingFactor = (outputSize/inputSize) / ( (float)outputSize); // calculate a scale factor that preserves magnitude after FFT and IFFT
    for(int i = 0; i < outputSize; ++i){
        outBuffer[0].buffer[i].real *= scalingFactor; // rescale outputs
        outBuffer[0].buffer[i].imag *= scalingFactor;
    }

    outBuffer[0].occupation += outputSize;
    inBuffer[0].occupation -= inputSize; // udate buffer occupation according to how much data the operaton serviced
    return 1;
}