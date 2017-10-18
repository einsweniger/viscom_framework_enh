/**
 * @file   WayPointAnimation.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.08.25
 *
 * @brief  Declaration of the way point animation object.
 */

#pragma once

#include "BaseAnimation.h"
#include <cereal/cereal.hpp>
#include <cereal/access.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/utility.hpp>
#include "enh/core/serialization_helper.h"

namespace viscom {
    class CameraHelper;
}

namespace viscom::enh {

    class ApplicationNodeBase;
    class WaypointAnimationEditor;

    struct WayPointInfo
    {
        glm::vec3 position_;

        template<class Archive> void serialize(Archive& ar, const std::uint32_t) {
            ar(cereal::make_nvp("position", position_));
        }
    };

    class WayPointAnimation : public BaseAnimation
    {
    public:
        using Editor = WaypointAnimationEditor;

        WayPointAnimation();
        WayPointAnimation(float totalTime, int interpolationMode, bool normalizeTime);
        ~WayPointAnimation();

        void AddWaypoint(const WayPointInfo& wp);
        void ResetWaypoints();

        float GetTotalTime() const { return totalTime_; }
        void SetTotalTime(float totalTime) { totalTime_ = totalTime; }
        int GetInterpolationMode() const { return interpolationMode_; }
        void SetInterpolationMode(int interpolationMode) { interpolationMode_ = interpolationMode; }
        bool DoNormalizeTime() const { return normalizeTime_; }
        void SetNormalizeTime(bool normalizeTime) { normalizeTime_ = normalizeTime; }
        const WayPointInfo& GetCurrentState() const { return currentState_; }

        void StartAnimation() override;
        bool DoAnimationStep(float elapsedTime) override;
        void ShowEditDialog(const std::string& name);

    private:
        /** Needed for serialization */
        friend class cereal::access;

        template<class Archive>
        void save(Archive & ar, const std::uint32_t) const
        {
            ar(cereal::base_class<BaseAnimation>(this),
                cereal::make_nvp("wayPoints", wayPoints_),
                cereal::make_nvp("totalTime", totalTime_),
                cereal::make_nvp("interpolationMode", interpolationMode_),
                cereal::make_nvp("normalizeTime", normalizeTime_));
        }

        template<class Archive>
        void load(Archive & ar, const std::uint32_t)
        {
            ar(cereal::base_class<BaseAnimation>(this),
                cereal::make_nvp("wayPoints", wayPoints_),
                cereal::make_nvp("totalTime", totalTime_),
                cereal::make_nvp("interpolationMode", interpolationMode_),
                cereal::make_nvp("normalizeTime", normalizeTime_));
        }

        /** Holds a list of way points. */
        std::vector<std::pair<WayPointInfo, float>> wayPoints_;

        /** Holds the total animation time. */
        float totalTime_;
        /** Holds the interpolation mode. */
        int interpolationMode_;
        /** Holds whether the time needs to be normalized. */
        bool normalizeTime_;
        /** Holds the current animation state. */
        WayPointInfo currentState_;
    };

    class WaypointAnimationEditor
    {
    public:
        explicit WaypointAnimationEditor(const CameraHelper* camera);

        void SetCurrentEdited(WayPointAnimation* edit) { edit_ = edit; }
        bool HandleMouse(int button, int action, int mods, float mouseWheelDelta, ApplicationNodeBase* sender);
        bool HandleKeyboard(int key, int scancode, int action, int mods, ApplicationNodeBase* sender);
        void UpdateInput(double elapsedTime);

    private:
        /** Holds the edited object. */
        WayPointAnimation* edit_ = nullptr;
    };
}

CEREAL_CLASS_VERSION(viscom::enh::WayPointInfo, 1)
CEREAL_CLASS_VERSION(viscom::enh::WayPointAnimation, 1)
