/**
 * @file   SimpleMeshRenderer.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.08.26
 *
 * @brief  Implementation of the SimpleMeshRenderer class.
 */

#include "SimpleMeshRenderer.h"
#include "core/gfx/mesh/Mesh.h"
#include "enh/ApplicationNodeBase.h"
#include "enh/gfx/gl/GLVertexAttributeArray.h"
#include "enh/gfx/gl/GLBuffer.h"

#include <glm/gtc/type_ptr.hpp>

namespace viscom::enh {

    struct SimpleVertex {

        glm::vec4 pos = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

        static void GatherAttributeNames(std::vector<std::string>& attribNames)
        {
            attribNames.emplace_back("position");
        }

        static void VertexAttributeSetup(GLVertexAttributeArray* vao, const std::vector<gl::GLint>& shaderPositions)
        {
            vao->StartAttributeSetup();
            if (shaderPositions[0] >= 0) vao->AddVertexAttribute(shaderPositions[0], 4, gl::GL_FLOAT, gl::GL_FALSE, sizeof(SimpleVertex), offsetof(SimpleVertex, pos)); //-V112
            vao->EndAttributeSetup();
        }

        SimpleVertex() = default;
        explicit SimpleVertex(const glm::vec4& position) : pos{ position } {}
    };

    SimpleMeshRenderer::SimpleMeshRenderer(ApplicationNodeBase* app) :
        simpleProgram_(app->GetGPUProgramManager().GetResource("drawSimple", std::vector<std::string>{"drawSimple.vert", "drawSimple.frag"}))
    {
        std::vector<SimpleVertex> vertices;
        std::vector<unsigned int> indices;
        std::array<std::string, 6> submeshNames = { "mesh_cone", "mesh_cube", "mesh_cylinder", "mesh_octahedron", "mesh_sphere", "mesh_torus" };
        for (unsigned int i = 0; i < 6; ++i) {
            auto mesh = app->GetMeshManager().GetResource("/models/simple/" + submeshNames[i] + ".obj");
            std::vector<SimpleVertex> meshVertices;
            for (const auto& vtx : mesh->GetVertices()) meshVertices.emplace_back(glm::vec4(vtx, 1.0f));

            submeshInfo_[i].first = static_cast<unsigned>(indices.size());
            submeshInfo_[i].second = static_cast<unsigned>(mesh->GetIndices().size());

            auto verticesMin = static_cast<unsigned>(vertices.size());
            vertices.reserve(vertices.size() + meshVertices.size());
            vertices.insert(vertices.end(), meshVertices.begin(), meshVertices.end());

            indices.reserve(indices.size() + mesh->GetIndices().size());
            for (unsigned int j : mesh->GetIndices()) {
                indices.push_back(j + verticesMin);
            }
        }

        submeshInfo_[6].first = static_cast<unsigned>(indices.size());
        submeshInfo_[6].second = 1;
        submeshInfo_[7].first = static_cast<unsigned>(indices.size());
        submeshInfo_[7].second = 2;
        indices.push_back(static_cast<unsigned>(vertices.size()));
        indices.push_back(static_cast<unsigned>(vertices.size() + 1));
        vertices.emplace_back(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        vertices.emplace_back(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

        vBuffer_ = std::make_unique<GLBuffer>(gl::GL_STATIC_DRAW);
        gl::glBindBuffer(gl::GL_ARRAY_BUFFER, vBuffer_->GetBuffer());
        vBuffer_->InitializeData(vertices);
        gl::glBindBuffer(gl::GL_ARRAY_BUFFER, 0);

        iBuffer_ = std::make_unique<GLBuffer>(gl::GL_STATIC_DRAW);
        gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, iBuffer_->GetBuffer());
        iBuffer_->InitializeData(indices);
        gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, 0);

        std::vector<std::string> attributeNames;
        SimpleVertex::GatherAttributeNames(attributeNames);

        auto shaderPositions = simpleProgram_->GetAttributeLocations(attributeNames);

        gl::glBindBuffer(gl::GL_ARRAY_BUFFER, vBuffer_->GetBuffer());

        drawAttribBinds_.GetVertexAttributes().push_back(std::make_unique<GLVertexAttributeArray>(vBuffer_->GetBuffer(), iBuffer_->GetBuffer()));
        SimpleVertex::VertexAttributeSetup(drawAttribBinds_.GetVertexAttributes().back().get(), shaderPositions);
        gl::glBindBuffer(gl::GL_ARRAY_BUFFER, 0);

        drawAttribBinds_.GetUniformIds() = simpleProgram_->GetUniformLocations({ "vpMatrix", "modelMatrix", "color", "pointSize" });
    }

    SimpleMeshRenderer::~SimpleMeshRenderer() = default;

    void SimpleMeshRenderer::DrawCone(const glm::mat4& VPMatrix, const glm::mat4& modelMatrix, const glm::vec4& color) const
    {
        DrawSubmesh(VPMatrix, modelMatrix, color, 0);
    }

    void SimpleMeshRenderer::DrawCube(const glm::mat4& VPMatrix, const glm::mat4& modelMatrix, const glm::vec4& color) const
    {
        DrawSubmesh(VPMatrix, modelMatrix, color, 1);
    }

    void SimpleMeshRenderer::DrawCylinder(const glm::mat4& VPMatrix, const glm::mat4& modelMatrix, const glm::vec4& color) const
    {
        DrawSubmesh(VPMatrix, modelMatrix, color, 2);
    }

    void SimpleMeshRenderer::DrawOctahedron(const glm::mat4& VPMatrix, const glm::mat4& modelMatrix, const glm::vec4& color) const
    {
        DrawSubmesh(VPMatrix, modelMatrix, color, 3);
    }

    void SimpleMeshRenderer::DrawSphere(const glm::mat4& VPMatrix, const glm::mat4& modelMatrix, const glm::vec4& color) const
    {
        DrawSubmesh(VPMatrix, modelMatrix, color, 4); //-V112
    }

    void SimpleMeshRenderer::DrawTorus(const glm::mat4& VPMatrix, const glm::mat4& modelMatrix, const glm::vec4& color) const
    {
        DrawSubmesh(VPMatrix, modelMatrix, color, 5);
    }

    void SimpleMeshRenderer::DrawPoint(const glm::mat4& VPMatrix, const glm::mat4& modelMatrix, const glm::vec4& color, float pointSize) const
    {
        DrawSubmesh(VPMatrix, modelMatrix, color, 6, pointSize);
    }

    void SimpleMeshRenderer::DrawLine(const glm::mat4& VPMatrix, const glm::mat4& modelMatrix, const glm::vec4& color) const
    {
        DrawSubmesh(VPMatrix, modelMatrix, color, 7);
    }

    void SimpleMeshRenderer::DrawSubmesh(const glm::mat4& VPMatrix, const glm::mat4& modelMatrix, const glm::vec4& color, unsigned int submeshId, float pointSize) const
    {
        gl::glUseProgram(simpleProgram_->getProgramId());
        gl::glUniformMatrix4fv(drawAttribBinds_.GetUniformIds()[0], 1, gl::GL_FALSE, glm::value_ptr(VPMatrix));
        gl::glUniformMatrix4fv(drawAttribBinds_.GetUniformIds()[1], 1, gl::GL_FALSE, glm::value_ptr(modelMatrix));
        gl::glUniform4fv(drawAttribBinds_.GetUniformIds()[2], 1, glm::value_ptr(color));
        gl::glUniform1f(drawAttribBinds_.GetUniformIds()[3], pointSize);

        gl::glBindBuffer(gl::GL_ARRAY_BUFFER, vBuffer_->GetBuffer());
        drawAttribBinds_.GetVertexAttributes()[0]->EnableVertexAttributeArray();

        auto primitiveType = gl::GL_TRIANGLES;
        if (submeshId == 6) primitiveType = gl::GL_POINTS;
        if (submeshId == 7) primitiveType = gl::GL_LINES;
        glDrawElements(primitiveType, submeshInfo_[submeshId].second, gl::GL_UNSIGNED_INT,
            (static_cast<char*> (nullptr)) + (submeshInfo_[submeshId].first * sizeof(unsigned int))); //-V104

        drawAttribBinds_.GetVertexAttributes()[0]->DisableVertexAttributeArray();
        gl::glUseProgram(0);
    }
}
