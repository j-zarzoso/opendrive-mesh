#pragma once

#include <iostream>
#include <string>
#include <stdexcept>
#include <cctype>
#include <locale>
#include "parser/OpenDRIVEParser.h"
#include "mesh/MeshGenerator.h"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: opendrive-mesh <input.xodr> <output.usd> [step_size]" << std::endl;
        std::cout << "  step_size: optional, default 0.5 (meters)" << std::endl;
        return 1;
    }
    
    std::string inputPath = argv[1];
    std::string outputPath = argv[2];
    double stepSize = 0.5;
    
    if (argc >= 4) {
        stepSize = std::stod(argv[3]);
    }
    
    try {
        std::cout << "Parsing OpenDRIVE file: " << inputPath << std::endl;
        OpenDRIVEParser parser;
        auto file = parser.parse(inputPath);
        
        std::cout << "Found " << file.roads.size() << " roads" << std::endl;
        int objCount = 0, sigCount = 0;
        for (const auto& road : file.roads) {
            objCount += road.objects.size();
            sigCount += road.signals.size();
        }
        std::cout << "Objects: " << objCount << " Signals: " << sigCount << std::endl;
        
   std::cout << "Generating meshes with step size " << stepSize << "m..." << std::endl;
    MeshGenerator generator(file, stepSize);
    
    std::string lowerOutput = outputPath;
    for (auto& c : lowerOutput) c = std::tolower(c, std::locale::classic());
    
    bool isObj = lowerOutput.rfind(".obj") != std::string::npos;
    
    if (isObj) {
        generator.exportToObj(outputPath);
    } else {
        generator.exportToUsd(outputPath);
    }
    
    std::cout << "Successfully exported to: " << outputPath << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
