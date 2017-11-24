//
// Created by Don Goodman-Wilson on 13/11/2017.
//

#include <regex>
#include "card.h"
#include <json.hpp>

namespace magique
{

std::vector<card::color> card::color_identities_from_array(const std::vector<std::string> arr)
{
    std::vector<color> colors{};
    for (auto color : arr)
    {
        std::transform(color.begin(), color.end(), color.begin(), ::tolower);

        if (color == "w")
        { colors.push_back(color::white); }
        else if (color == "u")
        { colors.push_back(color::blue); }
        else if (color == "b")
        { colors.push_back(color::black); }
        else if (color == "r")
        { colors.push_back(color::red); }
        else if (color == "g")
        { colors.push_back(color::green); }
        else if (color == "u") colors.push_back(color::colorless);
    }

    return colors;
}

std::vector<card::type>
card::types_from_array(const std::vector<std::string> &types, const std::vector<std::string> &supertypes)
{
    std::vector<type> types_arr{};

    for (const auto &type : types)
    {
        std::string type_str = type;
        std::transform(type_str.begin(), type_str.end(), type_str.begin(), ::tolower);

        if (type_str == "land")
        {
            bool basic_land{false};
            for (const auto &supertype : supertypes)
            {
                std::string supertype_str = supertype;
                std::transform(supertype_str.begin(), supertype_str.end(), supertype_str.begin(), ::tolower);

                if (supertype == "basic")
                {
                    basic_land = true;
                    break;
                }
            }
            if (basic_land)
            {
                types_arr.push_back(type::basic_land);
            }
            else
            {
                types_arr.push_back(type::land);
            }
        }
        else if (type_str == "creature")
        { types_arr.push_back(type::creature); }
        else if (type_str == "artifact")
        { types_arr.push_back(type::artifact); }
        else if (type_str == "enchantment")
        { types_arr.push_back(type::enchantment); }
        else if (type_str == "planeswalker")
        { types_arr.push_back(type::planeswalker); }
        else if (type_str == "instant")
        { types_arr.push_back(type::instant); }
        else if (type_str == "sorcery") types_arr.push_back(type::sorcery);

    }


    return types_arr;
}

void from_json(const nlohmann::json &j, card &p)
{
    p.name = j.at("name").get<std::string>();

    std::vector<std::string> supertypes;
    try
    {
        if (j.at("supertypes").is_array())
        {
            supertypes = j.at("supertypes").get<std::vector<std::string>>();
        }
    }
    catch (std::out_of_range e)
    {}

    try
    {
        p.types = card::types_from_array(j.at("types").get<std::vector<std::string>>(), supertypes);
    }
    catch (std::out_of_range e)
    {}

    try
    {
        if (j.at("subtypes").is_array())
        {
            p.subtypes = j.at("subtypes").get<std::vector<std::string>>();
        }
    }
    catch (std::out_of_range e)
    {}


    try
    {
        p.color_identity = card::color_identities_from_array(j.at("colorIdentity").get<std::vector<std::string>>());
    }
    catch (std::out_of_range e)
    {}

    try
    {
        p.text = j.at("text").get<std::string>();
    }
    catch (std::out_of_range e)
    {}


    try
    {
        p.power = j.at("power").get<std::string>();
    }
    catch (std::out_of_range e)
    {}


    try
    {
        p.toughness = j.at("toughness").get<std::string>();
    }
    catch (std::out_of_range e)
    {}

    p.converted_mana_cost = j.at("cmc").get<uint64_t>();

    //TODO first pass at finding interesting mechanics. In fact, this should be handled externally by something smarter

    // TODO interactions should be cumulative—the more of one kind you have, the more each gives points. We can improve on this algorithm.

    // TOPO case-insensitive search?

    // flying, reach
    if (p.text.find("Flying") != std::string::npos || p.text.find("Reach") != std::string::npos)
    {
        p.mechanics.insert("flying");
    }

    if (p.text.find("token") != std::string::npos)
    {
        p.mechanics.insert("token");
    }

    if (p.text.find("draw") != std::string::npos)
    {
        p.mechanics.insert("draw");
    }

    std::smatch sm;
    if(std::regex_search(p.text, sm, std::regex{R"(\+[0-9]/\+[0-9])"}) )
    {
        p.mechanics.insert("buff");
    }

    if (p.text.find("{E}") != std::string::npos)
    {
        p.mechanics.insert("energy");
    }

    if(std::regex_search(p.text, sm, std::regex{R"(\-[0-9]/\-[0-9])"}) )
    {
        p.mechanics.insert("debuff");
    }
}

std::string to_string(card::type t)
{
    switch (t)
    {
        case card::type::basic_land:
            return "basic land";
        case card::type::land:
            return "land";
        case card::type::creature:
            return "creature";
        case card::type::artifact:
            return "artifact";
        case card::type::enchantment:
            return "enchantment";
        case card::type::planeswalker:
            return "planeswalker";
        case card::type::instant:
            return "instant";
        case card::type::sorcery:
            return "sorcery";
    }
}

void to_json(nlohmann::json &j, const card &p)
{
    std::vector<std::string> color_identities;

    for (const auto &c : p.color_identity)
    {
        switch (c)
        {
            case card::color::white:
                color_identities.emplace_back("White");
                break;
            case card::color::blue:
                color_identities.emplace_back("Blue");
                break;
            case card::color::black:
                color_identities.emplace_back("Black");
                break;
            case card::color::red:
                color_identities.emplace_back("Red");
                break;
            case card::color::green:
                color_identities.emplace_back("Green");
                break;
            case card::color::colorless:
                color_identities.emplace_back("Colorless");
                break;
        }
    }

    std::vector<std::string> types;
    for (const auto &t : p.types)
    {
        switch (t)
        {
            case card::type::basic_land:
                types.emplace_back("Basic Land");
                break;
            case card::type::land:
                types.emplace_back("Land");
                break;
            case card::type::creature:
                types.emplace_back("Creature");
                break;
            case card::type::artifact:
                types.emplace_back("Artifact");
                break;
            case card::type::enchantment:
                types.emplace_back("Enchantment");
                break;
            case card::type::planeswalker:
                types.emplace_back("Planeswalker");
                break;
            case card::type::instant:
                types.emplace_back("Instant");
                break;
            case card::type::sorcery:
                types.emplace_back("Sorcery");
                break;
        }
    }

    if (p.subtypes.size() > 0)
    {
        j["subtypes"] = p.subtypes;
    }

    nlohmann::json power;
    if (p.power)
    {
        power = *p.power;
    }

    nlohmann::json toughness;
    if (p.toughness)
    {
        toughness = *p.toughness;
    }

    j = nlohmann::json{
//            {"id",             p.id},
            {"name",           p.name},
            {"types",          types},
            {"color_identity", color_identities},
            {"text",           p.text},
            {"power",          power},
            {"toughness",      toughness},
            {"cmc",            p.converted_mana_cost}
    };
}
} //namespace magique
