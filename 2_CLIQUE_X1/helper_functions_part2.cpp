#include "helper_functions.h" 

void Instance::computePairwiseConflictsRel(){
	// Create polygons
	for(int i = 0; i < nbItems; i++){
		PathD polyI; 
		for(int v = 0; v < items[i].nbVertices; v++){
			polyI.push_back(PointD(items[i].coordinates[v][0], items[i].coordinates[v][1]));
		}
		polygons.push_back(polyI);
	}
	
	// Compute relative conflicts
	pairwiseConflictsRel.resize(nbItems);
	for(int i = 0; i < nbItems; i++){
		for(int j = i; j < nbItems; j++){
			for(int l = -items[j].l; l < items[i].l; l++){
				for(int h = -items[j].h; h < items[i].h; h++){
					PathsD clip = {TranslatePath(polygons[j], l, h)};
					PathsD result =  BooleanOp(ClipType::Intersection, FillRule::NonZero, {polygons[i]}, clip, precision);
					if(result.empty() || Area(result) < 1e-8 || (i == j && l == 0 && h == 0)){
						continue;
					}
					pairwiseConflictsRel[i].push_back({j,l,h});
				}
			}
		}
	}
}

void Instance::computePairwiseConflits(){
	nbPairwiseConflicts = 0;
	pairwiseConflictsToId.resize(nbItempos,vector<int>(nbItempos,-1));
	
	// Compute pairwise conflicts 
	for (int l = 0; l < L; l++){
		for (int h = 0; h < H; h++){
			for(int i = 0; i < nbItems; i++){
				if(itemposToId[i][l][h] == -1) continue;
				for(int c = 0; c < pairwiseConflictsRel[i].size();c++){
					if(l + pairwiseConflictsRel[i][c][1] < 0) continue;
					if(l + pairwiseConflictsRel[i][c][1] >= L) continue;
					if(h + pairwiseConflictsRel[i][c][2] < 0) continue;
					if(h + pairwiseConflictsRel[i][c][2] >= H) continue; 
					if(itemposToId[pairwiseConflictsRel[i][c][0]][l + pairwiseConflictsRel[i][c][1]][h + pairwiseConflictsRel[i][c][2]] == -1) continue;
					if(itemposToId[i][l][h] >= itemposToId[pairwiseConflictsRel[i][c][0]][l + pairwiseConflictsRel[i][c][1]][h + pairwiseConflictsRel[i][c][2]]) continue;
					pairwiseConflicts.push_back({itemposToId[i][l][h],itemposToId[pairwiseConflictsRel[i][c][0]][l + pairwiseConflictsRel[i][c][1]][h + pairwiseConflictsRel[i][c][2]]});
					pairwiseConflictsToId[itemposToId[i][l][h]][itemposToId[pairwiseConflictsRel[i][c][0]][l + pairwiseConflictsRel[i][c][1]][h + pairwiseConflictsRel[i][c][2]]] = nbPairwiseConflicts;
					nbPairwiseConflicts++;	
					// cout << "Add " << itemposToId[i][l][h] << "(" << i << "," << l << "," << h << ")" << itemposToId[pairwiseConflictsRel[i][c][0]][l + pairwiseConflictsRel[i][c][1]][h + pairwiseConflictsRel[i][c][2]] << "(" << pairwiseConflictsRel[i][c][0] << "," << l + pairwiseConflictsRel[i][c][1] << "," << h + pairwiseConflictsRel[i][c][2] << ")" << endl;
				}
			}
		}
	}
	conflicts = pairwiseConflicts;
}

void Instance::computeCliqueConflictsECC8(const string& tempPath, const int& nbRuns){
	// Build graph
	string nameFile = tempPath;
	ofstream file(nameFile.c_str());
		
	file << nbItempos << endl;
	for (int p = 0; p < nbItempos; p++){
		int nbConf = 0;
		for (int pp = 0; pp < nbItempos; pp++){
			if (pairwiseConflictsToId[p][pp] != -1 || pairwiseConflictsToId[pp][p] != -1){
				nbConf++;
			}
		}
		file << p << " " << nbConf << endl;
	}
	for (int p = 0; p < nbItempos; p++){
		for (int pp = p+1; pp < nbItempos; pp++){
			if (pairwiseConflictsToId[p][pp] != -1){
				file << p << " " << pp << endl;
			}
		}
	}	
	file.close();
	
	// Run heuristic in java 
	string nameT =  name.substr(0, name.size() - 4);
	cout << "Running Java Edge Clique Cover..." << endl;
    string command1 = "java -jar ./_ECC-master/ECC8.jar -g " + tempPath + " -o " + nameT + " -f nde";
    string command2 = "rm -rf " + nameT;
	
	int bestVal = 999999;
	for(int i = 0; i < nbRuns; i++){	
		// Run the heuristic
		int result = system(command1.c_str());
		if (result == 0) {
			cout << "Java execution completed successfully!" << endl;
		} else {
			cerr << "Error running the Java package." << endl;
		}
		
		// Name of the file containing the number of cliques
		nameFile = nameT+ "/" + nameT + ".nde-rand.EPSc-stats.txt";	

		// Open the file
		ifstream fileIn(nameFile.c_str(), ios::in);
		
		// Read the file
		int nbCliques = 0;
		int time = 0;
		if (fileIn.is_open()) { 
			string key;
			while (fileIn >> key) {
				if (key == "Time:") {
					fileIn >> time; 
					infos.timeCPU[2] += double(time)/1000.0;
				}
				if (key == "Cliques:") {
					fileIn >> nbCliques; 
				}
			}
			fileIn.close();
		} else {
			cout << "Unable to open file " << nameFile << endl; 
		}
		
		// Continue only if the number of cliques is better than the current best
	/*	if (nbCliques == 0 || nbCliques >= bestVal){
			system(command2.c_str());
			continue;
		}*/
		bestVal = nbCliques;
		cout << "A better solution with " << nbCliques << " found !!!" << endl;
		
		// Read the file containing the cliques
		nameFile = nameT+ "/" + nameT + ".nde-rand.EPSc.cover";
	
		// Open the file
		fileIn.clear(); 
		fileIn.open(nameFile.c_str(), ios::in);

		// Read the file
		if (fileIn.is_open()) { 
			conflicts.resize(0);
			string line;
			while (getline(fileIn, line, '\n')) {
				vector<int> clique;
				stringstream ss(line);
				string token;
				while (getline(ss, token, ' ')) {
					clique.push_back(stoi(token));
				}
				conflicts.push_back(clique);
			}
			fileIn.close();
		}
		else {
			cout << "Unable to open file " << nameFile; 
		}
		system(command2.c_str());
	}
	for(int i = 0; i < conflicts.size(); i++){
		cout << i << ":";
		for(int j = 0; j < conflicts[i].size(); j++){
			cout << conflicts[i][j] << " ";
		}
		cout << endl;
	}
}