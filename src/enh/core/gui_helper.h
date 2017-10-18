/**
 * @file   gui_helper.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.08.29
 *
 * @brief  Declaration of some GUI dialogs.
 */

#pragma once

#include <string>
#include <tuple>

namespace viscom::enh {

    class GuiHelper final
    {
    public:
        enum class DialogReturn
        {
            NO_RETURN,
            OK,
            CANCEL
        };
        static std::tuple<DialogReturn, std::string> OpenFileDialog(const std::string& name, bool& showFileDialog);
    };
}
