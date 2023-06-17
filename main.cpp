#include "openvr.h"

#include <atomic>
#include <iostream>
#include <string>
#include <sstream>

#include <thread>
#include <mutex>

#include <vector>

struct Config {
    short duration_micros = 1000;
    int interval_millis = 1000;

    std::vector<vr::ETrackedControllerRole> roles;
};

std::mutex config_mutex;
Config thread_config;
std::atomic<bool> is_running;

void RunThread() {
    while (is_running) {
        {
            std::lock_guard<std::mutex> lock(config_mutex);

            for(const vr::ETrackedControllerRole role : thread_config.roles) {
                vr::VRSystem()->TriggerHapticPulse(
                        vr::VRSystem()->GetTrackedDeviceIndexForControllerRole(role),
                        vr::k_EButton_Grip, thread_config.duration_micros);
            }
        }


        std::this_thread::sleep_for(std::chrono::milliseconds(thread_config.interval_millis));
    }
}

int main(int argc, char *argv[]) {
    vr::EVRInitError err = vr::VRInitError_None;
    vr::VR_Init(&err, vr::VRApplication_Scene);

    if (err != vr::VRInitError_None) {
        std::cout << "failed to init openvr! " << vr::VR_GetVRInitErrorAsEnglishDescription(err) << std::endl;
        return 1;
    }

    std::cout << "Commands:" << std::endl;
    std::cout << "start, stop, quit" << std::endl;

    std::cout << "Options:\n";
    std::cout << "hand(s)+on duration(microseconds)+interval(milliseconds)\n";

    std::cout << "Example 1:\n";
    std::cout << "left,right+1000+5\n";

    std::cout << "Example 2:\n";
    std::cout << "left+2000+10\n";

    std::cout << "waiting for input" << std::endl;

    std::thread run_thread;

    while (true) {
        std::string line;
        std::getline(std::cin, line);

        //remove whitespace from line
        line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());

        if (line == "quit") {
            if (is_running.exchange(false)) {
                run_thread.join();
            }
            break;
        }

        if (line == "start") {
            if (!is_running.exchange(true)) {
                run_thread = std::thread(RunThread);
            } else {
                std::cout << "already running!" << std::endl;
            }
        } else if (line == "stop") {
            if (is_running.exchange(false)) {
                run_thread.join();
            } else {
                std::cout << "not running!" << std::endl;
            }
        } else {
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

            std::scoped_lock<std::mutex> lock(config_mutex);
            thread_config.roles.clear();
            for (const auto &hand: hands) {
                if (hand == "left") {
                    thread_config.roles.push_back(vr::TrackedControllerRole_LeftHand);
                } else if (hand == "right") {
                    thread_config.roles.push_back(vr::TrackedControllerRole_RightHand);
                } else {
                    std::cout << "invalid hands input!" << std::endl;
                    continue;
                }
            }

            thread_config.duration_micros = (short)std::stoi(tokens[1]);
            thread_config.interval_millis = std::stoi(tokens[2]);

            if (!is_running.exchange(true)) {
                run_thread = std::thread(RunThread);
            }
        }
    }

    return 0;
}
