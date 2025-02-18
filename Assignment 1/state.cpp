#include<unordered_map>
#include<iostream>
#include<random>
#include <ctime>
#include "stdlib.h"
#include <cstdlib>
#include<cmath>
#include<chrono>
#include <fstream>
#include <algorithm>

#include "state.h"
using namespace std;

mt19937 rng;

Allocation::Allocation(string inputfilename){
    
    readInInputFile(inputfilename);
    
    zone = (int * )malloc(sizeof(int)*locations);
    // for(int i=0;i<locations;i++){
    //     zone[i] = -1;
    // }
    // for(int i=1;i<=zones;i++){
    //     ZoneIndex[i] = -1;
    // }
    // for(int i=1;i<=this->locations;i++){
    //     if(i>this->zones){
    //         this->zone[i-1] = -1;
    //     }
    //     else{
    //         this->zone[i-1] = i;
    //         this->ZoneIndex[i] = i;
    //         // cout<<"alloting "<<i<<" "<<ZoneIndex[i]<<endl;
    //     }
    // }
}

void Allocation::readInInputFile(string inputfilename){
    
    fstream ipfile;
    ipfile.open(inputfilename, ios::in);
    if (!ipfile) {
        cout << "No such file\n";
        exit( 0 );
    } else {
        ipfile>> givenTime;
        ipfile >> zones;
        ipfile >> locations;

        if(zones>locations)
        {
            cout<<"Number of zones more than locations, check format of input file\n";
            exit(0);
        }
        
        long long **tempT;
        long long **tempN;
        
        tempT = new long long *[locations];
        for (int i = 0; i < locations; ++i)
            tempT[i] = new long long [locations];
        
        tempN = new long long *[zones];
        for (int i = 0; i < zones; ++i)
            tempN[i] = new long long [zones];
            
        for(int i=0;i<zones;i++)
        {
            for(int j=0;j<zones;j++)
            ipfile>>tempN[i][j];
        }

        for(int i=0;i<locations;i++)
        {
            for(int j=0;j<locations;j++)
            ipfile>>tempT[i][j];
        }
        
        ipfile.close();

        timeMatrix = tempT;
        freqMatrix = tempN;
            
        
    }
}



int Allocation::findLoc(int zone_num){
    return ZoneIndex[zone_num];
}

int Allocation::findZone(int loc_num){
    return zone[loc_num-1];
}

void Allocation::LocSwap(int loc1, int loc2){
    int z1 = this->zone[loc1-1];
    int z2 = this->zone[loc2-1];
    // cout<<"in func "<<z1<<" "<<z2<<endl;
    this->zone[loc1-1] = z2;
    this->zone[loc2-1] = z1;
    if(z2!=-1){ 
        this->ZoneIndex[z2] = loc1; 
    }
    if(z1!=-1){ 
        this->ZoneIndex[z1] = loc2; 
    }
    
    // if (z1 > 0 and z2 == -1) {
    //     cout << "hello\n";
    // }
    // if (z1 == -1 and z2 > 0) {
    //     cout << "hello\n";
    // }
    // if (z1 != -1 and z2 != -1) {
        for (auto& cluster : this->clusters) {
            for (int& loc : cluster) {
                if (loc == loc1) {
                    loc = loc2;
                } else if (loc == loc2) {
                    loc = loc1;
                }
            }
        }
    // }
    // cout << "Cluster Size: " << this->clusters.size() << endl;
    // for (size_t clusterIdx = 0; clusterIdx < this->clusters.size(); ++clusterIdx) {
    //     cout << "Cluster " << clusterIdx << ": ";
    //     for (int locIdx : this->clusters[clusterIdx]) {
    //         cout << locIdx << " ";
    //     }
    //     cout << endl;
    // }
    
}

double usedTime(chrono::steady_clock::time_point start) {
    auto end = chrono::steady_clock::now();
    chrono::duration<double> elapsed_seconds = end - start;
    return elapsed_seconds.count();
}

long long findCost(unordered_map<int,int>ZoneIndex, int *zone, long long ** N, long long ** T){
    long long ans =0;
    for(int i=1;i<=ZoneIndex.size();i++){
        for(int j=1;j<=ZoneIndex.size();j++){
            ans += (long long)N[i-1][j-1]*(long long)T[ZoneIndex[i]-1][ZoneIndex[j]-1];
        }
    }
    return ans;
}

long long Allocation::findDiff(int loc1, int loc2){
    // cout<<"in"<<endl;
    int z1 = zone[loc1-1];
    int z2 = zone[loc2-1];
    // cout<<z1<<" "<<z2<<endl;
    long long pos = 0;
    long long neg = 0;
    for(int i=1; i<=this->zones; i++){
        if(i!=z1 && i!=z2){
            if(z1!=-1){
                pos += (long long)this->freqMatrix[z1-1][i-1]*(long long)this->timeMatrix[loc2-1][ZoneIndex[i]-1]+(long long)this->freqMatrix[i-1][z1-1]*(long long)this->timeMatrix[ZoneIndex[i]-1][loc2-1];
                neg += (long long)this->freqMatrix[z1-1][i-1]*(long long)this->timeMatrix[loc1-1][ZoneIndex[i]-1]+(long long)this->freqMatrix[i-1][z1-1]*(long long)this->timeMatrix[ZoneIndex[i]-1][loc1-1];
            }
            if(z2!=-1){
                pos += (long long)this->freqMatrix[z2-1][i-1]*(long long)this->timeMatrix[loc1-1][ZoneIndex[i]-1]+(long long)this->freqMatrix[i-1][z2-1]*(long long)this->timeMatrix[ZoneIndex[i]-1][loc1-1];
                neg += (long long)this->freqMatrix[z2-1][i-1]*(long long)this->timeMatrix[loc2-1][ZoneIndex[i]-1]+(long long)this->freqMatrix[i-1][z2-1]*(long long)this->timeMatrix[ZoneIndex[i]-1][loc2-1];
            }
        }
    }
    long long diff = pos-neg;
    // cout<<pos<<" "<<neg<<endl;
    // cout<<diff<<endl;
    return diff;
}

// long long Allocation::findDiff(int loc1, int loc2){
//     // If the locations are the same, the difference in cost is 0
//     // cout<<"in--------"<<endl;
//     if (loc1 == loc2) return 0;

//     long long original_cost = 0;
//     long long swapped_cost = 0;
    
//     int zone1 = this->zone[loc1-1]-1;
//     int zone2 = this->zone[loc2-1]-1;
    
//     int *tempZone;
//     tempZone = this->zone;
//     for (int i=0; i < this->locations; i++){
//         cout << zone[i] << " ";
//     }
//     cout << endl;
//     return 0;
//     // for (int i = 0; i < this->zones; i++) {
//     //     // Ignore if i is same as zone1 or zone2
//     //     // cout<<"inside the loop"<<endl;
//     //     if (i == zone1 || i == zone2) continue;

//     //     if(zone1!=-2){        // Cost in original position
//     //     original_cost += (long long)this->freqMatrix[zone1][i] * (long long)this->timeMatrix[loc1-1][this->ZoneIndex[i+1]-1];
//     //     original_cost += (long long)this->freqMatrix[i][zone1] * (long long)this->timeMatrix[this->ZoneIndex[i+1]-1][loc1-1];

//     //     // Cost in swapped position
//     //     swapped_cost += (long long)this->freqMatrix[zone1][i] * (long long)this->timeMatrix[loc2-1][this->ZoneIndex[i+1]-1];
//     //     swapped_cost += (long long)this->freqMatrix[i][zone1] * (long long)this->timeMatrix[this->ZoneIndex[i+1]-1][loc2-1];}
//     // }

//     // // Calculate cost difference for paths involving loc2
//     // for (int i = 0; i < this->zones; i++) {
//     //     // Ignore if i is same as zone1 or zone2
//     //     if (i == zone1 || i == zone2) continue;
//     //     if(zone2!=-2){
//     //     original_cost += (long long)this->freqMatrix[zone2][i] * (long long)this->timeMatrix[loc2-1][this->ZoneIndex[i+1]-1];
//     //     original_cost += (long long)this->freqMatrix[i][zone2] * (long long)this->timeMatrix[this->ZoneIndex[i+1]-1][loc2-1];

//     //     // Cost in swapped position
//     //     swapped_cost += (long long)this->freqMatrix[zone2][i] * (long long)this->timeMatrix[loc1-1][this->ZoneIndex[i+1]-1];
//     //     swapped_cost += (long long)this->freqMatrix[i][zone2] * (long long)this->timeMatrix[this->ZoneIndex[i+1]-1][loc1-1];}
//     // }

//     // return swapped_cost - original_cost;
    
   
// }

long long Allocation::findScore(){
    long long ans =0;
    for(int i=1;i<=this->zones;i++){
        for(int j=1;j<=this->zones;j++){
            ans += (long long)this->freqMatrix[i-1][j-1]*(long long)this->timeMatrix[(this->ZoneIndex[i])-1][(this->ZoneIndex[j])-1];
            // cout<<"In score "<<i<<" "<<j<<" "<<this->ZoneIndex[i]<<" "<<this->ZoneIndex[j]-1<<" "<<this->freqMatrix[i-1][j-1]<<" "<<this->timeMatrix[this->ZoneIndex[i]-1][this->ZoneIndex[j]-1]<<endl;
        }
    }
    return ans;
}

void Allocation::createStart(chrono::steady_clock::time_point start){
    // srand(time(nullptr));

    double t = this->givenTime*60.0;
    
    long long change = 0;
    bool flag = false; 
    int l1=0;
    int l2=0;
    while(true){
        if(usedTime(start)>=t){
            // cout<<"here"<<endl;
            break;
        }
        
        // long long curr_cost =this->findScore();
        l1 = rand()%(this->locations);
        l2 = rand()%(this->locations);
        l1++;
        l2++;
        // dist.reset();
        // cout<<"Selected for swapping"<<l1<<" "<<l2<<endl;
        // if(l1 == l2){ continue;}
        if(this->zone[l1]==-1 && this->zone[l2]==-1){ continue;}
        change = this->findDiff(l1,l2);
        
        if(change<0){
            // cout<<"Change is "<<change<<endl;
            flag = true;
            // cout << "Change is "<<change<< ": " << this->findScore();
            // this->LocSwap(l1,l2);
            // cout<< " , " << this->findScore() << endl;
            break;
        }
        double t = this->givenTime*60*0.99898;
        if(usedTime(start) > t*0.001) {
            break;
        }
    }
    // cout<<"done"<<endl;
    // cout<<l1<<" "<<l2<<endl;
    if(flag==true){
        this->LocSwap(l1,l2);
    }
    return;
}

long long Allocation::start_state(chrono::steady_clock::time_point start){
    
    vector<int> unallocatedZones;
    vector<int> unallocatedLocations;
    
    for (int i = 1; i <= this->zones; ++i) {
        unallocatedZones.push_back(i);
    }
    for (int i = 1; i <= this->locations; ++i) {
        unallocatedLocations.push_back(i);
    }
    
    sort(unallocatedZones.begin(), unallocatedZones.end(), 
              [&](int zone1, int zone2) {
                  long long freq1 = 0, freq2 = 0;
                  for (int i = 1; i <= this->zones; ++i) {
                      freq1 += this->freqMatrix[i - 1][zone1 - 1];
                      freq2 += this->freqMatrix[i - 1][zone2 - 1];
                  }
                  return freq1 < freq2;
              });
              
   
    sort(unallocatedLocations.begin(), unallocatedLocations.end(),
              [&](int loc1, int loc2) {
                  long long time1 = 0, time2 = 0;
                  for (int i = 1; i <= this->locations; ++i) {
                      time1 += this->timeMatrix[i - 1][loc1 - 1];
                      time2 += this->timeMatrix[i - 1][loc2 - 1];
                  }
                  return time1 < time2;
              });
    
    // int c = 0;  
    // for (int i: unallocatedLocations) {
    //     this->timeLocations[i] = c;
    //     c++;
    // }
    
    // for(const auto& elem : this->timeLocations){
    //     cout << "Loaction: " << elem.first << ", Situated: " << elem.second << endl;        
    // }
    
    for (int i = 0; i < this->locations; ++i) {
        
        int location = unallocatedLocations[i];
        if(i>=this->zones) {
            this->zone[location-1] = -1;
        } else {
            int zone = unallocatedZones.back();
            unallocatedZones.pop_back();
            this->ZoneIndex[zone] = location;
            this->zone[location-1] = zone;
        }
    }    
    
    // for(const auto& elem : this->ZoneIndex){
    //     cout << "Zone: " << elem.first << ", Location: " << elem.second << endl;        
    // }
    // cout << "Done" << endl;
    
    return this->findScore();
}


long long Allocation::distance(int loc1, int loc2) {
    return this->timeMatrix[loc1-1][loc2-1];
}

void Allocation::clusterLocations(long long proximityThreshold, vector<int> unallocatedLocations) {
    
    // vector<int> unallocatedLocations;
    
    // for (int i = 1; i <= this->locations; ++i) {
    //     unallocatedLocations.push_back(i);
    // }
    
    // sort(unallocatedLocations.begin(), unallocatedLocations.end(),
    //           [&](int loc1, int loc2) {
    //               long long time1 = 0, time2 = 0;
    //               for (int i = 1; i <= this->locations; ++i) {
    //                   time1 += this->timeMatrix[i - 1][loc1 - 1];
    //                   time2 += this->timeMatrix[i - 1][loc2 - 1];
    //               }
    //               return time1 < time2;
    //           });
    
    this->clusters.clear();
    // cout << this->clusters.size() << endl;
    for (int locIdx: unallocatedLocations) {
        bool addedToCluster = false;
        
        for (auto& cluster : this->clusters) {
            if (any_of(cluster.begin(), cluster.end(), [&](int otherLocIdx) {
                return distance(locIdx, otherLocIdx) <= proximityThreshold;
            })) {
                cluster.push_back(locIdx);
                addedToCluster = true;
                break;
            }
        }

        if (!addedToCluster) {
            this->clusters.push_back({locIdx});
        }
    }
    
    sort(this->clusters.begin(), this->clusters.end(), [&](const vector<int>& cluster1, const vector<int>& cluster2) {
        return cluster1.size() > cluster2.size();
    });
    
    // cout << "Cluster Size: " << this->clusters.size() << endl;
    // for (size_t clusterIdx = 0; clusterIdx < this->clusters.size(); ++clusterIdx) {
    //     cout << "Cluster " << clusterIdx << ": ";
    //     for (int locIdx : this->clusters[clusterIdx]) {
    //         cout << locIdx << " ";
    //     }
    //     cout << endl;
    // }

    return;
}

long long Allocation::cluster_start(chrono::steady_clock::time_point start) {
    
    vector<int> unallocatedZones;
    
    for (int i = 1; i <= this->zones; ++i) {
        unallocatedZones.push_back(i);
    }
    
    sort(unallocatedZones.begin(), unallocatedZones.end(), 
              [&](int zone1, int zone2) {
                  long long freq1 = 0, freq2 = 0;
                  for (int i = 1; i <= this->zones; ++i) {
                      freq1 += this->freqMatrix[i - 1][zone1 - 1];
                      freq2 += this->freqMatrix[i - 1][zone2 - 1];
                  }
                  return freq1 < freq2;
              });
   
    
    int z = 0;
    bool flag = false;
    
    for (int i=0; i<this->locations; i++){
        this->zone[i] = -1;
    }
    
    for (auto& cluster : this->clusters) {
        // shuffle(cluster.begin(), cluster.end(), rng);
        // random_shuffle(cluster.begin(), cluster.end());
        for (int i = 0; i < cluster.size(); ++i) {
            if (z < this->zones) {
                // cout << "number: " << z << endl;
                int temp = unallocatedZones.back();
                unallocatedZones.pop_back();
                // cout<< "Sorted Zones: ";
                // for(int i: unallocatedZones) {
                //     cout << i << " ";
                // }
                // cout << endl;
                this->ZoneIndex[temp] = cluster[i];
                this->zone[cluster[i]-1] = temp;
                z++;
            } else {
                flag = true;
                // cout << "pipe broke\n";
                break;
            }
        }
        if (flag == true) {
            // cout << "pipe broke\n";
            break;
        }
    }
    
    // for(const auto& elem : this->ZoneIndex){
    //     cout << "Zone: " << elem.first << ", Location: " << elem.second << endl;        
    // }
    
    // for (int i=1; i <= this->zones; i++) {
    //     cout << "Zone: " << i << ", Location: " << ZoneIndex[i] << endl;
    // }
    // for (int i=0; i<this->locations; i++) {
    //     cout << this->zone[i] << " ";
    // }
    // cout << endl;
    
    // for (int i : this->zoneSorted) {
    //     for (auto& cluster : clusters) {
            
    //     }
    // }
    
    return this->findScore();
}



long long Allocation::neighborFunction(chrono::steady_clock::time_point start, long long proximityThreshold) {
    srand(time(nullptr));
    long long change = 0;
    bool flag = false;
    double t = givenTime*60.0;
    // random_shuffle(clusters.begin(), clusters.end());
    // for (const auto& cluster: clusters) {
    //     cout << "Cluster: ";
    //     for (int i: cluster) {
    //         cout << i << " ";
    //     }
    //     cout << endl;
    // }
    for (const auto& cluster : this->clusters) {
        int len = cluster.size();
        vector<int> cluster1 = cluster;
        random_device rd;
        mt19937 rng(rd());
        shuffle(cluster1.begin(), cluster1.end(),rng);
        // random_shuffle(cluster1.begin(), cluster1.end());
        // cout << "Cluster1: ";
        // for (int i : cluster1) {
        //     cout << i << " "; 
        // }
        // cout << endl;
        
        vector<int> cluster2 = cluster;
        
        shuffle(cluster2.begin(), cluster2.end(),rng);
        // random_shuffle(cluster2.begin(), cluster2.end());
        // cout << "Cluster2: ";
        // for (int i : cluster2) {
        //     cout << i << " "; 
        // }
        // cout << endl;
        int max_swaps = len / 2;
        int swaps = 0;
        bool s = false;
        for (int loc1 : cluster1) {
            for (int loc2 : cluster2) {
                if (loc1 != loc2) {
                    if(zone[loc1]==-1 && zone[loc2]==-1){ continue; }
                    
                    // if (swaps == max_swaps) {
                    //     s = true;
                    //     break;
                    // }
                    
                    change = this->findDiff(loc1,loc2);
                    swaps++;
                    // cout<<"ajter"<<endl;
                    if(change<0){
                        // cout<<"change is negative"<<endl;
                        // cout << "Change: " << change <<" " << this->findScore(); 
                        this->LocSwap(loc1,loc2);
                        // cout << " , " << this->findScore() << endl;
                        flag = true;
                        break;
                    }
                }
                // double t = givenTime*60.0;
                if(usedTime(start)>=t){
                    break;
                }
            }
            
            // if (s == true) {
            //     break;
            // }
            
            if(flag==true){
                break;
            }
            // double t = givenTime*60.0;
            if(usedTime(start)>=t){
                break;
            }
        }
        if(flag==true){
            break;
        }
        // double t = givenTime*60.0;
        if(usedTime(start)>=t){
            break;
        }
    }
    
    // double t = givenTime*60.0;
    // for (const auto& cluster : clusters) {
    //     int len = cluster.size();
    //     cout << "cluster\n";
    //     int swaps = 0;
    //     int max_swaps = len / 2;
    //     while (true) {
    //         int ind1 = rand()%len;
    //         int ind2 = rand()%len;
    //         if (ind1 != ind2) {
    //             int loc1 = cluster[ind1];
    //             int loc2 = cluster[ind2];
                
    //             if (swaps == max_swaps) {
    //                 break;
    //             }
                
    //             change = this->findDiff(loc1,loc2);
    //             swaps++;
    //             if (change < 0) {
    //                 cout << "hello\n";
    //                 this->LocSwap(loc1,loc2);
    //                 flag = true;
    //                 break;
    //             }
    //         }
            
            
    //         // if (usedTime(start) >= t*(len/this->locations)) {
    //         //     break;
    //         // }
    //     }
    //     if (flag == true) {
    //         break;
    //     }
    //     if (usedTime(start) >= t) {
    //         break;
    //     }
        
        
    // }
    
    if(change>=0){  // Local Maxima
        srand(time(nullptr));
        // cout<<"entered"<<endl;
        long long oldCost = this->findScore();
        // cout<<"old cose "<<oldCost<<endl;
        
        while(true){
            vector<int> tempLocations;
            // srand(time(nullptr));
            double t = givenTime*60.0*0.8888999;
            if(usedTime(start)>=t){
                // cout<<"time exceeded------------------------------------------"<<endl;
                // cout<<"done done done";
                // return this->findScore();
                break;

            }
            vector<bool>available(this->locations,true);
            // default_random_engine generator;
            // uniform_int_distribution<mt19937::result_type> dist(1,this->zones+1);
            unordered_map<int,int> newZoneIndex;
            int *Newzone = (int * )malloc(sizeof(int)*locations);
            for(int i=0;i<this->locations;i++){
                Newzone[i]=-1;
            }
            for(int i=1;i<this->zones+1;i++){
                int newL = rand()%(this->locations);
                newL++;
                if(available[newL-1]==true){
                    Newzone[newL-1] = i;
                    newZoneIndex[i] = newL;
                    available[newL-1]=false;
                    tempLocations.push_back(newL);
                }
                else{
                    i--;
                }
            }
            for(int j=0; j < this->locations; j++) {
                if (available[j] == true) {
                    tempLocations.push_back(j+1);
                }
            }
            
            // for(int i=0;i<this->locations;i++){
            //     // cout<<Newzone[i]<<" ";
            // }
            // cout<<endl;
            long long newCost = findCost(newZoneIndex,Newzone, this->freqMatrix, this->timeMatrix);
            // cout<<"Newcost is "<<newCost<<endl;
            change = newCost-oldCost;
            if(change<0){
                free(this->zone);
                this->ZoneIndex.clear();
                this->zone = Newzone;
                this->ZoneIndex = newZoneIndex; 
                // cout << "temp: ";
                // for (int i: tempLocations) {
                //     cout << i << " "; 
                // }
                // cout << endl;
                this->clusterLocations(proximityThreshold, tempLocations);
                // cout << "Cluster Size: " << this->clusters.size() << endl;
                // for (size_t clusterIdx = 0; clusterIdx < this->clusters.size(); ++clusterIdx) {
                //     cout << "Cluster " << clusterIdx << ": ";
                //     for (int locIdx : this->clusters[clusterIdx]) {
                //         cout << locIdx << " ";
                //     }
                //     cout << std::endl;
                // }
                break;
            }
            else{
                free(Newzone);
                newZoneIndex.clear();
            }
        }
    }
    // cout<<"broken"<<endl;
    return this->findScore();
    
    
}



long long Allocation::findSuccessor(chrono::steady_clock::time_point start){
    long long change = 0;
    bool flag = false;
    for(int i=0;i<this->locations;i++){
        for(int j=0;j<this->locations;j++){
            if (i != j) {
                if(zone[i]==-1 && zone[j]==-1){ continue; }
                change = this->findDiff(i+1,j+1);
                // cout<<"ajter"<<endl;
                if(change<0){
                    // cout<<"change is negative"<<endl;
                    this->LocSwap(i+1,j+1);
                    flag = true;
                    break;
                }
            }
            double t = givenTime*60.0;
            if(usedTime(start)>=t){
                break;
            }
            // cout<<i<<" "<<j<<endl;
            // cout<<"at the end"<<endl;
            
        }
        if(flag==true){
            break;
        }
        double t = givenTime*60.0;
        if(usedTime(start)>=t){
            break;
        }
    }
    //time elapsed missing in code below
    if(change>=0){  // Local Maxima
        srand(time(nullptr));
        // cout<<"entered"<<endl;
        long long oldCost = this->findScore();
        // cout<<"old cose "<<oldCost<<endl;
        while(true){
            // srand(time(nullptr));
            double t = givenTime*60.0*0.8888999;
            if(usedTime(start)>=t){
                // cout<<"time exceeded------------------------------------------"<<endl;
                // cout<<"done done done";
                // return this->findScore();
                break;

            }
            vector<bool>available(this->locations,true);
            // default_random_engine generator;
            // uniform_int_distribution<mt19937::result_type> dist(1,this->zones+1);
            unordered_map<int,int> newZoneIndex;
            int *Newzone = (int * )malloc(sizeof(int)*locations);
            for(int i=0;i<this->locations;i++){
                Newzone[i]=-1;
            }
            for(int i=1;i<this->zones+1;i++){
                int newL = rand()%(this->locations);
                newL++;
                if(available[newL-1]==true){
                    Newzone[newL-1] = i;
                    newZoneIndex[i] = newL;
                    available[newL-1]=false;
                }
                else{
                    i--;
                }
            }
            // for(int i=0;i<this->locations;i++){
            //     // cout<<Newzone[i]<<" ";
            // }
            // cout<<endl;
            long long newCost = findCost(newZoneIndex,Newzone, this->freqMatrix, this->timeMatrix);
            // cout<<"Newcost is "<<newCost<<endl;
            change = newCost-oldCost;
            if(change<0){
                this->zone = Newzone;
                this->ZoneIndex = newZoneIndex; 
                break;
            }
            else{
                free(Newzone);
                newZoneIndex.clear();
            }
        }
    }
    // cout<<"broken"<<endl;
    return this->findScore();
}


void Allocation::explorer(chrono::steady_clock::time_point start, long long proximityThreshold){
    long long curr_cost = this->findScore();
    // cout<<"original cost is "<<curr_cost<<endl;
    long long succ_cost = 0;
    double t = givenTime*60.0;
    while(true){
        succ_cost = this->neighborFunction(start, proximityThreshold);
        if(usedTime(start)>=t){
            // cout<<"ïn time limit"<<endl;
            break;
        }
        if(succ_cost<curr_cost){
            // cout<<"in update"<<endl;
            curr_cost = succ_cost; 
        }
    }
    // cout<<"now the cost "<<curr_cost<<endl;
}

void Allocation::allot(chrono::steady_clock::time_point start){
    double t = givenTime*60.0*0.99898;
    
    vector<int> unallocatedLocations;
    
    for (int i = 1; i <= this->locations; ++i) {
        unallocatedLocations.push_back(i);
    }
    
    sort(unallocatedLocations.begin(), unallocatedLocations.end(),
              [&](int loc1, int loc2) {
                  long long time1 = 0, time2 = 0;
                  for (int i = 1; i <= this->locations; ++i) {
                      time1 += this->timeMatrix[i - 1][loc1 - 1];
                      time2 += this->timeMatrix[i - 1][loc2 - 1];
                  }
                  return time1 < time2;
              });
    long long totalDistance = 0;
    for (int i = 0; i < this->locations; ++i) {
        for (int j = 0; j < this->locations; ++j) {
            totalDistance += timeMatrix[i][j];
        }
    }
    long long avgDistance = totalDistance / (this->locations * (this->locations - 1));
    long long thresholdPercentage = 80; // Adjust as needed
    long long proximityThreshold = (avgDistance * thresholdPercentage) / 100;
    // cout << "proximityThreshold: " << proximityThreshold << endl;
    this->clusterLocations(proximityThreshold, unallocatedLocations);
    // cout << "Cluster Size: " << this->clusters.size() << endl;
    // for (size_t clusterIdx = 0; clusterIdx < this->clusters.size(); ++clusterIdx) {
    //     cout << "Cluster " << clusterIdx << ": ";
    //     for (int locIdx : this->clusters[clusterIdx]) {
    //         cout << locIdx << " ";
    //     }
    //     cout << std::endl;
    // }
    
    long long x = this->cluster_start(start);
    // long long x = this->start_state(start);
    // cout << "Start Cost: " << x << endl;
    while(true){
        // cout<<"Outer while loop started-------------------------------------------------------------------------"<<endl;

        this->createStart(start);

        if(usedTime(start)>=0.01*t){
            // cout<<"Outer while loop started-------------------------------------------------------------------------"<<endl;
            
            break;
        } 
    }
    
    // cout << "Latest Score: " << this->findScore() << endl;
    

    // long long x = this->cluster_start(start, clusters);
    // cout<<"out of loop"<<endl;
    this->explorer(start, proximityThreshold);
    return;
}

void Allocation::readOutputFile(string output_filename){
    fstream ipfile;
    ipfile.open(output_filename, ios::in);
    if (!ipfile) {
        cout << "No such file\n";
        exit( 0 );
    }   
    else {
        vector<int> ip;
        while (1) {
            int t;
            ipfile >> t;
            // cout<<"content is "<<t<<endl;
            ip.push_back(t);
            if (ipfile.eof())
                break;
        }
    // cout<<ip.size()<<" "<<this->zones+1<<" "<<"check here"<<endl;
    if(ip.size()!=this->zones+1){
        cout<<"number of values not equal to number of zones, check output format\n";
        exit(0);
    }
    for(int i=0;i<this->zones;i++) this->ZoneIndex[i]=ip[i];
    ipfile.close();
    if(!check_output_format())
        exit(0);
    cout<<"Read output file, format OK"<<endl;
    } 
}

void Allocation::write_to_file(string outputfilename){
    // Open the file for writing
    ofstream outputFile(outputfilename);
    // Check if the file is opened successfully
    if (!outputFile.is_open()) {
        cerr << "Failed to open the file for writing." << std::endl;
        exit(0);
    }
    for(int i=1;i<=this->zones;i++)
    outputFile<<this->ZoneIndex[i]<<" ";
    // Close the file
    outputFile.close();
    cout << "Allocation written to the file successfully." << endl;
}

bool Allocation::check_output_format(){
    vector<bool> visited(this->locations,false);
    for(int i=0;i<this->zones;i++){
        if((ZoneIndex[i]>=1 && ZoneIndex[i]<=this->locations)){
            if(!visited[ZoneIndex[i]-1])
            visited[this->ZoneIndex[i]-1]=true;
            else{
                cout<<"Repeated locations, check format\n";
                return false;
            }
        }
        else{
            cout<<"Invalid location, check format\n";
            return false;
        }
    }
    return true;
}


