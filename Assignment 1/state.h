#ifndef STATE_H
#define STATE_H
#include<unordered_map>
#include<iostream>
#include<chrono>
#include <fstream>
using namespace std;

class Allocation{
    private:
    unordered_map<int,int> ZoneIndex;  // given a zone number outputs its location number
    
    int zones;  // number of zones
    int locations;  // number of locations
    long long ** timeMatrix;  // T - Tij - time taken to walk from location i to j
    long long ** freqMatrix;  // N - Nxy - number of daily walks from zone x to y
    // long long *freqZones;
    // vector<int> zoneSorted;
    // unordered_map<int,int> timeLocations;
    float givenTime;  // max time given
    int *zone;  // given a location number outputs its zone number
    vector<vector<int>> clusters;

    public:
    Allocation(string inputfilename); 
    void readInInputFile(string inputfilename);
    // void write_to_file(string outputfilename);
    int findLoc(int zone);
    int findZone(int location);
    void LocSwap(int loc1, int loc2);
    void createStart(chrono::steady_clock::time_point start); //time
    long long start_state(chrono::steady_clock::time_point start);
    long long distance(int loc1, int loc2);
    void clusterLocations(long long proximityThreshold, vector<int> unallocatedLocations);
    long long cluster_start(chrono::steady_clock::time_point start);
    long long findDiff(int location1, int location2);
    long long findSuccessor(chrono::steady_clock::time_point start); //time
    long long neighborFunction(chrono::steady_clock::time_point start, long long proximityThreshold);
    // long long findNeighbor(chrono::steady_clock::time_point start);
    long long findScore();
    void explorer(chrono::steady_clock::time_point start, long long proximityThreshold); //time
    void allot(chrono::steady_clock::time_point start);
    bool check_output_format();
    void write_to_file(string outputfilename);
    void readOutputFile(string output_filename);
};
#endif