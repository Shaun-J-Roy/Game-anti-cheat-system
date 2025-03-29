#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>
using namespace std;
using namespace chrono;

class AntiCheatSystem {
private:
    struct SpeedRecord {
        int speed;
        steady_clock::time_point timestamp;
    };
    
    struct PlayerData {
        vector<SpeedRecord> speedHistory;
    };
    
    unordered_map<string, PlayerData> players;
    ofstream logFile;
    int speedThreshold = 120;
    double suddenIncreaseFactor = 1.5;
    bool autoScanEnabled = true;
    
    static bool detectSpeedHack(const vector<SpeedRecord>& speedHistory, int speedThreshold, double suddenIncreaseFactor) {
        if (speedHistory.size() < 3) return false;
        int suddenIncreaseCount = 0;
        int consistentHighSpeedCount = 0;
        
        for (size_t i = 1; i < speedHistory.size(); ++i) {
            int prevSpeed = speedHistory[i - 1].speed;
            int currSpeed = speedHistory[i].speed;
            auto timeDiff = duration_cast<seconds>(speedHistory[i].timestamp - speedHistory[i - 1].timestamp).count();
            
            if (timeDiff > 0 && currSpeed > prevSpeed * suddenIncreaseFactor) suddenIncreaseCount++;
            if (currSpeed > speedThreshold) consistentHighSpeedCount++;
        }
        
        return suddenIncreaseCount >= 2 || consistentHighSpeedCount >= 3;
    }
    
    string getCurrentTimestamp() {
        auto now = system_clock::now();
        time_t now_time = system_clock::to_time_t(now);
        char buffer[20];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&now_time));
        return string(buffer);
    }
    
    void logCheatAttempt(const string& playerName, const string& reason) {
        logFile << "{" << endl;
        logFile << "  \"timestamp\": \"" << getCurrentTimestamp() << "\"," << endl;
        logFile << "  \"player\": \"" << playerName << "\"," << endl;
        logFile << "  \"reason\": \"" << reason << "\"" << endl;
        logFile << "}," << endl;
    }
    
public:
    AntiCheatSystem() {
        logFile.open("cheat_log.json", ios::app);
        if (!logFile) {
            cerr << "Error opening log file!" << endl;
        }
    }
    
    ~AntiCheatSystem() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }
    
    void addPlayer(const string& playerName, int speed) {
        players[playerName] = {{SpeedRecord{speed, steady_clock::now()}}};
    }
    
    void updatePlayerSpeed(const string& playerName, int newSpeed) {
        if (players.find(playerName) != players.end()) {
            players[playerName].speedHistory.push_back(SpeedRecord{newSpeed, steady_clock::now()});
            if (players[playerName].speedHistory.size() > 5) {
                players[playerName].speedHistory.erase(players[playerName].speedHistory.begin());
            }
        }
    }
    
    void scanForCheaters() {
        for (const auto& player : players) {
            bool speedHack = detectSpeedHack(player.second.speedHistory, speedThreshold, suddenIncreaseFactor);
            
            if (speedHack) {
                cout << "Player " << player.first << " is suspected of cheating!" << endl;
                cout << " - Speed hack detected" << endl;
                logCheatAttempt(player.first, "Speed hack detected");
            }
        }
    }
    
    void enableAutoScan() {
        while (autoScanEnabled) {
            this_thread::sleep_for(seconds(10)); // Run scan every 10 seconds
            scanForCheaters();
        }
    }
    
    void setThresholds(int newSpeedThreshold, double newIncreaseFactor) {
        speedThreshold = newSpeedThreshold;
        suddenIncreaseFactor = newIncreaseFactor;
    }
    
    void stopAutoScan() {
        autoScanEnabled = false;
    }
};

int main() {
    AntiCheatSystem antiCheat;
    int choice;
    thread autoScanThread(&AntiCheatSystem::enableAutoScan, &antiCheat);
    
    while (true) {
        cout << "\nAnti-Cheat System Menu:\n";
        cout << "1. Add Player\n";
        cout << "2. Update Player Speed\n";
        cout << "3. Scan for Cheaters\n";
        cout << "4. Set Detection Thresholds\n";
        cout << "5. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;
        
        if (choice == 1) {
            string playerName;
            int speed;
            cout << "Enter player name: ";
            cin >> playerName;
            cout << "Enter initial speed: ";
            cin >> speed;
            antiCheat.addPlayer(playerName, speed);
        } else if (choice == 2) {
            string playerName;
            int newSpeed;
            cout << "Enter player name: ";
            cin >> playerName;
            cout << "Enter new speed: ";
            cin >> newSpeed;
            antiCheat.updatePlayerSpeed(playerName, newSpeed);
        } else if (choice == 3) {
            antiCheat.scanForCheaters();
        } else if (choice == 4) {
            int newThreshold;
            double newFactor;
            cout << "Enter new speed threshold: ";
            cin >> newThreshold;
            cout << "Enter new sudden increase factor: ";
            cin >> newFactor;
            antiCheat.setThresholds(newThreshold, newFactor);
        } else if (choice == 5) {
            antiCheat.stopAutoScan();
            cout << "Exiting program..." << endl;
            autoScanThread.join();
            break;
        } else {
            cout << "Invalid choice! Try again." << endl;
        }
    }
    
    return 0;
}

