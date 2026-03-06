#pragma once

#include <nlohmann/json.hpp>

#include <string>

namespace drp::button_config
{

struct ButtonRule
{
    std::string matchCondition; // Format: "%key%=value" (e.g., "%album%=MyAlbum")
    std::string label;          // Button label
    std::string url;            // Button URL template (supports %artist%, %title%, etc.)

    ButtonRule() = default;

    ButtonRule( const std::string& condition, const std::string& lbl, const std::string& u )
        : matchCondition( condition )
        , label( lbl )
        , url( u )
    {
    }

    NLOHMANN_DEFINE_TYPE_INTRUSIVE( ButtonRule, matchCondition, label, url )
};

} // namespace drp::button_config
