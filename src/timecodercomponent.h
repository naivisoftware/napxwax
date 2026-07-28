/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

 // Local includes
#include "timecodernode.h"

// Nap includes
#include <nap/resourceptr.h>
#include <nap/timer.h>

// Audio includes
#include <audio/utility/safeptr.h>
#include <audio/component/audiocomponentbase.h>
#include <audio/resource/audiobufferresource.h>
#include <audio/node/inputnode.h>
#include <audio/node/gainnode.h>

namespace nap
{
    namespace audio
    {
        // Forward declares
        class TimecoderComponentInstance;

        class NAPAPI TimecoderComponent final : public AudioComponentBase
        {
            friend class TimecoderComponentInstance;
        RTTI_ENABLE(AudioComponentBase)
        DECLARE_COMPONENT(TimecoderComponent, TimecoderComponentInstance)
        public:
            // Properties
            nap::ComponentPtr<audio::AudioComponentBase> mInput;	///< property: 'Input' The component whose audio output will be send
			std::array<int, 2> mChannelRouting = { 0,1 };			///< property: 'ChannelRouting' Stereo input routing
            float mReferenceSpeed = 1.0f;							///< Property: 'ReferenceSpeed' The reference speed of the timecoder
            ETimecodeContol mControl = ETimecodeContol::SERATO_2A;	///< Property: 'Control' The vinyl control mode of the timecoder
			ETimecodeMode mMode = ETimecodeMode::DVS;				///< Property: 'Mode' The DVS interpretation mode by this component

            /**
             * Returns true if instance is set
             * @return true if instance is set
             */
            bool hasInstance() const;

            /**
             * Returns reference to instance, check with hasInstance() first. Asserts if instance is not set
             * @return reference to instance
             */
            TimecoderComponentInstance& getInstance() const;

        private:
            TimecoderComponentInstance* mInstance = nullptr;
        };


        class NAPAPI TimecoderComponentInstance final : public AudioComponentBaseInstance
        {
        RTTI_ENABLE(AudioComponentBaseInstance)
        public:

            TimecoderComponentInstance(EntityInstance& entity, Component& resource)
                : AudioComponentBaseInstance(entity, resource) { }

            ~TimecoderComponentInstance() override;

            /**
             * Initializes the instance, returns false on failure
             * @param errorState contains any error messages
             * @return false on failure
             */
            bool init(utility::ErrorState& errorState) override;

            /**
             * Called before deconstruction
             * Removes listener from VBANPacketReceiver
             */
            void onDestroy() override;

            /**
             * Returns amount of channels
             * @return amount of channels
             */
            int getChannelCount() const override { return 2; }

            /**
             * Returns output pin for given channel, no bound checking, assert on out of bound
             * @param channel the channel
             * @return OutputPin for channel
             */
			OutputPin* getOutputForChannel(int channel) override;

            /**
             * Updates the time and pitch with current timecode and pitch computed by TimecoderNode
             * @param deltaTime time in seconds since last update
             */
            void update(double deltaTime) override;

            /**
             * Returns the pitch computed by the TimecoderNode
             * @return pitch
             */
            double getPitch() const	{ return mPitch; }

            /**
             * Returns the absolute timecode in seconds computed by the TimecoderNode, will be -1.0 if no timecode is available or invalid
             * @return timecode
             */
            double getTimecode() const{ return mTimecode;}

            /**
             * Sets the control mode of the timecoder
             * @param control the control mode
             */
            void setControl(ETimecodeContol control);

			/**
			 * Sets the mode to use, passthrough or DVS
			 * When the mode is set to DVS the output gain is set to 0
			 */
			void setMode(ETimecodeMode mode);

            /**
             * Returns current control mode
             * @return current control mode
             */
            ETimecodeContol getControl() const;

            /**
             * Sets the reference speed of the timecoder, 1.0 is 33 1/3, 1.35 is 45 rpm
             * @param referenceSpeed
             */
            void setReferenceSpeed(float referenceSpeed);

            /**
             * Returns the reference speed of the timecoder
             * @return the reference speed of the timecoder
             */
            float getReferenceSpeed() const;

        private:
            ComponentInstancePtr<audio::AudioComponentBase> mInput	= { this, &TimecoderComponent::mInput };
            audio::SafeOwner<audio::TimecoderNode> mTimecoderNode = nullptr;
			std::array<audio::SafeOwner<audio::GainNode>, 2> mGainNodes = { nullptr };

            double mPitch = 0.0;
            double mTimecode = 0.0;
            float mReferenceSpeed = 1.0f;
            ETimecodeContol mControl = ETimecodeContol::SERATO_2A;
			ETimecodeMode mMode = ETimecodeMode::DVS;
			std::array<int, 2> mChannelRouting = { 0,1 };

			void createGraph();
        };
    }
}
