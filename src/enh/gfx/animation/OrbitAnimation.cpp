/**
 * @file   OrbitAnimation.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.08.29
 *
 * @brief  Implementation of the orbit animation object.
 */

#include "OrbitAnimation.h"
#include "enh/ApplicationNodeBase.h"
#include "core/glfw.h"
#include <imgui.h>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>

namespace viscom::enh {

    OrbitAnimation::OrbitAnimation() :
        startPosition_(0.0f),
        rotationAxis_(0.0f, 1.0f, 0.0f),
        frequency_(0.0f)
    {
    }

    OrbitAnimation::OrbitAnimation(const glm::vec3& startPosition, const glm::vec3& axis, float frequency) :
        startPosition_(startPosition),
        rotationAxis_(axis),
        frequency_(frequency)
    {
    }

    OrbitAnimation::~OrbitAnimation() = default;

    bool OrbitAnimation::DoAnimationStep(float elapsedTime)
    {
        if (!BaseAnimation::DoAnimationStep(elapsedTime)) return false;
        if (frequency_ < 0.00001f) {
            StopAnimation();
            return false;
        }
        currentState_ = glm::rotate(startPosition_, glm::two_pi<float>() * (GetCurrentTime() / frequency_), rotationAxis_);
        return true;
    }

    void OrbitAnimation::ShowEditDialog(const std::string& name)
    {
        const auto winWidth = 250.0f;
        const auto winHeight = 80.0f;
        ImGui::SetNextWindowSize(ImVec2(winWidth, winHeight), ImGuiSetCond_Always);
        ImGui::SetNextWindowPos(ImVec2(10.0f, ImGui::GetIO().DisplaySize.y - winHeight - 10.0f), ImGuiSetCond_Always);
        ImGui::Begin(("Orbit Animation (" + name + ")").c_str());
        ImGui::InputFloat("Frequency", &frequency_);
        ImGui::End();
    }

    OrbitAnimationEditor::OrbitAnimationEditor(const CameraHelper* camera) :
        camera_{ camera },
        axisArcball{ GLFW_MOUSE_BUTTON_LEFT }
    {
    }

    bool OrbitAnimationEditor::HandleMouse(int button, int action, int mods, float, ApplicationNodeBase* sender)
    {
        if (edit_ != nullptr) {
            return axisArcball.HandleMouse(button, action, sender);
        }
        return false;
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    // ReSharper disable once CppMemberFunctionMayBeConst
    bool OrbitAnimationEditor::HandleKeyboard(int, int, int, int, ApplicationNodeBase*)
    {
        return false;
    }

    void OrbitAnimationEditor::UpdateInput(double elapsedTime)
    {
        auto orientation = glm::rotation(glm::vec3(0.0f, 1.0f, 0.0f), edit_->GetRotationAxis());
        auto orient = glm::inverse(axisArcball.GetWorldRotation(elapsedTime, glm::inverse(camera_->GetOrientation()))) * orientation;
        edit_->SetRotationAxis(glm::rotate(orient, glm::vec3(0.0f, 1.0f, 0.0f)));
    }
}
