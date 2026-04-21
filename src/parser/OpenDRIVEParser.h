#pragma once
#include <string>
#include "types.h"
#include <pugixml.hpp>

class OpenDRIVEParser {
public:
    OpenDRIVEFile parse(const std::string& filepath);

private:
    void parseHeader(pugi::xml_node node, Header& header);
    void parseRoads(pugi::xml_node parent, OpenDRIVEFile& file);
    void parsePlanView(pugi::xml_node roadNode, Road& road);
    void parseElevationProfile(pugi::xml_node roadNode, Road& road);
    void parseLateralProfile(pugi::xml_node roadNode, Road& road);
    void parseLanes(pugi::xml_node roadNode, Road& road);
    void parseLaneSections(pugi::xml_node lanesNode, Road& road);
    void parseLaneGroup(pugi::xml_node groupNode, std::vector<Lane>& lanes, const std::string& side);
    void parseObjects(pugi::xml_node roadNode, Road& road);
    void parseSignals(pugi::xml_node roadNode, Road& road);
};

