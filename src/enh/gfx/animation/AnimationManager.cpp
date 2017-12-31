/**
 * @file   AnimationManager.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.08.25
 *
 * @brief  Implementation of the animation manager.
 */

#include "AnimationManager.h"
#include "enh/core/gui_helper.h"
#include <imgui.h>
#include <fstream>
#include <cereal/archives/xml.hpp>
#include <cereal/types/vector.hpp>

namespace viscom::enh {


    template <class T>
    TAnimationManager<T>::TAnimationManager(const std::string& dir, const CameraHelper* camera) :
        directory_(dir),
        editor_(camera)
    {
    }

    template <class T>
    TAnimationManager<T>::~TAnimationManager() = default;

    template <class T>
    unsigned TAnimationManager<T>::AddLastAnimation(const std::string& name)
    {
        auto id = static_cast<unsigned>(animations_.size() - 1);
        animationsByName_.insert(std::make_pair(name, std::make_pair(id, &animations_.back())));
        return id;
    }

    template <class T>
    void TAnimationManager<T>::StartAnimation()
    {
        for (auto& a : animations_) a.StartAnimation();
    }

    template <class T>
    void TAnimationManager<T>::StopAnimation()
    {
        for (auto& a : animations_) a.StopAnimation();
    }

    template <class T>
    bool TAnimationManager<T>::DoAnimationStep(float elapsedTime)
    {
        auto running = false;
        for (auto& a : animations_) if (a.DoAnimationStep(elapsedTime)) running = true;
        return running;
    }

    template <class T>
    void TAnimationManager<T>::ShowAnimationMenu(const std::string& name, bool showMenu, bool showEdit)
    {
        static auto showSelectEditAnimationPopup = false;
        static auto showLoadAllPopup = false;
        static auto showSaveAllPopup = false;
        static auto showSelectLoadPopup = false;
        static auto showSelectSavePopup = false;
        static auto showLoadSelectedPopup = false;
        static auto showSaveSelectedPopup = false;
        static auto selectedSet = 0;
        if (showMenu) {
            ImGui::MenuItem("Select Edited", nullptr, &showSelectEditAnimationPopup);
            ImGui::MenuItem("Load All", nullptr, &showLoadAllPopup);
            ImGui::MenuItem("Save All", nullptr, &showSaveAllPopup);
            ImGui::MenuItem(("Load " + name).c_str(), nullptr, &showSelectLoadPopup);
            ImGui::MenuItem(("Save " + name).c_str(), nullptr, &showSelectSavePopup);
        }

        if (showSelectEditAnimationPopup) ImGui::OpenPopup(("Select Edit " + name).c_str());
        if (ImGui::BeginPopupModal(("Select Edit " + name).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            auto rbChange = ImGui::RadioButton("None", &currentAnimation_, -1);
            for (const auto& set : animationsByName_) if (ImGui::RadioButton(set.first.c_str(), &currentAnimation_, static_cast<int>(set.second.first))) rbChange = true;
            if (rbChange) editor_.SetCurrentEdited(currentAnimation_ == -1 ? nullptr : &animations_[static_cast<std::size_t>(currentAnimation_)]);
            if (ImGui::Button("Close")) {
                ImGui::CloseCurrentPopup();
                showSelectEditAnimationPopup = false;
            }
            ImGui::EndPopup();
        }

        if (showSelectLoadPopup) ImGui::OpenPopup(("Select Load " + name).c_str());
        if (ImGui::BeginPopupModal(("Select Load " + name).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            for (const auto& set : animationsByName_) ImGui::RadioButton(set.first.c_str(), &selectedSet, static_cast<int>(set.second.first));
            if (ImGui::Button("Load")) {
                showLoadSelectedPopup = true;
                ImGui::CloseCurrentPopup();
                showSelectLoadPopup = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Close")) {
                ImGui::CloseCurrentPopup();
                showSelectLoadPopup = false;
                selectedSet = 0;
            }
            ImGui::EndPopup();
        }

        if (showSelectSavePopup) ImGui::OpenPopup(("Select Save " + name).c_str());
        if (ImGui::BeginPopupModal(("Select Save " + name).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            for (const auto& set : animationsByName_) ImGui::RadioButton(set.first.c_str(), &selectedSet, static_cast<int>(set.second.first));
            if (ImGui::Button("Edit")) {
                showSaveSelectedPopup = true;
                ImGui::CloseCurrentPopup();
                showSelectSavePopup = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Close")) {
                ImGui::CloseCurrentPopup();
                showSelectSavePopup = false;
                selectedSet = 0;
            }
            ImGui::EndPopup();
        }

        GuiHelper::DialogReturn dlgReturn;
        std::string fileName;
        std::tie(dlgReturn, fileName) = GuiHelper::OpenFileDialog(("Load All " + name).c_str(), showLoadAllPopup);
        if (dlgReturn == GuiHelper::DialogReturn::OK) LoadAll(fileName);

        std::tie(dlgReturn, fileName) = GuiHelper::OpenFileDialog(("Save All " + name).c_str(), showSaveAllPopup);
        if (dlgReturn == GuiHelper::DialogReturn::OK) SaveAll(fileName);

        if (showLoadSelectedPopup) {
            std::tie(dlgReturn, fileName) = GuiHelper::OpenFileDialog(("Load Selected " + name).c_str(), showLoadSelectedPopup);
            if (dlgReturn == GuiHelper::DialogReturn::OK) LoadAnimation(fileName, static_cast<std::size_t>(selectedSet));
            if (dlgReturn != GuiHelper::DialogReturn::NO_RETURN) selectedSet = 0;
        }

        if (showSaveSelectedPopup) {
            std::tie(dlgReturn, fileName) = GuiHelper::OpenFileDialog(("Save Selected" + name).c_str(), showSaveSelectedPopup);
            if (dlgReturn == GuiHelper::DialogReturn::OK) SaveAnimation(fileName, static_cast<std::size_t>(selectedSet));
            if (dlgReturn != GuiHelper::DialogReturn::NO_RETURN) selectedSet = 0;
        }

        if (!showMenu && showEdit && currentAnimation_ != -1) {
            animations_[static_cast<std::size_t>(currentAnimation_)].ShowEditDialog(names_[static_cast<std::size_t>(currentAnimation_)]);
        }
    }

    template <class T>
    void TAnimationManager<T>::LoadAnimation(const std::string& filename, std::size_t set)
    {
        std::ifstream wpFile(directory_ + "/" + filename, std::ios::in);
        if (wpFile.is_open()) {
            cereal::XMLInputArchive ia(wpFile);
            ia(cereal::make_nvp("animation", animations_[set]));
        }
    }

    template <class T>
    void TAnimationManager<T>::SaveAnimation(const std::string& filename, std::size_t set)
    {
        std::ofstream ofs(directory_ + "/" + filename, std::ios::out);

        cereal::XMLOutputArchive oa(ofs);
        oa(cereal::make_nvp("animation", animations_[set]));
    }

    template <class T>
    void TAnimationManager<T>::LoadAll(const std::string& filename)
    {
        std::ifstream ifs(directory_ + "/" + filename, std::ios::in);
        LoadAllFromStream(ifs);
    }

    template <class T>
    void TAnimationManager<T>::SaveAll(const std::string& filename)
    {
        std::ofstream ofs(directory_ + "/" + filename, std::ios::out);
        SaveAllFromStream(ofs);
    }

    template <class T>
    void TAnimationManager<T>::LoadAllFromStream(std::istream& stream)
    {
        animationsByName_.clear();
        animations_.clear();
        std::vector<std::string> animationNames;

        cereal::XMLInputArchive ia(stream);
        ia(cereal::make_nvp("animations", animations_),
            cereal::make_nvp("animationNames", animationNames));

        for (std::size_t i = 0; i < animations_.size(); ++i) {
            animationsByName_.insert(std::make_pair(animationNames[i], std::make_pair(i, &animations_[i])));
        }
    }

    template <class T>
    void TAnimationManager<T>::SaveAllFromStream(std::ostream& stream)
    {
        std::vector<std::string> animationNames(animations_.size());
        for (const auto& set : animationsByName_) animationNames[set.second.first] = set.first;

        cereal::XMLOutputArchive oa(stream);
        oa(cereal::make_nvp("animations", animations_),
            cereal::make_nvp("animationNames", animationNames));
    }

    AnimationManager::AnimationManager(const std::string& dir, const CameraHelper* camera) :
        wpAnimations_(dir, camera),
        rotAnimations_(dir, camera),
        orbAnimations_(dir, camera),
        directory_(dir)
    {
    }

    AnimationManager::~AnimationManager() = default;

    void AnimationManager::StartAnimation()
    {
        wpAnimations_.StartAnimation();
        rotAnimations_.StartAnimation();
        orbAnimations_.StartAnimation();
    }

    void AnimationManager::StopAnimation()
    {
        wpAnimations_.StopAnimation();
        rotAnimations_.StopAnimation();
        orbAnimations_.StopAnimation();
    }

    bool AnimationManager::DoAnimationStep(float elapsedTime)
    {
        auto running = false;
        if (wpAnimations_.DoAnimationStep(elapsedTime)) running = true;
        if (rotAnimations_.DoAnimationStep(elapsedTime)) running = true;
        if (orbAnimations_.DoAnimationStep(elapsedTime)) running = true;
        return running;
    }

    void AnimationManager::ShowAnimationMenu(const std::string& name, bool showMenu)
    {
        auto showWaypoints = false;
        auto showRotations = false;
        auto showOrbits = false;
        static auto showLoadAllPopup = false;
        static auto showSaveAllPopup = false;
        if (showMenu && ImGui::BeginMenu(name.c_str())) {
            if (ImGui::BeginMenu("Waypoints")) {
                currentAnimation_ = AnimationType::WAYPOINT;
                rotAnimations_.UnselectCurrent();
                orbAnimations_.UnselectCurrent();
                wpAnimations_.ShowAnimationMenu("Waypoints", true, true);
                showWaypoints = true;
                showRotations = false;
                showOrbits = false;
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Rotations")) {
                currentAnimation_ = AnimationType::ROTATION;
                wpAnimations_.UnselectCurrent();
                orbAnimations_.UnselectCurrent();
                rotAnimations_.ShowAnimationMenu("Rotations", true, true);
                showWaypoints = false;
                showRotations = true;
                showOrbits = false;
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Orbits")) {
                currentAnimation_ = AnimationType::ORBIT;
                wpAnimations_.UnselectCurrent();
                rotAnimations_.UnselectCurrent();
                orbAnimations_.ShowAnimationMenu("Orbits", true, true);
                showWaypoints = false;
                showRotations = false;
                showOrbits = true;
                ImGui::EndMenu();
            }

            ImGui::MenuItem("Load All", nullptr, &showLoadAllPopup);
            ImGui::MenuItem("Save All", nullptr, &showSaveAllPopup);
            ImGui::EndMenu();
        }

        if (!showWaypoints) wpAnimations_.ShowAnimationMenu("Waypoints", false, currentAnimation_ == AnimationType::WAYPOINT);
        if (!showRotations) rotAnimations_.ShowAnimationMenu("Rotations", false, currentAnimation_ == AnimationType::ROTATION);
        if (!showOrbits) orbAnimations_.ShowAnimationMenu("Orbits", false, currentAnimation_ == AnimationType::ORBIT);

        GuiHelper::DialogReturn dlgReturn;
        std::string fileName;
        std::tie(dlgReturn, fileName) = GuiHelper::OpenFileDialog("Load All Animations", showLoadAllPopup);
        if (dlgReturn == GuiHelper::DialogReturn::OK) LoadAnimations(fileName);

        std::tie(dlgReturn, fileName) = GuiHelper::OpenFileDialog("Save All Animations", showSaveAllPopup);
        if (dlgReturn == GuiHelper::DialogReturn::OK) SaveAnimations(fileName);
    }

    void AnimationManager::LoadAnimations(const std::string& fileName)
    {
        std::ifstream ifs(directory_ + "/" + fileName, std::ios::in);
        wpAnimations_.LoadAllFromStream(ifs);
        rotAnimations_.LoadAllFromStream(ifs);
        orbAnimations_.LoadAllFromStream(ifs);
    }

    void AnimationManager::SaveAnimations(const std::string& fileName)
    {
        std::ofstream ofs(directory_ + "/" + fileName, std::ios::out);
        wpAnimations_.SaveAllFromStream(ofs);
        rotAnimations_.SaveAllFromStream(ofs);
        orbAnimations_.SaveAllFromStream(ofs);
    }

    template unsigned int WPAnimationManager::AddLastAnimation(const std::string&);
    template unsigned int RotationAnimationManager::AddLastAnimation(const std::string&);
    template unsigned int OrbitAnimationManager::AddLastAnimation(const std::string&);
}
