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
    parseJunctions(root, file);
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

void OpenDRIVEParser::parseLaneType(const std::string& typeStr, LaneType& laneType) {
    if (typeStr == "driving") laneType = LANE_DRIVING;
    else if (typeStr == "shoulder") laneType = LANE_SHOULDER;
    else if (typeStr == "border") laneType = LANE_BORDER;
    else if (typeStr == "stop") laneType = LANE_STOP;
    else if (typeStr == "none") laneType = LANE_NONE;
    else if (typeStr == "restricted") laneType = LANE_RESTRICTED;
    else if (typeStr == "parking") laneType = LANE_PARKING;
    else if (typeStr == "median") laneType = LANE_MEDIAN;
    else if (typeStr == "biking") laneType = LANE_BIKING;
    else if (typeStr == "sidewalk") laneType = LANE_SIDEWALK;
    else if (typeStr == "curb") laneType = LANE_CURB;
    else if (typeStr == "walking") laneType = LANE_WALKING;
    else if (typeStr == "tram") laneType = LANE_TRAM;
    else if (typeStr == "rail") laneType = LANE_RAIL;
    else if (typeStr == "bidirectional") laneType = LANE_BIDIRECTIONAL;
    else if (typeStr == "shared") laneType = LANE_SHARED;
    else if (typeStr == "onRamp") laneType = LANE_ONRAMP;
    else if (typeStr == "offRamp") laneType = LANE_OFFRAMP;
    else if (typeStr == "connectingRamp") laneType = LANE_CONNECTINGRAMP;
    else if (typeStr == "entry") laneType = LANE_ENTRY;
    else if (typeStr == "exit") laneType = LANE_EXIT;
    else if (typeStr == "bus") laneType = LANE_BUS;
    else if (typeStr == "taxi") laneType = LANE_TAXI;
    else if (typeStr == "hov") laneType = LANE_HOV;
    else if (typeStr == "slipLane") laneType = LANE_SLIP_LANE;
    else if (typeStr == "roadworks") laneType = LANE_ROADWORKS;
    else if (typeStr == "special1") laneType = LANE_SPECIAL1;
    else if (typeStr == "special2") laneType = LANE_SPECIAL2;
    else if (typeStr == "special3") laneType = LANE_SPECIAL3;
    else laneType = LANE_DRIVING;
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
                parseLaneType(typeStr, lane.type);
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
        parseLaneType(typeStr, lane.type);
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
void OpenDRIVEParser::parseObjectType(const std::string& typeStr, ObjectType& objType) {
    if (typeStr == "barrier") objType = OBJ_BARRIER;
    else if (typeStr == "pole") objType = OBJ_POLE;
    else if (typeStr == "tree") objType = OBJ_TREE;
    else if (typeStr == "vegetation") objType = OBJ_VEGETATION;
    else if (typeStr == "building") objType = OBJ_BUILDING;
    else if (typeStr == "obstacle") objType = OBJ_OBSTACLE;
    else if (typeStr == "gantry") objType = OBJ_GANTRY;
    else if (typeStr == "crosswalk") objType = OBJ_CROSSWALK;
    else if (typeStr == "roadMark") objType = OBJ_ROAD_MARK;
    else if (typeStr == "roadSurface") objType = OBJ_ROAD_SURFACE;
    else if (typeStr == "parkingSpace") objType = OBJ_PARKING_SPACE;
    else if (typeStr == "trafficIsland") objType = OBJ_TRAFFIC_ISLAND;
    // Vehicle and device dependent objects fall through to OBJ_NONE
    else objType = OBJ_NONE;
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
        parseObjectType(typeStr, obj.type);
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

void OpenDRIVEParser::parseSignalType(const std::string& typeStr, SignalType& sigType) {
    if (typeStr == "speedLimit") sigType = SIG_SPEED_LIMIT;
    else if (typeStr == "maxSpeed") sigType = SIG_MAX_SPEED;
    else if (typeStr == "minSpeed") sigType = SIG_MIN_SPEED;
    else if (typeStr == "noPassing") sigType = SIG_NO_PASSING;
    else if (typeStr == "stop") sigType = SIG_STOP;
    else if (typeStr == "yield") sigType = SIG_YIELD;
    else if (typeStr == "trafficLight") sigType = SIG_TRAFFIC_LIGHT;
    else if (typeStr == "endAllRestrictions") sigType = SIG_END_ALL_RESTRICTIONS;
    else if (typeStr == "general") sigType = SIG_GENERAL;
    else if (typeStr == "priorityRoad") sigType = SIG_PRIORITY_ROAD;
    else if (typeStr == "parking") sigType = SIG_PARKING;
    else if (typeStr == "streetName") sigType = SIG_STREET_NAME;
    else if (typeStr == "endOfParking") sigType = SIG_END_OF_PARKING;
    else if (typeStr == "endOfNoPassing") sigType = SIG_END_OF_NO_PASSING;
    else if (typeStr == "maxSpeedEnd") sigType = SIG_MAX_SPEED_END;
    else if (typeStr == "minSpeedEnd") sigType = SIG_MIN_SPEED_END;
    else if (typeStr == "endOfAllRestrictions") sigType = SIG_END_OF_ALL_RESTRICTIONS;
    else if (typeStr == "zoneEnd") sigType = SIG_ZONE_END;
    else if (typeStr == "recommendedSpeed") sigType = SIG_RECOMMENDED_SPEED;
    else if (typeStr == "pedS") sigType = SIG_PEDS;
    else if (typeStr == "cyclePath") sigType = SIG_CYCLE_PATH;
    else if (typeStr == "dangerousCorners") sigType = SIG_DANGEROUS_CORNERS;
    else if (typeStr == "otherDanger") sigType = SIG_OTHER_DANGER;
    else if (typeStr == "roadWide") sigType = SIG_ROAD_WIDE;
    else if (typeStr == "roadNarrowsRight") sigType = SIG_ROAD_NARROWS_RIGHT;
    else if (typeStr == "roadNarrowsLeft") sigType = SIG_ROAD_NARROWS_LEFT;
    else if (typeStr == "roadworks") sigType = SIG_ROADWORKS;
    else if (typeStr == "preferOthers") sigType = SIG_PREFER_OTHERS;
    else if (typeStr == "roadBump") sigType = SIG_ROAD_BUMP;
    else if (typeStr == "unstop") sigType = SIG_UNSTOP;
    else if (typeStr == "unyield") sigType = SIG_UNYIELD;
    else if (typeStr == "sidewalk") sigType = SIG_SIDEWALK;
    else if (typeStr == "sidewalkEnd") sigType = SIG_SIDEWALK_END;
    else if (typeStr == "cyclePathEnd") sigType = SIG_CYCLE_PATH_END;
    else if (typeStr == "pedestrians") sigType = SIG_PEDESTRIANS;
    else if (typeStr == "pedestriansEnd") sigType = SIG_PEDESTRIANS_END;
    else if (typeStr == "railwayCrossing") sigType = SIG_RAILWAY_CROSSING;
    else if (typeStr == "automobileRoad") sigType = SIG_AUTOMOBILE_ROAD;
    else if (typeStr == "automobileEntry") sigType = SIG_AUTOMOBILE_ENTRY;
    else if (typeStr == "automobileEnd") sigType = SIG_AUTOMOBILE_END;
    else if (typeStr == "automobileFollow") sigType = SIG_AUTOMOBILE_FOLLOW;
    else if (typeStr == "priorityAutomobile") sigType = SIG_PRIORITY_AUTOMOBILE;
    else if (typeStr == "roadSigns") sigType = SIG_ROAD_SIGNS;
    else if (typeStr == "trafficSignal") sigType = SIG_TRAFFIC_SIGNAL;
    else if (typeStr == "trafficSignalBack") sigType = SIG_TRAFFIC_SIGNAL_BACK;
    else if (typeStr == "generalInformation") sigType = SIG_GENERAL_INFORMATION;
    else if (typeStr == "serviceArea") sigType = SIG_SERVICE_AREA;
    else if (typeStr == "automobileHospital") sigType = SIG_AUTOMOBILE_HOSPITAL;
    else if (typeStr == "automobileTelephone") sigType = SIG_AUTOMOBILE_TELEPHONE;
    else if (typeStr == "automobileShelter") sigType = SIG_AUTOMOBILE_SHELTER;
    else if (typeStr == "automobileGarage") sigType = SIG_AUTOMOBILE_GARAGE;
    else if (typeStr == "automobilePetrolStation") sigType = SIG_AUTOMOBILE_PETROL_STATION;
    else if (typeStr == "automobileExcursion") sigType = SIG_AUTOMOBILE_EXCURSION;
    else if (typeStr == "automobileCamping") sigType = SIG_AUTOMOBILE_CAMPING;
    else if (typeStr == "automobileRestArea") sigType = SIG_AUTOMOBILE_REST_AREA;
    else if (typeStr == "automobileToll") sigType = SIG_AUTOMOBILE_TOLL;
    else if (typeStr == "automobileTelephoneEmergency") sigType = SIG_AUTOMOBILE_TELEPHONE_EMERGENCY;
    else if (typeStr == "automobileTelephoneGeneral") sigType = SIG_AUTOMOBILE_TELEPHONE_GENERAL;
    else if (typeStr == "automobileTelephoneEmergencyRed") sigType = SIG_AUTOMOBILE_TELEPHONE_EMERGENCY_RED;
    else if (typeStr == "automobileTelephoneEmergencyGreen") sigType = SIG_AUTOMOBILE_TELEPHONE_EMERGENCY_GREEN;
    else if (typeStr == "automobileTelephoneEmergencyBlue") sigType = SIG_AUTOMOBILE_TELEPHONE_EMERGENCY_BLUE;
    else if (typeStr == "automobileTelephoneEmergencyYellow") sigType = SIG_AUTOMOBILE_TELEPHONE_EMERGENCY_YELLOW;
    else if (typeStr == "automobileTelephoneEmergencyOrange") sigType = SIG_AUTOMOBILE_TELEPHONE_EMERGENCY_ORANGE;
    else if (typeStr == "automobileTelephoneEmergencyViolet") sigType = SIG_AUTOMOBILE_TELEPHONE_EMERGENCY_VIOLET;
    else if (typeStr == "automobileTelephoneEmergencyBlack") sigType = SIG_AUTOMOBILE_TELEPHONE_EMERGENCY_BLACK;
    else if (typeStr == "automobileTelephoneEmergencyWhite") sigType = SIG_AUTOMOBILE_TELEPHONE_EMERGENCY_WHITE;
    else if (typeStr == "automobileTelephoneEmergencyGray") sigType = SIG_AUTOMOBILE_TELEPHONE_EMERGENCY_GRAY;
    else if (typeStr == "automobileTelephoneEmergencyBrown") sigType = SIG_AUTOMOBILE_TELEPHONE_EMERGENCY_BROWN;
    else if (typeStr == "automobileTelephoneEmergency") sigType = SIG_AUTOMOBILE_TELEPHONE_EMERGENCY;
    else sigType = SIG_GENERIC;
}

void OpenDRIVEParser::parseSignals(pugi::xml_node roadNode, Road& road) {
    auto sigGroup = roadNode.child("signals");
    if (!sigGroup) return;
    for (auto& sigNode : sigGroup.children("signal")) {
        Signal sig;
        sig.id = sigNode.attribute("id").as_string();
        sig.name = sigNode.attribute("name").as_string();
        std::string typeStr = sigNode.attribute("type").as_string();
        parseSignalType(typeStr, sig.type);
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

void OpenDRIVEParser::parseJunctions(pugi::xml_node parent, OpenDRIVEFile& file) {
    for (auto& junctionNode : parent.children("junction")) {
        Junction junction;
        junction.id = junctionNode.attribute("id").as_int();
        junction.name = junctionNode.attribute("name").as_string();
        junction.type = junctionNode.attribute("type").as_int();
        
        for (auto& mgNode : junctionNode.children("maneuverGroup")) {
            ManeuverGroup mg;
            mg.id = mgNode.attribute("id").as_string();
            for (auto& icNode : mgNode.children("incomingConnection")) {
                IncomingConnection ic;
                ic.roadId = icNode.attribute("roadId").as_int();
                mg.incomingConnections.push_back(ic);
            }
            for (auto& ocNode : mgNode.children("outgoingConnection")) {
                OutgoingConnection oc;
                oc.roadId = ocNode.attribute("roadId").as_int();
                mg.outgoingConnections.push_back(oc);
            }
            junction.maneuverGroups.push_back(mg);
        }
        
        for (auto& icNode : junctionNode.children("incomingConnection")) {
            IncomingConnection ic;
            ic.roadId = icNode.attribute("roadId").as_int();
            junction.roadIds.push_back(ic.roadId);
        }
        for (auto& ocNode : junctionNode.children("outgoingConnection")) {
            OutgoingConnection oc;
            oc.roadId = ocNode.attribute("roadId").as_int();
            junction.roadIds.push_back(oc.roadId);
        }
        
        file.junctions.push_back(junction);
    }
}
