#pragma once

#include "timecodecontroltypes.h"

#include <audio/core/audionode.h>
#include <audio/service/audioservice.h>
#include <audio/utility/dirtyflag.h>
#include <concurrentqueue.h>

namespace nap
{
	namespace audio
	{
	    /**
	     * Timecoder node performs timecode analysis on node input.
	     * Input must be stereo audio.
	     * Output just passes through the input. This allows for buffer players to be controlled by analysis of the timecoder.
	     * Use the consumeTimeAndPitch method to get the current time and pitch from the main thread
	     * Use the getPitch and getTime methods to get the current pitch and time from the audio thread
	     */
	    class NAPAPI TimecoderNode final : public audio::Node
	    {
	        friend class TimecoderComponentInstance;
	    RTTI_ENABLE(audio::Process)
	    public:
	        /**
	         * Constructor
	         * @param nodeManager reference to nodeManager
	         */
	        TimecoderNode(NodeManager& nodeManager);
	
	        /**
	         * Constructor
	         * @param nodeManager reference to nodeManager
	         */
	        TimecoderNode(NodeManager& nodeManager, float referenceSpeed, ETimecodeContol control);
	
	        /**
	         * Destructor
	         */
	        ~TimecoderNode() override;
	
	        /**
	         * Pulls buffers from left and right input pins,
	         * Converts samples to 16 bit PCM and submits them to the timecoder.
	         * Computes pitch and time and sets the dirty flag.
	         */
	        void process() override;
	
	        /**
	         * Reinits the timecoder with the new sample rate.
	         * @param sampleRate the new samplerate
	         */
	        void sampleRateChanged(float sampleRate) override;
	
	        /**
	         * Sets time in seconds and pitch when the dirty flag is set, returns true if the dirty flag was set, false otherwise.
	         * @param time will be set to current time in seconds
	         * @param pitch will be set to current pitch
	         * @return true if the dirty flag was set, false otherwise. Given values will only be updated when dirty flag was set.
	         */
	        bool consumeTimeAndPitch(double &time, double &pitch, bool &timecodeValid);
	
	        /**
	         * Change control, re-initializes the timecoder.
	         * @param control control to change to
	         */
	        void changeControl(ETimecodeContol control);
	
			/**
	         * @return current control method
	         */
			ETimecodeContol getControl() const { return mControl; }
	
	        /**
	         * Change reference speed, re-initializes the timecoder.
	         * @param referenceSpeed the new reference speed
	         */
	        void changeReferenceSpeed(float referenceSpeed);
	
			/**
	         * @return reference speed, 1.0 = 33RPM, 1.35 = 45RPM
	         * @return the reference speed
	         */
	        float getReferenceSpeed() const { return mReferenceSpeed; }
	
	        /**
	         * Returns the current pitch, should only be called from audio thread, so either a node or another process attached to the audio thread
	         * @return the current pitch
	         */
	        double getPitch() const { return mPitch; }
	
	        /**
	         * Returns the current time in seconds, should only be called from audio thread, so either a node or another process attached to the audio thread
	         * @return the current time in seconds
	         */
	        double getTime() const { return mTime; }

	    	/**
        	 * Returns elapsed time in seconds when time code was read - audio thread only!
        	 * @return elapsed time in seconds when time code was read - audio thread only!
        	 */
        	double getDelta() const { return mDelta; }

	    	/**
	    	 * If the returned time code is considered safe to use - audio thread only!
	    	 * @return if the time code is considered safe to use - audio thread only!
	    	 */
	    	bool getSafe() const { return mSafe;}
	
			/**
			 * @return if current time code is valid
			 */
	        bool getCurrentTimecodeValid() const { return mCurrentTimecodeValid; }
	
	        // these input pins are connected by the TimecoderComponentInstance init method
	        InputPin audioInputLeft = { this };
	        InputPin audioInputRight = { this };

	        OutputPin audioOutputLeft = { this };
	    	OutputPin audioOutputRight = { this };

	    private:
	        /**
	         * Implementation in .cpp file
	         * Implementation wraps the timecoder struct from xwax
	         */
	        class Impl;
	        std::unique_ptr<Impl> mImpl;

	        double mDelta = 0.0;
	    	bool mSafe = false;

	        short mSamples[2] = { 0, 0 };
	        SampleBuffer* mBuffers[2] = {nullptr, nullptr};
	        std::atomic<double> mTime{0.0};
	        std::atomic<double> mPitch{0.0};
	        std::atomic_bool mCurrentTimecodeValid{false};
	        DirtyFlag mDirty;
	
	        // accessed only from update / main thread
	        double mConsumedPitch = 0.0f;
	        double mConsumedTime = 0.0;
	        bool mConsumedTimecodeValid = false;
	
	        float mReferenceSpeed = 1.0f;
	        ETimecodeContol mControl;
	
	        /**
	         * Creates a new timecoder, creation will be queued and executed in the process method.
	         */
	        void createTimecoder();
	        moodycamel::ConcurrentQueue<std::function<void()>> mTaskQueue;
	    };
	}
}
