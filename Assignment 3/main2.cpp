#include <bits/stdc++.h>
using namespace std;

vector< vector<int> > output;

int main(int argc, char** argv){
	ifstream file;
	file.open(argv[1]); //sat output
	if(!file.is_open()){
		cerr << "file not present" << endl;
		exit(-1);
	}
	int N,E,K1,K2;
	cin >> N >> E >> K1 >> K2;

	string result;
	file >> result;
	if(result.compare("UNSAT") == 0){
		cout << "0" << endl;
		file.close();
		return 0;
	}

	output.resize(2);
	getline(file,result);
	getline(file,result);
	stringstream pro(result);
	int temp;
	for(int i = 1 ; i <= N ; i++){
		 for(int j = 1 ; j <= 2 ; j++){
		 	pro >> temp;
		 	if(temp > 0){
		 		output[j-1].push_back(i);
		 	}
		 }
	}

	for(int i=0; i<output.size(); i++){
		cout << "#" << i+1 <<endl;
		for(int j=0; j<output[i].size(); j++){
			cout << output[i][j];
			if(j < output[i].size()-1)
				cout << " ";
		}
		cout << endl;
	}
}
