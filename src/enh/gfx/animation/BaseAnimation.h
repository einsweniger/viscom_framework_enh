/**
 * @file   BaseAnimation.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.08.29
 *
 * @brief  Declaration of the base class for animations.
 */

#pragma once

#include <cereal/cereal.hpp>
#include <cereal/access.hpp>

namespace viscom::enh {

    class BaseAnimation
    {
    public:
        BaseAnimation();
        virtual ~BaseAnimation();

        virtual void StartAnimation();
        void StopAnimation() { animationRunning_ = false; }
        virtual bool DoAnimationStep(float elapsedTime);

        bool IsAnimationRunning() const { return animationRunning_; }

    protected:
        float GetCurrentTime() const { return currentAnimationTime_; }

    private:
        /** Needed for serialization */
        friend class cereal::access;

        template<class Archive>
        void save(Archive & ar, const std::uint32_t) const {}

        template<class Archive>
        void load(Archive & ar, const std::uint32_t)
        {
            animationRunning_ = false;
            currentAnimationTime_ = 0.0f;
        }

        /** Hold whether the animation is running. */
        bool animationRunning_ = false;
        /** Holds the current animation time. */
        float currentAnimationTime_ = 0.0f;
    };
}

CEREAL_CLASS_VERSION(viscom::enh::BaseAnimation, 1)
