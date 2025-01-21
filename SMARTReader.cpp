#include "SMARTReader.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <vector>
#include <sstream>
#include <format>
#include <algorithm>


using namespace std;

const int HDD = 1;
const string HDD_LABEL = "HDD";
const int SSD = 2;
const string SSD_LABEL = "SSD";

// SMART keys
const string MODEL_PARAM = "Model";
// HDD detector
const string SPIN_UP_TIME = "Spin_Up_Time";
const string ROTATION_RATE = "Rotation_Rate";
// INDICATORS of HDD LIFETIME
const string RAW_READ_ERROR_RATE = "Raw_Read_Error_Rate";
const int RAW_READ_ERROR_RATE_MAX = 1000;
const string REALLOCATED_SECTOR_CT = "Reallocated_Sector_Ct";
const int REALLOCATED_SECTOR_CT_MAX = 1000;
const string SEEK_ERROR_RATE = "Seek_Error_Rate";
const int SEEK_ERROR_RATE_MAX = 1000;
const string POWER_ON_HOURS = "Power_On_Hours";
const string REALLOCATED_EVENT_COUNT = "Reallocated_Event_Count";
const int REALLOCATED_EVENT_COUNT_MAX = 1000;
const string HDD_TEMPERATURE_CELSIUS = "Temperature_Celsius";

const string HDD_TEMPERATURE_CELSIUS_MIN_LABEL = "Min_Temperature_Celcius";
const string HDD_TEMPERATURE_CELSIUS_MAX_LABEL = "Max_Temperature_Celcius";
const string HDD_TEMPERATURE_CELSIUS_ACTUAL_LABEL = "Actual_Temperature_Celcius";
// INDICATORS of SSD LIFETIME
const string SSD_TEMPERATURE_CELSIUS_ACTUAL_LABEL = "Temperature";
const string SSD_PERCENTAGE_USED = "Percentage Used";

SMARTReader::SMARTReader()
{
    SmartData = LoadSmartData();
}

int SMARTReader::DetermineDiskType(string line)
{
    if (
        line.find(SPIN_UP_TIME) != string::npos ||
        line.find(ROTATION_RATE) != string::npos
        ) {
        return HDD;
    }
    return SSD;
}

vector<map<string, string>> SMARTReader::LoadSmartData()
{
    vector<map<string, string>> formattedResult;
    vector<map<string, string>> devices = GetDeviceList();
    vector<vector<string>> devicesRawResult = ParseLoadedDeviceResultRaw(devices);

    vector<map<string, string>> devicesData;

    for (vector<string> deviceRaw : devicesRawResult)
    {
        map<string, string> device = ProcessRawDeviceData(deviceRaw);
        devicesData.push_back(device);
    }

    for (map<string, string> device : devicesData) {
        map<string, string> formattedResultMap;

        formattedResultMap["type"] = device["type"];
        formattedResultMap["name"] = device["name"];

        int maximumLifeTimePercent = 100;
        int remainingLifeTimePercent = 100;
        if (device["type"] == HDD_LABEL) {
            try {
                double actualDevicePercentDecrease;
                // Doing just by guess, cannot determine truly
                int actualRECD = stoi(device[REALLOCATED_EVENT_COUNT]);
                double actualSERD = stoi(device[SEEK_ERROR_RATE]);
                double actualRSCD = stoi(device[REALLOCATED_SECTOR_CT]);
                double actualRRERD = stoi(device[RAW_READ_ERROR_RATE]);

                actualDevicePercentDecrease = (
                    (((actualRECD / REALLOCATED_EVENT_COUNT_MAX) * 100) * 0.25) +
                    (((actualSERD / SEEK_ERROR_RATE_MAX) * 100) * 0.25) +
                    (((actualRSCD / REALLOCATED_SECTOR_CT_MAX) * 100) * 0.25) +
                    (((actualRRERD / RAW_READ_ERROR_RATE_MAX) * 100) * 0.25)
                    );
                remainingLifeTimePercent = maximumLifeTimePercent - actualDevicePercentDecrease;
            }
            catch (const invalid_argument& e) {
                continue;
            }
            catch (const out_of_range& e) {
                continue;
            }
        }
        else if (device["type"] == SSD_LABEL) {
            try {
                remainingLifeTimePercent = maximumLifeTimePercent - stoi(device[SSD_PERCENTAGE_USED]);
            }
            catch (const invalid_argument& e) {
                continue;
            }
            catch (const out_of_range& e) {
                continue;
            }
        }
        else {
            continue;
        }

        formattedResultMap["remaining_lifetime_percent"] = to_string(remainingLifeTimePercent);
        formattedResult.push_back(formattedResultMap);
    }

    return formattedResult;
}

vector<map<string, string>> SMARTReader::GetDeviceList()
{
    vector<string> result;
    const string command = "smartctl --scan > output.txt";
    system(command.c_str());

    ifstream file("output.txt");
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            result.push_back(line);
        }
        file.close();
    }
    else {
        cerr << "Failed to open output.txt" << endl;
    }
    remove("output.txt");

    vector<map<string, string>> devices;

    for (int i = 0; i < result.size(); ++i) {
        vector<string> localResult = Split(result[i], ' ');
        map<string, string> localMap;
        localMap["name"] = localResult[2];
        localMap["path"] = localResult[0];
        devices.push_back(localMap);
    }

    return devices;
}

vector<string> SMARTReader::Split(const string& str, char delimiter) {
    vector<string> result;
    string token;
    istringstream tokenStream(str);

    // Разделяем строку по разделителю
    while (getline(tokenStream, token, delimiter)) {
        result.push_back(token);
    }

    return result;
}

vector<vector<string>> SMARTReader::ParseLoadedDeviceResultRaw(vector<map<string, string>> devices)
{
    vector<vector<string>> devicesRawResult;

    for (int i = 0; i < devices.size(); i++) {
        vector<string> result;
        string device_path = devices[i]["path"];
        ostringstream command_stream;
        command_stream << "smartctl -a " << device_path << " > output.txt";
        string command = command_stream.str();
        system(command.c_str());

        // Читаем файл
        ifstream file("output.txt");
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                result.push_back(line);
            }
            file.close();
        }
        else {
            cerr << "Failed to open output.txt" << endl;
        }
        devicesRawResult.push_back(result);
    }
    remove("output.txt");

    return devicesRawResult;
}

map<string, string> SMARTReader::ProcessRawDeviceData(vector<string> device)
{
    map<string, string> MapDevice;
    int diskType = SSD;

    for (string line : device)
    {
        diskType = DetermineDiskType(line);
        if (diskType == HDD) {
            break;
        }
    }

    MapDevice["type"] = diskType == HDD ? HDD_LABEL : SSD_LABEL;

    for (string line : device)
    {
        if (line.find(MODEL_PARAM) != string::npos) {
            vector<string> lineMembers = Split(line, ':');
            if (lineMembers.size() > 0) {
                MapDevice["name"] = TrimString(lineMembers[1]);
            }
        }
        if (diskType == HDD) {
            if (line.find(RAW_READ_ERROR_RATE) != string::npos) {
                vector<string> lineMembers = Split(line, ' ');
                if (lineMembers.size() > 0) {
                    MapDevice[RAW_READ_ERROR_RATE] = TrimString(lineMembers[lineMembers.size() - 1]);
                }
            }
            if (line.find(REALLOCATED_SECTOR_CT) != string::npos) {
                vector<string> lineMembers = Split(line, ' ');
                if (lineMembers.size() > 0) {
                    MapDevice[REALLOCATED_SECTOR_CT] = TrimString(lineMembers[lineMembers.size() - 1]);
                }
            }
            if (line.find(SEEK_ERROR_RATE) != string::npos) {
                vector<string> lineMembers = Split(line, ' ');
                if (lineMembers.size() > 0) {
                    MapDevice[SEEK_ERROR_RATE] = TrimString(lineMembers[lineMembers.size() - 1]);
                }
            }
            if (line.find(POWER_ON_HOURS) != string::npos) {
                vector<string> lineMembers = Split(line, ' ');
                if (lineMembers.size() > 0) {
                    MapDevice[POWER_ON_HOURS] = TrimString(lineMembers[lineMembers.size() - 1]);
                }
            }
            if (line.find(HDD_TEMPERATURE_CELSIUS) != string::npos) {
                vector<string> lineMembers = Split(line, ' ');
                if (lineMembers.size() > 0) {
                    MapDevice["Temperature"] = TrimString(lineMembers[lineMembers.size() - 3]);
                    vector<string> splittedMinMax = Split(TrimString(lineMembers[lineMembers.size() - 1]), '/');
                    if (splittedMinMax.size() > 0) {
                        MapDevice["Temperature Min"] = splittedMinMax[0];
                        MapDevice["Temperature Max"] = splittedMinMax[1].erase(splittedMinMax.size(), 1);
                    }
                }
            }
            if (line.find(REALLOCATED_EVENT_COUNT) != string::npos) {
                vector<string> lineMembers = Split(line, ' ');
                if (lineMembers.size() > 0) {
                    MapDevice[REALLOCATED_EVENT_COUNT] = TrimString(lineMembers[lineMembers.size() - 1]);
                }
            }
        }
        else if (diskType == SSD) {
            if (line.find(SSD_TEMPERATURE_CELSIUS_ACTUAL_LABEL) != string::npos) {
                vector<string> lineMembers = Split(line, ' ');
                if (lineMembers.size() > 0) {
                    MapDevice["Temperature"] = TrimString(lineMembers[lineMembers.size() - 2]);
                }
            }
            if (line.find(SSD_PERCENTAGE_USED) != string::npos) {
                vector<string> lineMembers = Split(line, ' ');
                if (lineMembers.size() > 0) {
                    string rawString = TrimString(lineMembers[lineMembers.size() - 1]);
                    MapDevice[SSD_PERCENTAGE_USED] = Split(rawString, '%')[0];
                }
            }
        }
        else {
            break;
        }
    }
    return MapDevice;
}

string SMARTReader::TrimString(string str)
{
    str.erase(str.begin(), find_if(str.begin(), str.end(), [](unsigned char ch) {
                  return !isspace(ch);
              }));

    str.erase(find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
                  return !isspace(ch);
              }).base(), str.end());

    return str;
}
