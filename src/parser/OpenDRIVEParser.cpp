#include "OpenDRIVEParser.h"
#include <algorithm>
#include <stdexcept>

OpenDRIVEFile OpenDRIVEParser::parse(const std::string& filepath) {
    OpenDRIVEFile file;
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(filepath.c_str());
    if (!result) {
        throw std::runtime_error("Failed to parse OpenDRIVE file");
    }
    
  pugi::xml_node root = doc.child("OpenDRIVE");
    std::string root_name = root.name();
    if (root_name != "OpenDRIVE") {
        throw std::runtime_error("Root element must be OpenDRIVE");
    }
    
    pugi::xml_node headerNode = root.child("header");
    if (headerNode) {
        parseHeader(headerNode, file.header);
    }
    parseRoads(root, file);
    return file;
}

void OpenDRIVEParser::parseHeader(pugi::xml_node node, Header& header) {
    header.name = node.attribute("name").as_string();
    header.version = node.attribute("version").as_string();
    header.date = node.attribute("date").as_string();
    header.vendor = node.attribute("vendor").as_string();
    header.revMajor = node.attribute("revMajor").as_int();
    header.revMinor = node.attribute("revMinor").as_int();
    header.east = node.attribute("east").as_double();
    header.north = node.attribute("north").as_double();
    header.south = node.attribute("south").as_double();
    header.west = node.attribute("west").as_double();
    
    auto offsetNode = node.child("offset");
    if (offsetNode) {
        header.offsetX = offsetNode.attribute("x").as_double();
        header.offsetY = offsetNode.attribute("y").as_double();
        header.offsetZ = offsetNode.attribute("z").as_double();
        header.offsetHdg = offsetNode.attribute("hdg").as_double();
    }
}

void OpenDRIVEParser::parseRoads(pugi::xml_node parent, OpenDRIVEFile& file) {
    for (auto& roadNode : parent.children("road")) {
        Road road;
        road.id = roadNode.attribute("id").as_int();
        road.name = roadNode.attribute("name").as_string();
        road.length = roadNode.attribute("length").as_double();
        road.rule = roadNode.attribute("rule").as_string();
        if (road.rule.empty()) road.rule = "RHT";
        road.header = &file.header;
        parsePlanView(roadNode, road);
        parseElevationProfile(roadNode, road);
        parseLateralProfile(roadNode, road);
        parseLanes(roadNode, road);
        parseObjects(roadNode, road);
        parseSignals(roadNode, road);
        file.roads.push_back(road);
    }
}
void OpenDRIVEParser::parsePlanView(pugi::xml_node roadNode, Road& road) {
    auto planView = roadNode.child("planView");
    if (!planView) return;
    for (auto& geomNode : planView.children("geometry")) {
        Geometry geom;
        geom.s = geomNode.attribute("s").as_double();
        geom.x = geomNode.attribute("x").as_double();
        geom.y = geomNode.attribute("y").as_double();
        geom.hdg = geomNode.attribute("hdg").as_double();
        geom.length = geomNode.attribute("length").as_double();
        for (auto& child : geomNode.children()) {
            std::string geomType = child.name();
            if (geomType == "line") {
                geom.type = GEOM_LINE;
            } else if (geomType == "spiral") {
                geom.type = GEOM_SPIRAL;
                geom.curvStart = child.attribute("curvStart").as_double();
                geom.curvEnd = child.attribute("curvEnd").as_double();
            } else if (geomType == "arc") {
                geom.type = GEOM_ARC;
                geom.curvature = child.attribute("curvature").as_double();
            } else if (geomType == "poly3") {
                geom.type = GEOM_POLY3;
                geom.a = child.attribute("a").as_double();
                geom.b = child.attribute("b").as_double();
                geom.c = child.attribute("c").as_double();
                geom.d = child.attribute("d").as_double();
            } else if (geomType == "paramPoly3") {
                geom.type = GEOM_PARAMPOLY3;
                geom.aU = child.attribute("aU").as_double();
                geom.aV = child.attribute("aV").as_double();
                geom.bU = child.attribute("bU").as_double();
                geom.bV = child.attribute("bV").as_double();
                geom.cU = child.attribute("cU").as_double();
                geom.cV = child.attribute("cV").as_double();
                geom.dU = child.attribute("dU").as_double();
                geom.dV = child.attribute("dV").as_double();
                std::string pRange = child.attribute("pRange").as_string();
                geom.pRangeNormalized = (pRange == "normalized");
            }
        }
        road.planViewGeometries.push_back(geom);
    }
}
void OpenDRIVEParser::parseElevationProfile(pugi::xml_node roadNode, Road& road) {
    auto elevProfile = roadNode.child("elevationProfile");
    if (!elevProfile) return;
    for (auto& elevNode : elevProfile.children("elevation")) {
        ElevationPoint ep;
        ep.s = elevNode.attribute("s").as_double();
        ep.a = elevNode.attribute("a").as_double();
        ep.b = elevNode.attribute("b").as_double();
        ep.c = elevNode.attribute("c").as_double();
        ep.d = elevNode.attribute("d").as_double();
        road.elevations.push_back(ep);
    }
}

void OpenDRIVEParser::parseLateralProfile(pugi::xml_node roadNode, Road& road) {
    auto latProfile = roadNode.child("lateralProfile");
    if (!latProfile) return;
    for (auto& superelevNode : latProfile.children("superelevation")) {
        SuperelevationPoint se;
        se.s = superelevNode.attribute("s").as_double();
        se.a = superelevNode.attribute("a").as_double();
        se.b = superelevNode.attribute("b").as_double();
        se.c = superelevNode.attribute("c").as_double();
        se.d = superelevNode.attribute("d").as_double();
        road.superelevations.push_back(se);
    }
}

void OpenDRIVEParser::parseLanes(pugi::xml_node roadNode, Road& road) {
    auto lanesNode = roadNode.child("lanes");
    if (!lanesNode) return;
    parseLaneSections(lanesNode, road);
    for (auto& loNode : lanesNode.children("laneOffset")) {
        LaneWidthCoeff lw;
        lw.s = loNode.attribute("s").as_double();
        lw.a = loNode.attribute("a").as_double();
        lw.b = loNode.attribute("b").as_double();
        lw.c = loNode.attribute("c").as_double();
        lw.d = loNode.attribute("d").as_double();
        road.laneOffsets.push_back(lw);
    }
}

void OpenDRIVEParser::parseLaneSections(pugi::xml_node lanesNode, Road& road) {
    for (auto& lsNode : lanesNode.children("laneSection")) {
        LaneSection ls;
        ls.s = lsNode.attribute("s").as_double();
        auto center = lsNode.child("center");
        if (center) {
            for (auto& laneNode : center.children("lane")) {
                Lane lane;
                lane.id = laneNode.attribute("id").as_int();
                std::string typeStr = laneNode.attribute("type").as_string();
                if (typeStr == "driving") lane.type = LANE_DRIVING;
                else if (typeStr == "shoulder") lane.type = LANE_SHOULDER;
                else if (typeStr == "border") lane.type = LANE_BORDER;
                else if (typeStr == "stop") lane.type = LANE_STOP;
                else if (typeStr == "none") lane.type = LANE_NONE;
                else if (typeStr == "restricted") lane.type = LANE_RESTRICTED;
                else if (typeStr == "parking") lane.type = LANE_PARKING;
                else if (typeStr == "median") lane.type = LANE_MEDIAN;
                else if (typeStr == "biking") lane.type = LANE_BIKING;
                else if (typeStr == "sidewalk") lane.type = LANE_SIDEWALK;
                else if (typeStr == "curb") lane.type = LANE_CURB;
                else if (typeStr == "walking") lane.type = LANE_WALKING;
                else if (typeStr == "tram") lane.type = LANE_TRAM;
                else if (typeStr == "rail") lane.type = LANE_RAIL;
                else lane.type = LANE_DRIVING;
                ls.centerLane.push_back(lane);
            }
        }
        auto left = lsNode.child("left");
        if (left) {
            parseLaneGroup(left, ls.leftLanes, "left");
        }
        auto right = lsNode.child("right");
        if (right) {
            parseLaneGroup(right, ls.rightLanes, "right");
        }
        road.laneSections.push_back(ls);
    }
}

void OpenDRIVEParser::parseLaneGroup(pugi::xml_node groupNode, std::vector<Lane>& lanes, const std::string& side) {
    for (auto& laneNode : groupNode.children("lane")) {
        Lane lane;
        lane.id = laneNode.attribute("id").as_int();
        std::string typeStr = laneNode.attribute("type").as_string();
        if (typeStr == "driving") lane.type = LANE_DRIVING;
        else if (typeStr == "shoulder") lane.type = LANE_SHOULDER;
        else if (typeStr == "border") lane.type = LANE_BORDER;
        else if (typeStr == "stop") lane.type = LANE_STOP;
        else if (typeStr == "none") lane.type = LANE_NONE;
        else if (typeStr == "restricted") lane.type = LANE_RESTRICTED;
        else if (typeStr == "parking") lane.type = LANE_PARKING;
        else if (typeStr == "median") lane.type = LANE_MEDIAN;
        else if (typeStr == "biking") lane.type = LANE_BIKING;
        else if (typeStr == "sidewalk") lane.type = LANE_SIDEWALK;
        else if (typeStr == "curb") lane.type = LANE_CURB;
        else if (typeStr == "walking") lane.type = LANE_WALKING;
        else if (typeStr == "tram") lane.type = LANE_TRAM;
        else if (typeStr == "rail") lane.type = LANE_RAIL;
        else lane.type = LANE_DRIVING;
        std::string levelStr = laneNode.attribute("level").as_string();
       if (levelStr == "true") lane.level = true;
        for (auto& widthNode : laneNode.children("width")) {
            LaneWidthCoeff w;
            w.s = widthNode.attribute("sOffset").as_double();
            w.a = widthNode.attribute("a").as_double();
            w.b = widthNode.attribute("b").as_double();
            w.c = widthNode.attribute("c").as_double();
            w.d = widthNode.attribute("d").as_double();
            lane.widths.push_back(w);
        }
        lanes.push_back(lane);
    }
}
void OpenDRIVEParser::parseObjects(pugi::xml_node roadNode, Road& road) {
    auto objGroup = roadNode.child("objects");
    if (!objGroup) return;
    for (auto& objNode : objGroup.children("object")) {
        Object obj;
        obj.id = objNode.attribute("id").as_string();
        obj.s = objNode.attribute("s").as_double();
        obj.t = objNode.attribute("t").as_double();
        obj.zOffset = objNode.attribute("zOffset").as_double();
        obj.width = objNode.attribute("width").as_double();
        obj.length = objNode.attribute("length").as_double();
        obj.height = objNode.attribute("height").as_double();
        obj.radius = objNode.attribute("radius").as_double();
        obj.hdg = objNode.attribute("hdg").as_double();
        obj.pitch = objNode.attribute("pitch").as_double();
        obj.roll = objNode.attribute("roll").as_double();
        obj.name = objNode.attribute("name").as_string();
        obj.subtype = objNode.attribute("subtype").as_string();
        std::string typeStr = objNode.attribute("type").as_string();
        if (typeStr == "barrier") obj.type = OBJ_BARRIER;
        else if (typeStr == "pole") obj.type = OBJ_POLE;
        else if (typeStr == "tree") obj.type = OBJ_TREE;
        else if (typeStr == "vegetation") obj.type = OBJ_VEGETATION;
        else if (typeStr == "building") obj.type = OBJ_BUILDING;
        else if (typeStr == "obstacle") obj.type = OBJ_OBSTACLE;
        else if (typeStr == "gantry") obj.type = OBJ_GANTRY;
        else obj.type = OBJ_NONE;
        obj.hasRepeat = false;
        auto repeatNode = objNode.child("repeat");
        if (repeatNode) {
            obj.hasRepeat = true;
            obj.repeat.s = repeatNode.attribute("s").as_double();
            obj.repeat.length = repeatNode.attribute("length").as_double();
            obj.repeat.distance = repeatNode.attribute("distance").as_double();
            obj.repeat.tStart = repeatNode.attribute("tStart").as_double();
            obj.repeat.tEnd = repeatNode.attribute("tEnd").as_double();
            obj.repeat.zOffsetStart = repeatNode.attribute("zOffsetStart").as_double();
            obj.repeat.zOffsetEnd = repeatNode.attribute("zOffsetEnd").as_double();
            obj.repeat.heightStart = repeatNode.attribute("heightStart").as_double();
            obj.repeat.heightEnd = repeatNode.attribute("heightEnd").as_double();
        }
        road.objects.push_back(obj);
    }
}

void OpenDRIVEParser::parseSignals(pugi::xml_node roadNode, Road& road) {
    auto sigGroup = roadNode.child("signals");
    if (!sigGroup) return;
    for (auto& sigNode : sigGroup.children("signal")) {
        Signal sig;
        sig.id = sigNode.attribute("id").as_string();
        sig.name = sigNode.attribute("name").as_string();
        sig.type = SIG_GENERIC;
        sig.subtype = sigNode.attribute("subtype").as_string();
        auto posNode = sigNode.child("positionRoad");
        if (posNode) {
            sig.s = posNode.attribute("s").as_double();
            sig.z = posNode.attribute("z").as_double();
        }
        sig.hOffset = sigNode.attribute("hOffset").as_double();
        sig.orientation = sigNode.attribute("orientation").as_string();
        sig.dynamic = (sigNode.attribute("dynamic").as_int() == 1);
        sig.height = sigNode.attribute("height").as_double();
        road.signals.push_back(sig);
    }
}
