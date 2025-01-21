#ifndef SMARTREADER_H
#define SMARTREADER_H

#include <iostream>
#include <map>
#include <vector>

using namespace std;

class SMARTReader
{
public:
    vector<map<string, string>> SmartData;
    int POWER_ON_HOURS_VALUE = 30000;

    SMARTReader();

private:
    int DetermineDiskType(string deviceType);

    vector<map<string, string>> LoadSmartData();

    vector<map<string, string>> GetDeviceList();

    vector<string> Split(const string& str, char delimiter);

    vector<vector<string>> ParseLoadedDeviceResultRaw(vector<map<string, string>> devices);

    map<string, string> ProcessRawDeviceData(vector<string> device);

    string TrimString(string str);
};

#endif // SMARTREADER_H
