/**
 * @file   PickHandler.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.06.14
 *
 * @brief  Definition of a generic pick handler.
 */

#pragma once

#include <memory>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

namespace viscom::enh {

    class ApplicationNodeBase;
    class GLBuffer;

    class PickHandler
    {
    public:
        PickHandler();
        virtual ~PickHandler();

        virtual bool HandleMouse(int button, int action, int mods, float mouseWheelDelta, ApplicationNodeBase* sender);
        virtual bool HandleKeyboard(int key, int scancode, int action, int mods, ApplicationNodeBase* sender);

        // unsigned int PickVertex(const ConnectivityMesh& mesh, const glm::mat4& world, const PerspectiveCamera& camera);
        const GLBuffer* GetPickIndexBuffer() const { return pickIBuffer_.get(); }
        unsigned int GetNumAdjacencyVertices() const { return numAdjVertices_; }
        void ResetPick() { numAdjVertices_ = 0; }

    private:
        /** Holds if pick mode is on. */
        bool pickMode_;
        /** Holds the mouse coordinates as integer. */
        glm::ivec2 iMouseCoords_;
        /** Holds the mouse coordinates. */
        glm::vec4 mouseCoords_;
        /** Holds the index buffer for the pick region. */
        std::unique_ptr<GLBuffer> pickIBuffer_;
        /** Holds the number of adjacency line vertices. */
        unsigned int numAdjVertices_;
        /** Holds whether mouse button is currently down. */
        bool mouseDown_;
    };
}
