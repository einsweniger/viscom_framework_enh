/**
 * @file   WayPointAnimation.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.08.25
 *
 * @brief  Implementation of the way point animation object.
 */

#include "WayPointAnimation.h"
#include "glm/gtx/spline.hpp"
#include <imgui.h>

namespace viscom::enh {

    WayPointAnimation::WayPointAnimation() :
        totalTime_(0.0f),
        interpolationMode_(0),
        normalizeTime_(false)
    {
    }

    WayPointAnimation::WayPointAnimation(float totalTime, int interpolationMode, bool normalizeTime) :
        totalTime_(totalTime),
        interpolationMode_(interpolationMode),
        normalizeTime_(normalizeTime)
    {
    }

    WayPointAnimation::~WayPointAnimation() = default;

    void WayPointAnimation::AddWaypoint(const WayPointInfo& wp)
    {
        wayPoints_.push_back(std::make_pair(wp, 0.0f));
    }

    void WayPointAnimation::ResetWaypoints()
    {
        wayPoints_.clear();
    }

    void WayPointAnimation::StartAnimation()
    {
        BaseAnimation::StartAnimation();
        if (wayPoints_.size() < 2) {
            StopAnimation();
            return;
        }
        std::vector<float> distances(wayPoints_.size(), totalTime_ / static_cast<float>(wayPoints_.size() - 1));
        distances[0] = 0.0f;
        if (normalizeTime_) {
            auto totalDistances = 0.0f;
            for (std::size_t i = 1; i < distances.size(); ++i) {
                distances[i] = glm::distance(wayPoints_[i - 1].first.position_, wayPoints_[i].first.position_);
                totalDistances += distances[i];
            }

            for (std::size_t i = 1; i < distances.size(); ++i) distances[i] = distances[i] * totalTime_ / totalDistances;
        }

        wayPoints_[0].second = 0.0f;
        for (std::size_t i = 1; i < distances.size(); ++i) wayPoints_[i].second = wayPoints_[i - 1].second + distances[i];
    }

    bool WayPointAnimation::DoAnimationStep(float elapsedTime)
    {
        if (!BaseAnimation::DoAnimationStep(elapsedTime)) return false;
        if (totalTime_ < 0.00001f) {
            StopAnimation();
            return false;
        }

        std::size_t cI = 1;
        for (; cI < wayPoints_.size(); ++cI) if (wayPoints_[cI].second > GetCurrentTime()) break;

        if (cI >= wayPoints_.size()) {
            cI = wayPoints_.size() - 1;
            StopAnimation();
        }
        auto alpha = (GetCurrentTime() - wayPoints_[cI - 1].second) / (wayPoints_[cI].second - wayPoints_[cI - 1].second);
        auto& wpCI = wayPoints_[cI].first;
        auto& wpCIn1 = wayPoints_[cI - 1].first;
        switch (interpolationMode_) {
        case 1: {
            auto v0 = (cI == 1) ? 2.0f * wpCIn1.position_ - wpCI.position_ : wayPoints_[cI - 2].first.position_;
            auto v4 = (cI == wayPoints_.size() - 1) ? 2.0f * wpCI.position_ - wpCIn1.position_ : wayPoints_[cI + 1].first.position_;
            currentState_.position_ = glm::catmullRom(v0, wpCIn1.position_, wpCI.position_, v4, alpha);
        } break;
        case 2: {
            auto v0 = (cI == 1) ? 2.0f * wpCIn1.position_ - wpCI.position_ : wayPoints_[cI - 2].first.position_;
            auto v4 = (cI == wayPoints_.size() - 1) ? 2.0f * wpCI.position_ - wpCIn1.position_ : wayPoints_[cI + 1].first.position_;
            currentState_.position_ = glm::cubic(v0, wpCIn1.position_, wpCI.position_, v4, alpha);
        } break;
        case 3: {
            auto t0 = (cI == 1) ? glm::normalize(wpCI.position_ - wpCIn1.position_)
                : glm::normalize(wpCI.position_ - wayPoints_[cI - 2].first.position_);
            auto t1 = (cI == wayPoints_.size() - 1) ? glm::normalize(wpCI.position_ - wpCIn1.position_)
                : glm::normalize(wayPoints_[cI + 1].first.position_ - wpCIn1.position_);
            currentState_.position_ = glm::hermite(wpCIn1.position_, t0, wpCI.position_, t1, alpha);
        } break;
        default: {
            currentState_.position_ = glm::mix(wayPoints_[cI - 1].first.position_, wpCI.position_, alpha);
        } break;
        }


        if (GetCurrentTime() > wayPoints_.back().second) StopAnimation();
        return true;
    }

    void WayPointAnimation::ShowEditDialog(const std::string& name)
    {
        const auto winWidth = 250.0f;
        const auto winHeight = 170.0f;
        ImGui::SetNextWindowSize(ImVec2(winWidth, winHeight), ImGuiSetCond_Always);
        ImGui::SetNextWindowPos(ImVec2(10.0f, ImGui::GetIO().DisplaySize.y - winHeight - 10.0f), ImGuiSetCond_Always);
        ImGui::Begin(("Wayppoint Animation (" + name + ")").c_str());
        ImGui::InputFloat("Total Time", &totalTime_);
        ImGui::RadioButton("Linear Interpolation", &interpolationMode_, 0);
        ImGui::RadioButton("Catmull-Rom Interpolation", &interpolationMode_, 1);
        ImGui::RadioButton("Cubic Interpolation", &interpolationMode_, 2);
        ImGui::RadioButton("Hermite Interpolation", &interpolationMode_, 3);
        ImGui::Checkbox("Normalize Time", &normalizeTime_);
        ImGui::End();
    }

    WaypointAnimationEditor::WaypointAnimationEditor(const CameraHelper*)
    {
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    // ReSharper disable once CppMemberFunctionMayBeConst
    bool WaypointAnimationEditor::HandleMouse(int, int, int, float, ApplicationNodeBase*)
    {
        return false;
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    // ReSharper disable once CppMemberFunctionMayBeConst
    bool WaypointAnimationEditor::HandleKeyboard(int key, int, int action, int, ApplicationNodeBase*)
    {
        return false;
    }

    void WaypointAnimationEditor::UpdateInput(double)
    {
    }
}
