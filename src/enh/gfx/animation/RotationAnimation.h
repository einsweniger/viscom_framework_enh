/**
 * @file   RotationAnimation.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.08.29
 *
 * @brief  Declaration of a rotation animation.
 */

#pragma once

#include "BaseAnimation.h"
#include "core/camera/Arcball.h"

#include <cereal/cereal.hpp>
#include <cereal/access.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/base_class.hpp>
#include "enh/core/serialization_helper.h"

#include <glm/gtc/quaternion.hpp>

namespace viscom {
    class CameraHelper;
}

namespace viscom::enh {

    class ApplicationNodeBase;
    class RotationAnimationEditor;

    class RotationAnimation : public BaseAnimation
    {
    public:
        using Editor = RotationAnimationEditor;

        RotationAnimation();
        RotationAnimation(const glm::quat& startOrientation, const glm::vec3& axis, float frequency);
        ~RotationAnimation();

        const glm::quat& GetStartOrientation() const { return startOrientation_; }
        void SetStartOrientation(const glm::quat& startOrientation) { startOrientation_ = startOrientation; }
        const glm::vec3& GetRotationAxis() const { return rotationAxis_; }
        void SetRotationAxis(const glm::vec3& rotationAxis) { rotationAxis_ = rotationAxis; }
        float GetFrequency() const { return frequency_; }
        void SetFrequency(float frequency) { frequency_ = frequency; }
        const glm::quat& GetCurrentState() const { return currentState_; }

        bool DoAnimationStep(float elapsedTime) override;
        void ShowEditDialog(const std::string& name);

    private:
        /** Needed for serialization */
        friend class cereal::access;

        template<class Archive>
        void save(Archive & ar, const std::uint32_t) const
        {
            ar(cereal::base_class<BaseAnimation>(this),
                cereal::make_nvp("startOrientation", startOrientation_),
                cereal::make_nvp("rotationAxis", rotationAxis_),
                cereal::make_nvp("frequency", frequency_));
        }

        template<class Archive>
        void load(Archive & ar, const std::uint32_t)
        {
            ar(cereal::base_class<BaseAnimation>(this),
                cereal::make_nvp("startOrientation", startOrientation_),
                cereal::make_nvp("rotationAxis", rotationAxis_),
                cereal::make_nvp("frequency", frequency_));
        }

        /** Holds the start orientation. */
        glm::quat startOrientation_;
        /** Holds the rotation axis. */
        glm::vec3 rotationAxis_;
        /** Holds the frequency. */
        float frequency_;
        /** Holds the current animation state. */
        glm::quat currentState_;
    };

    class RotationAnimationEditor
    {
    public:
        explicit RotationAnimationEditor(const CameraHelper* camera);

        void SetCurrentEdited(RotationAnimation* edit) { edit_ = edit; }
        bool HandleMouse(int button, int action, int mods, float mouseWheelDelta, ApplicationNodeBase* sender);
        bool HandleKeyboard(int key, int scancode, int action, int mods, ApplicationNodeBase* sender);
        void UpdateInput(double elapsedTime);

    private:
        /** Holds the edited object. */
        RotationAnimation* edit_ = nullptr;
        /** Holds the camera object. */
        const CameraHelper* camera_;
        /** Holds the arcball used for rotation of the axis. */
        Arcball axisArcball;
    };
}

CEREAL_CLASS_VERSION(viscom::enh::RotationAnimation, 1)
