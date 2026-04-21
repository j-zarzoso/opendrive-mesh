#pragma once
#include <string>
#include <vector>
#include <cstdint>

struct Header {
    std::string name, version, date, vendor;
    int revMajor = 1, revMinor = 8;
    double east = 0, north = 0, south = 0, west = 0;
    double offsetX = 0, offsetY = 0, offsetZ = 0, offsetHdg = 0;
};

enum GeometryType {
    GEOM_LINE,
    GEOM_SPIRAL,
    GEOM_ARC,
    GEOM_POLY3,
    GEOM_PARAMPOLY3
};

struct Geometry {
    GeometryType type;
    double s, x, y, hdg, length;
    double curvature; // for ARC
    double curvStart, curvEnd; // for SPIRAL
    double a, b, c, d; // for POLY3
    // For PARAMPOLY3
    double aU, aV, bU, bV, cU, cV, dU, dV;
    bool pRangeNormalized; // false=arcLength, true=normalized
};

struct ElevationPoint {
    double s, a, b, c, d;
};

struct SuperelevationPoint {
    double s, a, b, c, d;
};

struct LaneWidthCoeff {
    double s, a, b, c, d;
};

struct LaneBorderPoint {
    double sOffset, a, b, c, d;
    int laneId;
};

enum LaneType {
    LANE_SHOULDER,
    LANE_BORDER,
    LANE_DRIVING,
    LANE_STOP,
    LANE_NONE,
    LANE_RESTRICTED,
    LANE_PARKING,
    LANE_MEDIAN,
    LANE_BIKING,
    LANE_SHARED,
    LANE_SIDEWALK,
    LANE_CURB,
    LANE_EXIT,
    LANE_ENTRY,
    LANE_ONRAMP,
    LANE_OFFRAMP,
    LANE_CONNECTINGRAMP,
    LANE_BIDIRECTIONAL,
    LANE_TRAM,
    LANE_RAIL,
    LANE_BUS,
    LANE_TAXI,
    LANE_HOV,
    LANE_WALKING,
    LANE_SLIP_LANE,
    LANE_ROADWORKS,
    LANE_SPECIAL1,
    LANE_SPECIAL2,
    LANE_SPECIAL3
};

struct Lane {
    int id;
    LaneType type;
    std::vector<LaneWidthCoeff> widths;
    std::vector<LaneBorderPoint> borders;
    bool level = false; // keep on level, no superelevation
};

struct LaneSection {
    double s;
    std::vector<Lane> leftLanes, centerLane, rightLanes;
    
    Lane* getLaneById(int id);
    std::vector<Lane*> getAllLanes() const;
};

enum ObjectType {
    OBJ_NONE,
    OBJ_OBSTACLE,
    OBJ_POLE,
    OBJ_TREE,
    OBJ_VEGETATION,
    OBJ_BARRIER,
    OBJ_BUILDING,
    OBJ_PARKING_SPACE,
    OBJ_TRAFFIC_ISLAND,
    OBJ_CROSSWALK,
    OBJ_GANTRY,
    OBJ_ROAD_MARK,
    OBJ_ROAD_SURFACE,
    OBJ_VAN,
    OBJ_BUS_DEP,
    OBJ_BIKE_DEP,
    OBJ_MOTORBIKE_DEP,
    OBJ_TRAM_DEP,
    OBJ_TRAIN_DEP,
    OBJ_PED_DEP,
    OBJ_WIND_DEP,
    OBJ_CAR_DEP,
    OBJ_VAN_DEP,
    OBJ_TRAILER_DEP,
    OBJ_BUS_DEP2
};

enum Orientation {
    ORIENT_POS,
    ORIENT_NEG,
    ORIENT_NONE
};

struct Object {
    std::string id, name;
    ObjectType type;
    double s, t, zOffset;
    double width, length, height;
    double radius;
    double hdg, pitch, roll;
    std::string subtype;
    Orientation orientation;
    bool perpToRoad = false;
    
    struct Repeat {
        double s, length, distance;
        double tStart, tEnd;
        double zOffsetStart, zOffsetEnd;
        double heightStart, heightEnd;
        double radiusStart, radiusEnd;
        double widthStart, widthEnd;
        double lengthStart, lengthEnd;
    };
    Repeat repeat;
    bool hasRepeat = false;
};

enum SignalType {
    SIG_SPEED_LIMIT,
    SIG_MAX_SPEED,
    SIG_MIN_SPEED,
    SIG_NO_PASSING,
    SIG_STOP,
    SIG_YIELD,
    SIG_TRAFFIC_LIGHT,
    SIG_END_ALL_RESTRICTIONS,
    SIG_GENERAL,
    SIG_PRIORITY_ROAD,
    SIG_PARKING,
    SIG_STREET_NAME,
    SIG_END_OF_PARKING,
    SIG_END_OF_NO_PASSING,
    SIG_MAX_SPEED_END,
    SIG_MIN_SPEED_END,
    SIG_END_OF_ALL_RESTRICTIONS,
    SIG_ZONE_END,
    SIG_RECOMMENDED_SPEED,
    SIG_PEDS,
    SIG_CYCLE_PATH,
    SIG_DANGEROUS_CORNERS,
    SIG_OTHER_DANGER,
    SIG_ROAD_WIDE,
    SIG_ROAD_NARROWS_RIGHT,
    SIG_ROAD_NARROWS_LEFT,
    SIG_ROADWORKS,
    SIG_PREFER_OTHERS,
    SIG_ROAD_BUMP,
    SIG_UNSTOP,
    SIG_UNYIELD,
    SIG_SIDEWALK,
    SIG_SIDEWALK_END,
    SIG_CYCLE_PATH_END,
    SIG_PEDESTRIANS,
    SIG_PEDESTRIANS_END,
    SIG_RAILWAY_CROSSING,
    SIG_AUTOMOBILE_ROAD,
    SIG_AUTOMOBILE_ENTRY,
    SIG_AUTOMOBILE_END,
    SIG_AUTOMOBILE_FOLLOW,
    SIG_PRIORITY_AUTOMOBILE,
    SIG_ROAD_SIGNS,
    SIG_TRAFFIC_SIGNAL,
    SIG_TRAFFIC_SIGNAL_BACK,
    SIG_GENERAL_INFORMATION,
    SIG_SERVICE_AREA,
    SIG_AUTOMOBILE_HOSPITAL,
    SIG_AUTOMOBILE_TELEPHONE,
    SIG_AUTOMOBILE_SHELTER,
    SIG_AUTOMOBILE_GARAGE,
    SIG_AUTOMOBILE_PETROL_STATION,
    SIG_AUTOMOBILE_EXCURSION,
    SIG_AUTOMOBILE_CAMPING,
    SIG_AUTOMOBILE_REST_AREA,
    SIG_AUTOMOBILE_TOLL,
    SIG_AUTOMOBILE_TELEPHONE_EMERGENCY,
    SIG_AUTOMOBILE_TELEPHONE_GENERAL,
    SIG_AUTOMOBILE_TELEPHONE_EMERGENCY_RED,
    SIG_AUTOMOBILE_TELEPHONE_EMERGENCY_GREEN,
    SIG_AUTOMOBILE_TELEPHONE_EMERGENCY_BLUE,
    SIG_AUTOMOBILE_TELEPHONE_EMERGENCY_YELLOW,
    SIG_AUTOMOBILE_TELEPHONE_EMERGENCY_ORANGE,
    SIG_AUTOMOBILE_TELEPHONE_EMERGENCY_VIOLET,
    SIG_AUTOMOBILE_TELEPHONE_EMERGENCY_BLACK,
    SIG_AUTOMOBILE_TELEPHONE_EMERGENCY_WHITE,
    SIG_AUTOMOBILE_TELEPHONE_EMERGENCY_GRAY,
    SIG_AUTOMOBILE_TELEPHONE_EMERGENCY_BROWN,
    SIG_GENERIC
};

struct Signal {
    std::string id, name;
    SignalType type;
    std::string subtype;
    double s, z, hOffset;
    double height = 0;
    std::string orientation;
    bool dynamic = false;
};

struct Road {
    int id;
    std::string name;
    double length;
    std::string rule; // RHT or LHT
    Header* header = nullptr;
    
    std::vector<Geometry> planViewGeometries;
    std::vector<ElevationPoint> elevations;
    std::vector<SuperelevationPoint> superelevations;
    std::vector<LaneSection> laneSections;
    std::vector<LaneWidthCoeff> laneOffsets;
    std::vector<Object> objects;
    std::vector<Signal> signals;
};

struct OpenDRIVEFile {
    Header header;
    std::vector<Road> roads;
    std::vector<Object> globalObjects;
    std::vector<Signal> globalSignals;
};
