/**
 * @file   OrbitAnimation.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.08.29
 *
 * @brief  Declaration of the orbit animation object.
 */

#pragma once

#include "BaseAnimation.h"
#include "core/camera/Arcball.h"

#include <cereal/cereal.hpp>
#include <cereal/access.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/base_class.hpp>
#include "enh/core/serialization_helper.h"

namespace viscom {
    class CameraHelper;
}

namespace viscom::enh {

    class ApplicationNodeBase;
    class OrbitAnimationEditor;

    class OrbitAnimation : public BaseAnimation
    {
    public:
        using Editor = OrbitAnimationEditor;

        OrbitAnimation();
        OrbitAnimation(const glm::vec3& startPosition, const glm::vec3& axis, float frequency);
        ~OrbitAnimation();

        const glm::vec3& GetStartPosition() const { return startPosition_; }
        void SetStartPosition(const glm::vec3& startPosition) { startPosition_ = startPosition; }
        const glm::vec3& GetRotationAxis() const { return rotationAxis_; }
        void SetRotationAxis(const glm::vec3& rotationAxis) { rotationAxis_ = rotationAxis; }
        float GetFrequency() const { return frequency_; }
        void SetFrequency(float frequency) { frequency_ = frequency; }
        const glm::vec3& GetCurrentState() const { return currentState_; }

        bool DoAnimationStep(float elapsedTime) override;
        void ShowEditDialog(const std::string& name);

    private:
        /** Needed for serialization */
        friend class cereal::access;

        template<class Archive>
        void save(Archive & ar, const std::uint32_t) const
        {
            ar(cereal::base_class<BaseAnimation>(this),
                cereal::make_nvp("startPosition", startPosition_),
                cereal::make_nvp("rotationAxis", rotationAxis_),
                cereal::make_nvp("frequency", frequency_));
        }

        template<class Archive>
        void load(Archive & ar, const std::uint32_t)
        {
            ar(cereal::base_class<BaseAnimation>(this),
                cereal::make_nvp("startPosition", startPosition_),
                cereal::make_nvp("rotationAxis", rotationAxis_),
                cereal::make_nvp("frequency", frequency_));
        }

        /** Holds the start position. */
        glm::vec3 startPosition_;
        /** Holds the rotation axis. */
        glm::vec3 rotationAxis_;
        /** Holds the frequency. */
        float frequency_;
        /** Holds the current animation state. */
        glm::vec3 currentState_;
    };

    class OrbitAnimationEditor
    {
    public:
        explicit OrbitAnimationEditor(const CameraHelper* camera);

        void SetCurrentEdited(OrbitAnimation* edit) { edit_ = edit; }
        bool HandleMouse(int button, int action, int mods, float mouseWheelDelta, ApplicationNodeBase* sender);
        bool HandleKeyboard(int key, int scancode, int action, int mods, ApplicationNodeBase* sender);
        void UpdateInput(double elapsedTime);

    private:
        /** Holds the edited object. */
        OrbitAnimation* edit_ = nullptr;
        /** Holds the camera object. */
        const CameraHelper* camera_;
        /** Holds the arcball used for rotation of the axis. */
        Arcball axisArcball;
    };
}

CEREAL_CLASS_VERSION(viscom::enh::OrbitAnimation, 1)
