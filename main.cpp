#include "openvr.h"

#include <atomic>
#include <iostream>
#include <string>
#include <sstream>

#include <thread>
#include <mutex>

#include <filesystem>
#include <vector>

int main(int argc, char *argv[]) {
    vr::EVRInitError err = vr::VRInitError_None;
    vr::VR_Init(&err, vr::VRApplication_Scene);

    if (err != vr::VRInitError_None) {
        std::cout << "failed to init openvr! " << vr::VR_GetVRInitErrorAsEnglishDescription(err) << std::endl;
        return 1;
    }

    std::filesystem::path cwd = std::filesystem::current_path() / "actions.json";

    vr::EVRInputError inputError;
    inputError = vr::VRInput()->SetActionManifestPath(cwd.string().c_str());

    if(inputError != vr::VRInputError_None) {
        std::cout << "action manifest error " << inputError << std::endl;
    }

    vr::VRActionSetHandle_t actionSet;
    vr::VRInput()->GetActionSetHandle("/actions/default", &actionSet);

    vr::VRActionHandle_t haptic;
    vr::VRInput()->GetActionHandle("/actions/default/in/HapticVibration", &haptic);

    vr::VRInputValueHandle_t leftHand;
    vr::VRInput()->GetInputSourceHandle("/user/hand/left", &leftHand);

    vr::VRInputValueHandle_t rightHand;
    vr::VRInput()->GetInputSourceHandle("/user/hand/right", &rightHand);

    std::cout << "Commands:" << std::endl;
    std::cout << "quit" << std::endl;

    std::cout << "Usage:\n";
    std::cout << "hand(s)+duration(seconds)+frequency(hz)+amplitude(0 - 1)\n";

    std::cout << "Example:\n";
    std::cout << "left,right+1+4+1\n";
    std::cout << "right+0.5+10+0.5\n";

    std::cout << "waiting for input" << std::endl;

    while (true) {
        std::string line;
        std::getline(std::cin, line);

        //remove whitespace from line
        line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());

        if (line == "quit") {
            break;
        }

        std::stringstream ssline(line);

        std::vector<std::string> tokens;
        while (std::getline(ssline, line, '+')) {
            tokens.push_back(line);
        }

        if (tokens.size() < 3) {
            std::cout << "invalid input!" << std::endl;
            continue;
        }

        //parse hands
        std::vector<std::string> hands;
        std::stringstream sshands(tokens[0]);
        while (std::getline(sshands, line, ',')) {
            hands.push_back(line);
        }
        if (hands.empty()) {
            std::cout << "no hands!" << std::endl;
            continue;
        }

        vr::VRActiveActionSet_t activeActionSet = { 0 };
        activeActionSet.ulActionSet = actionSet;
        vr::VRInput()->UpdateActionState( &activeActionSet, sizeof(activeActionSet), 1 );

        const float fSeconds = std::stof(tokens[1]);
        const float fFrequency = std::stof(tokens[2]);
        const float fAmplitude = std::stof(tokens[3]);

        for (const auto& hand : hands) {
            vr::VRInputValueHandle_t inputValueHandle = vr::k_ulInvalidInputValueHandle;
            if (hand == "left") {
                inputValueHandle = leftHand;
            }
            if (hand == "right") {
                inputValueHandle = rightHand;
            }

            vr::EVRInputError hapticError = vr::VRInput()->TriggerHapticVibrationAction(haptic, 0, fSeconds, fFrequency, fAmplitude, inputValueHandle);

            if (hapticError != vr::VRInputError_None) {
                std::cout << "haptic error " << hapticError << std::endl;
            }
        }

    }

    return 0;
}
