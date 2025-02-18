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
int get_edge_var(int e){
	return (e+N);
}
int get_node_var(int n){
	return (n);
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
		if(!(iss>>N>>E)){
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
	

	//intermediateFile1 << "This is data for intermediate file 1." << std::endl;
	

	int low = 1;
	int high = N+1;
	int K1 = 0;
	while(low+1!=high){
	
	std::ofstream intermediateFile1("satinput.txt", std::ios::out);

	if (!intermediateFile1.is_open()) {
		std::cerr << "Failed to open intermediate1.txt for writing." << std::endl;
		return 1;
	}	
	K1 = (low+high)/2;
	cout<<low<<"  "<<high<<"  "<<" "<<K1<<endl;
	int variables = N + E + (N+1)*(K1+2) ;
	int clauses = N*(N-1) - 2*E + K1 + (N+1) + 4*N*K1 + 2 ;

	intermediateFile1<<"p cnf "<<variables<<" "<<clauses<<endl;


		for(int n1=1;n1<N;n1++){
		for(int n2=n1+1;n2<=N;n2++){
			pair<int,int> e = make_pair(n1,n2);
			if(get_edge_id.find(e) == get_edge_id.end()){
				//constraint 3.1 : if edge between 2 nodes doesn't exist, they can't be in the same clique
				// outputFile<<"edge doesn't exist between ---- "<<n1<<" "<<n2<<endl;
				intermediateFile1<<"-"<<get_node_var(n1)<<" -"<<get_node_var(n2)<<" 0"<<endl;
			//	outputFile<<"-"<<get_node_var(n1,2)<<" -"<<get_node_var(n2,2)<<" 0"<<endl;
			}
		}
	}

	//constraint 4: clique k1 should have exact size k1
	int s = N + E + 1;  //s00 = 2*N+2*E+1
		//constraint 4.1
	for(int j=1; j<=K1; j++){
		intermediateFile1<<"-"<<s+j<<" 0"<<endl;
	}
	for(int i=0;i<=N;i++){
		intermediateFile1<<s+(i)*(K1+2)<<" 0"<<endl;
	}
		//constraint 4.2
	int s1 = s + K1+1;
	for(int i=1; i<=N; i++){
		for(int j=1; j<=K1; j++){
			intermediateFile1<<"-"<<s1+(i-1)*(K1+2)+j+1<<" "<<s1+(i-2)*(K1+2)+j+1<<" "<<get_node_var(i)<<" 0"<<endl;
			intermediateFile1<<"-"<<s1+(i-1)*(K1+2)+j+1<<" "<<s1+(i-2)*(K1+2)+j+1<<" "<<s1+(i-2)*(K1+2)+j<<" 0"<<endl;
			intermediateFile1<<"-"<<s1+(i-2)*(K1+2)+j+1<<" "<<s1+(i-1)*(K1+2)+j+1<<" 0"<<endl;
			intermediateFile1<<"-"<<get_node_var(i)<<" -"<<s1+(i-2)*(K1+2)+j<<" "<<s1+(i-1)*(K1+2)+j+1<<" 0"<<endl;
		}
	}
		//constraint 4.3
	intermediateFile1<<s1+(N-1)*(K1+2)+K1+1<<" 0"<<endl;
		//constraint 4.4
	intermediateFile1<<"-"<<s1+(N-1)*(K1+2)+K1+1+1<<" 0"<<endl;
intermediateFile1.close();
const char* command = "./minisat satinput.txt out.txt";
	system(command);
	std::ifstream satoutFile("out.txt");

	if (!satoutFile.is_open()) {
		std::cerr << "Error: Unable to open out.txt for reading." << std::endl;
		return 1;
	}

	std::string firstLine;
	if (std::getline(satoutFile, firstLine)) {
		// Print the first line to the terminal
		//std::cout << "First line of out.txt: " << firstLine << std::endl;
		if(firstLine=="UNSAT"){
		high = K1;
		}
		else{
		low = K1;
		std::ofstream maxKFile("maxKFile.txt");
		if (!maxKFile.is_open()) {
		std::cerr << "Error: Unable to open the maxKFile file for writing." << std::endl;
		return 1;
	}

	std::string fileContent;
	std::string line;
	fileContent+= firstLine + '\n';
	while (std::getline(satoutFile, line)) {
		fileContent += line + '\n'; // Append each line to the fileContent string
	}

	//satoutFile.close(); // Close the input file

	// Write the content to the output file
	maxKFile << fileContent;
	//cout<<"FILECONTENT "<< fileContent<<endl;

	maxKFile.close(); 
		}
	} else {
		std::cerr << "Error: out.txt is empty." << std::endl;
	}

	// Close the input file
	satoutFile.close();
}


	if(low==1){
	outputFile<<"0"<<endl;
}
	else{
	vector<int> output;
	std::ifstream maxKFile("maxKFile.txt");

	if (!maxKFile.is_open()) {
		std::cerr << "Error: Unable to open out.txt for reading." << std::endl;
		return 1;
	}

	//std::string firstLine;
	//if (std::getline(inputFile, firstLine)) {
	string result;
	maxKFile >> result;
	//cout<<"RESULT "<<result<<endl; 
	getline(maxKFile,result);
	getline(maxKFile,result);
	stringstream pro(result);
	int temp;
	for(int i = 1 ; i <= N ; i++){
		 //for(int j = 1 ; j <= K ; j++){
		 	pro >> temp;
		 	if(temp > 0){
		 		output.push_back(i);
		 	}
		 }
	
	outputFile << "#1" <<endl;
	for(int i=0; i<output.size(); i++){
		//for(int j=0; j<output[i].size(); j++){
			outputFile << output[i];
			if(i < output.size()-1)
				outputFile << " ";
		
	}
	outputFile << endl;
	//outputFile<<
	maxKFile.close();	
}
	// outputFile<<"------------------------SIZE 2------------------------------------"<<endl;
	//constraint 5: clique k2 should have exact size k2
	//----//constraint 5.1: Atmost k1 variables out n should be true
	
	return 0;
}
