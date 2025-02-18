#include<iostream>
#include<sstream>
#include<vector>
#include<fstream>
#include<utility>
#include<map>
using namespace std;


pair<int,int> edge;
vector<pair<int,int>> get_edge;
map<pair<int,int>, int> get_edge_id;
long long N,E,K1,K2;
int get_edge_var(int e, int k){
    return ((e-1)*2+k+N*2);
}
int get_node_var(int n, int k){
    return ((n-1)*2 + k);
}
void get_sorted_edge(pair<int,int>& e){
    if( e.first>e.second ) swap(e.first,e.second); 
}

int main(int argc, char* argv[]){
    if(argc<2){
        cerr<<"give the file name"<<endl;
        return 1;
    }
    string inputFileName = argv[1];
    string outputFileName = argv[2];

    ifstream inputFile(inputFileName);
    ofstream outputFile(outputFileName);

    if(!inputFile){
        cerr<<"Error: Unable to open input file: "<< inputFileName<<endl;
        return 1;
    }
    if(!outputFile){
        cerr<<"Error: Unable to open output file: "<<outputFileName<<endl;
        return 1;
    }
    string line;
    if(getline(inputFile,line)){
        istringstream iss(line);
        if(!(iss>>N>>E>>K1>>K2)){
            cerr<<"Error: Invalid input format in the first line."<<endl;
            return 1;
        }
    }
    else{
        cerr<<"Error: Empty input file."<<endl;
        return 1;
    }
    get_edge.resize(E+1);
    int k = 1;
    while(getline(inputFile, line)){
        long long node1, node2;
        istringstream iss(line);
        if(iss>>node1>>node2){
            pair<int,int> e;
            e.first = node1;
            e.second = node2;
            get_sorted_edge(e);
            get_edge_id[e] = k;
            get_edge[k] = e; 
            k++;
        }
        else{
            cerr<<"Error: Invalid input format in edge line"<<endl;
            return 1;
        }
        
    }
    int variables = 2*N + 2*E + (N+1)*(K1+2) + (N+1)*(K2+2);
    int clauses = N + E + 2*E + 4*E + 2*N*(K1+K2) - 3*(K1+K2) + 2*N -2 + 4*N*N - 2*N*(K1+K2) - 6*N + 3*(K1+K2) + 2*N  - 2;

    outputFile<<"p cnf "<<variables<<" "<<clauses<<endl;

    //constraint 1 : each node can be atmost in one clique
    for(int i=1;i<=N;i++){
        outputFile<<"-"<<get_node_var(i,1)<<" -"<<get_node_var(i,2)<<" 0"<<endl;
    }

    // //constraint 3 : if 2 nodes are in a clique then corresponding edge must also be in the clique
    for(int n1=1;n1<N;n1++){
        for(int n2=n1+1;n2<=N;n2++){
            pair<int,int> e = make_pair(n1,n2);
            if(get_edge_id.find(e) == get_edge_id.end()){
                //constraint 3.1 : if edge between 2 nodes doesn't exist, they can't be in the same clique
                // outputFile<<"edge doesn't exist between ---- "<<n1<<" "<<n2<<endl;
                outputFile<<"-"<<get_node_var(n1,1)<<" -"<<get_node_var(n2,1)<<" 0"<<endl;
                outputFile<<"-"<<get_node_var(n1,2)<<" -"<<get_node_var(n2,2)<<" 0"<<endl;
            }
        }
    }

    //constraint 4: clique k1 should have exact size k1
    int s = 2*N + 2*E + 1;  //s00 = 2*N+2*E+1
        //constraint 4.1
    for(int j=1; j<=K1; j++){
        outputFile<<"-"<<s+j<<" 0"<<endl;
    }
    for(int i=0;i<=N;i++){
        outputFile<<s+(i)*(K1+2)<<" 0"<<endl;
    }
        //constraint 4.2
    int s1 = s + K1+1;
    for(int i=1; i<=N; i++){
        for(int j=1; j<=K1; j++){
            outputFile<<"-"<<s1+(i-1)*(K1+2)+j+1<<" "<<s1+(i-2)*(K1+2)+j+1<<" "<<get_node_var(i,1)<<" 0"<<endl;
            outputFile<<"-"<<s1+(i-1)*(K1+2)+j+1<<" "<<s1+(i-2)*(K1+2)+j+1<<" "<<s1+(i-2)*(K1+2)+j<<" 0"<<endl;
            outputFile<<"-"<<s1+(i-2)*(K1+2)+j+1<<" "<<s1+(i-1)*(K1+2)+j+1<<" 0"<<endl;
            outputFile<<"-"<<get_node_var(i,1)<<" -"<<s1+(i-2)*(K1+2)+j<<" "<<s1+(i-1)*(K1+2)+j+1<<" 0"<<endl;
        }
    }
        //constraint 4.3
    outputFile<<s1+(N-1)*(K1+2)+K1+1<<" 0"<<endl;
        //constraint 4.4
    outputFile<<"-"<<s1+(N-1)*(K1+2)+K1+1+1<<" 0"<<endl;

    //constraint 5: clique k2 should have exact size k2
    int s2 = 2*N + 2*E + (N+1)*(K1+2) + 1; //s00 = 2*N + 2*E + (N+1)*(K1+2) + 1
        //constraint 5.1
    for(int j=1; j<=K2; j++){
        outputFile<<"-"<<s2+j<<" 0"<<endl;
    }
    for(int i=0;i<=N;i++){
        outputFile<<s2+(i)*(K2+2)<<" 0"<<endl;
    }
        //constraint 5.2
    int s3 = s2 + K2 + 1;
    for(int i=1; i<=N; i++){
        for(int j=1; j<=K2; j++){
            outputFile<<"-"<<s3+(i-1)*(K2+2)+j+1<<" "<<s3+(i-2)*(K2+2)+j+1<<" "<<get_node_var(i,2)<<" 0"<<endl;
            outputFile<<"-"<<s3+(i-1)*(K2+2)+j+1<<" "<<s3+(i-2)*(K2+2)+j+1<<" "<<s3+(i-2)*(K2+2)+j<<" 0"<<endl;
            outputFile<<"-"<<s3+(i-2)*(K2+2)+j+1<<" "<<s3+(i-1)*(K2+2)+j+1<<" 0"<<endl;
            outputFile<<"-"<<get_node_var(i,2)<<" -"<<s3+(i-2)*(K2+2)+j<<" "<<s3+(i-1)*(K2+2)+j+1<<" 0"<<endl;
        }
    }
        //constraint 5.3
    outputFile<<s3+(N-1)*(K2+2)+K2+1<<" 0"<<endl;
        //constraint 5.4
    outputFile<<"-"<<s3+(N-1)*(K2+2)+K2+1+1<<" 0"<<endl;
    return 0;
}





