/*

*/


#ifndef HAIRCUT_DEF // prevent recursive inclusion
#define HAIRCUT_DEF

#include <string>
#include "plog/Log.h"
#include "plog/Initializers/RollingFileInitializer.h"
#include "plog/Appenders/ConsoleAppender.h"
#include "fftw3.h"
#include <thread>
#include <mutex>

using namespace std; // default namespace

class HAIRCUT;

static mutex fftwPlannerMutex; // FFTW planner is not threadsafe by design. This mutex allows only one running instance of FFTW_plan

typedef struct f32_complex{
    volatile float real;
    volatile float imag;
} f32_complex;

class HAIRCUT{
    class BlockInterface;

    typedef struct sampleBuffer{
        f32_complex* buffer;
        int occupation;
        int size;
        vector<BlockInterface*> connection;  // pointer to either upstream or downstream sink/source
    } sampleBuffer_t;

    class BlockInterface {
    public:
        int inChannels = 1;
        int outChannels = 1;
        sampleBuffer_t* inBuffer;
        sampleBuffer_t* outBuffer;
    public:
        int initInterface(int inSamples, int outSamples);

        /// Function to input samples to the DSP block
        /// \param src pointer to complex float samples
        /// \param count number of samples to give
        /// \return number of samples actually taken in
        int push(f32_complex* src, int count, int channel = 0);

        /// Function to get samples from the DSP block
        /// \param dest pointer to a complex float sample buffer
        /// \param count number of samples requested
        /// \return number of samples actually transferred
        int pull(f32_complex* dest, int count, int channel = 0);

        /// Gets number of samples available in the output buffer
        inline int getNumSamplesAvailable(int channel = 0){return outBuffer[channel].occupation;};
        /// Gets number of samples available in the input buffer
        inline int getBufferSpaceAvailable(int channel = 0){return inBuffer[channel].size - inBuffer[channel].occupation;};

//        /// If size of vector is greater than zero and this DSP block requires multiple input streams, samples for this block will be obtained by calling sourceBlock->output() for each dsp block pointer in the list
//        vector<BlockInterface*> sourceBlock;
//        /// If size of vector is greater than zero, samples will be outputted from this block by calling destinationBlock->input() for each dsp block pointer in the list
//        vector<BlockInterface*> destinationBlock;

        virtual int execute() = 0;
    };





public:
    /// Library init function. Input a PLOG appender to allow the library to log through the parent executable.
    /// \param severity The minimum severity to report. ex: plog::info
    /// \return pointer to a plog appender
    bool init(plog::IAppender* appender, plog::Severity severity = plog::debug);

    class FFT : public BlockInterface{
    private:
        int fftSize;
        fftwf_plan fftPlan;
        int execute() override;
        //int inChannels = 2;
    public:
        /// Initialize this DSP block as an FFT block.
        /// \param size Fourrier transform size
        /// \param batchSize number of batches to operate on at a time. A seperate thread is created for each batch, so multiple batches is advantageous for multithreading
        /// \param inverse If true, an inverse fft will be performed
        int init(int size, int batchSize = 1, bool inverse = false);
    };

    class resample : public BlockInterface{
    private:
        int inputSize;
        int outputSize;

        fftwf_plan fftForward;
        f32_complex* forwardOutBuffer;
        f32_complex* inverseInBuffer;
        fftwf_plan fftInverse;

        int execute() override;
        //int inChannels = 2;
    public:

        int init(int inputSize, int outputSize, int batchSize = 1);
    };

    class callbackSink : public BlockInterface{
    private:
        void (*callback)(f32_complex*, int count);
        int execute() override;
        int sampleCount;
    public:

        int init(void (*callback)(f32_complex*, int count), int sampleCount_i);
    };
};



#endif
