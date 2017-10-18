/**
 * @file   PickHandler.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.06.14
 *
 * @brief  Implementation of the pick handler.
 */

#define GLM_FORCE_SWIZZLE

#include "PickHandler.h"
#include "enh/ApplicationNodeBase.h"
#include "enh/gfx/gl/GLBuffer.h"
#include "core/glfw.h"
#include <set>
#include <glm/gtc/matrix_inverse.hpp>

namespace viscom::enh {

    PickHandler::PickHandler() :
        pickMode_{false},
        iMouseCoords_{0, 0},
        mouseCoords_{0.0f},
        pickIBuffer_{ std::make_unique<GLBuffer>(gl::GL_DYNAMIC_DRAW) },
        numAdjVertices_{0},
        mouseDown_{false}
    {
        gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, pickIBuffer_->GetBuffer());
        std::array<unsigned int, 10> indices;
        std::fill(indices.begin(), indices.end(), 0);
        pickIBuffer_->InitializeData(indices);
        gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    PickHandler::~PickHandler() = default;

    bool PickHandler::HandleMouse(int button, int action, int mods, float mouseWheelDelta, ApplicationNodeBase* sender)
    {
        if (pickMode_ && button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE && mouseDown_) {
            mouseDown_ = false;
            return false;
        }

        if (pickMode_ && button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS && !mouseDown_) {
            mouseCoords_ = glm::vec4(sender->GetMousePosition(), 0.0f, 1.0f);
            mouseCoords_.y = 1.0f - mouseCoords_.y; // TODO: check if this is correct.
            iMouseCoords_.x = static_cast<int>(mouseCoords_.x);
            iMouseCoords_.y = static_cast<int>(mouseCoords_.y);
            mouseDown_ = true;
            return true;
        }
        return false;
    }

    bool PickHandler::HandleKeyboard(int key, int scancode, int action, int mods, ApplicationNodeBase* sender)
    {
        if (key == GLFW_KEY_P && action == GLFW_PRESS) {
            pickMode_ = !pickMode_;
            return true;
        }
        return false;
    }

    // TODO: this needs a new implementation. [10/18/2017 Sebastian Maisch]
    /*unsigned int PickHandler::PickVertex(const ConnectivityMesh& mesh, const glm::mat4& world, const PerspectiveCamera& camera)
    {
        gl::glReadPixels(iMouseCoords_.x, iMouseCoords_.y, 1, 1, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, &mouseCoords_.z);
        mouseCoords_.xyz = (mouseCoords_.xyz * 2.0f) - 1.0f;
        LOG(INFO) << "Picked position: (" << mouseCoords_.x << ", " << mouseCoords_.y << ", " << mouseCoords_.z << ")";
        auto projInv = glm::inverse(camera.GetProjMatrix());
        auto viewInv = glm::inverse(camera.GetViewMatrix());
        auto worldInv = glm::inverse(world);

        auto postProjPos = projInv * mouseCoords_;
        postProjPos /= postProjPos.w;
        auto pos = worldInv * viewInv * postProjPos;

        auto pickIdx = mesh.FindNearest(pos.xyz);
        auto pickIdxLocal = mesh.GetVertices()[pickIdx].locOnlyIdx;
        std::set<unsigned int> adjIdx;
        for (const auto& tri : mesh.GetVertices()[pickIdxLocal].triangles) {
            for (const auto idx : mesh.GetTriangle(tri).locOnlyVtxIds_) {
                if (idx != pickIdx && idx != pickIdxLocal) adjIdx.insert(idx);
            }
        }

        std::vector<unsigned int> iBufferData;
        for (const auto idx : adjIdx) {
            iBufferData.push_back(pickIdxLocal);
            iBufferData.push_back(idx);
        }

        numAdjVertices_ = static_cast<unsigned int>(iBufferData.size());
        pickIBuffer_->UploadData(0, iBufferData);
        return pickIdx;
    }*/
}
