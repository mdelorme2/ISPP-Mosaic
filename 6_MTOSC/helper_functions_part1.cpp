#include "helper_functions.h" 

void Instance::load(const string& path, const string& filein){   
	// Get names
	name = filein;
	string nameFile = path + filein;
	
	// Open the file
	ifstream file(nameFile.c_str(), ios::in);

    // Read the file
    if (file.is_open()) { //if the file is open
        string line;
        // first line contains H and L
		getline(file, line, '\t'); H = stoi(line);
		getline(file, line, '\n'); L = stoi(line);
		// second line contains the number of items
		getline(file, line, '\n'); nbItems = stoi(line);
		// then for each item
		for(int i = 0; i < nbItems;i++){
			Item it;
			// first line contains id, nbVertices, and demand
			getline(file, line, '\t'); it.id = stoi(line); 
			getline(file, line, '\t'); it.nbVertices = stoi(line); 
			getline(file, line, '\n'); it.demand = stoi(line); 
			// second line contains abcissas
			vector<int> abs;
			for(int j = 0; j < it.nbVertices - 1;j++){
				getline(file, line, '\t'); abs.push_back(stoi(line));
			}
			getline(file, line, '\n'); abs.push_back(stoi(line)); 
			// third line contains ordinates
			for(int j = 0; j < it.nbVertices - 1;j++){
				getline(file, line, '\t'); it.coordinates.push_back({abs[j],stoi(line)}); 
			}
			getline(file, line, '\n'); it.coordinates.push_back({abs.back(),stoi(line)});  
			items.push_back(it);
		}
        // close the file
        file.close(); 
		// compute the outer box of each item and set (0,0) as the reference point
		for(int i = 0; i < nbItems;i++){
			int minX = L; int maxX = 0; int minY = H; int maxY = 0;
			for(int j = 0; j < items[i].nbVertices;j++){
				minX = min(minX,items[i].coordinates[j][0]);
				maxX = max(maxX,items[i].coordinates[j][0]);
				minY = min(minY,items[i].coordinates[j][1]);
				maxY = max(maxY,items[i].coordinates[j][1]);
			}
			items[i].l = maxX - minX;
			items[i].h = maxY - minY;
			for(int j = 0; j < items[i].nbVertices;j++){
				items[i].coordinates[j][0] -= minX;
				items[i].coordinates[j][1] -= minY;
			}
		}
	}
    else {
        cout << "Unable to open file " << nameFile; 
    }
}

void Instance::computeItempos(){
	// Initialize itemposToId
	itemposToId.resize(0); itemposToId.resize(nbItems);
	for(int i = 0; i < nbItems; i++){
		itemposToId[i].resize(L);
		for(int l = 0; l < L; l++){
			itemposToId[i][l].resize(H,-1);
		}
	}
	
	// Fill itempos and itemposToId
	nbItempos = 0; itempos.resize(0);
	for(int i = 0; i < nbItems; i++){
		for(int l = 0; l < L-items[i].l+1; l++){
			for(int h = 0; h < H-items[i].h+1; h++){
				Itempos Ip; 
				Ip.id = nbItempos; 
				Ip.i = i; 
				Ip.x = l; 
				Ip.y = h; 
				itempos.push_back(Ip);
				itemposToId[i][l][h] = nbItempos;
				nbItempos++;
			}
		}
	}		
}

void Instance::printProb(){
	cout << "Instance " << name << endl;
	cout << "H = " << H << ", L = " << L << ", nbItems = " << nbItems << endl;
	for(int i = 0; i < nbItems; i++){
		cout << "Item " << i << "[" << items[i].l << "x" << items[i].h << "] (x " << items[i].demand << "): ";
		for(int j = 0; j < items[i].nbVertices-1; j++){
			cout << "("<< items[i].coordinates[j][0] << "," << items[i].coordinates[j][1] << "),";
		}
		cout << "("<< items[i].coordinates.back()[0] << "," << items[i].coordinates.back()[1] << ")" << endl;
	}	
	/*for(int p = 0; p < nbItempos; p++){
		cout << "Itempos " << p << " " << itempos[p].i << " " << itempos[p].x << " " << itempos[p].y << endl;
	}*/
}

void Instance::checkSol(){	
	infos.correct = 1;
	vector<int> produced(nbItems,0);
	PathsD paths;
	for(int i = 0; i < solution.size(); i++){
		produced[solution[i][0]] += 1;
		paths.push_back(TranslatePath(polygons[solution[i][0]], solution[i][1], solution[i][2]));
	}	
	PathsD strip = {MakePathD({0,0, 0,H, infos.ObjVal,H, infos.ObjVal,0})};
	
    for (int i = 0; i < paths.size(); i++) {
		// Check if fully contained in the strip
		PathsD result =  BooleanOp(ClipType::Difference, FillRule::NonZero, {paths[i]}, strip, precision);
		if(!result.empty()){
			cout << "Item " << solution[i][0] << " not in the strip!" << endl;
			infos.correct = -1;
		}
		// Check for interesection with other polygons
		for (int j = i+1; j < paths.size(); j++) {
			result = BooleanOp(ClipType::Intersection, FillRule::NonZero, {paths[i]}, {paths[j]}, precision);
			if(!result.empty()){
				cout << "Intersection between items " << solution[i][0] << " and " << solution[j][0] << "!" << endl;
				infos.correct = -2;
			}
		}
    }
	for(int i = 0; i < nbItems; i++){
		if(produced[i] < items[i].demand){
			cout << "Missing item " << i << "!" << endl;
			infos.correct = -3;
		}
	}		
	if(infos.correct == 1){
		cout << "Solution checked!" << endl;
	}
}

void Instance::printSol(const string& pathAndSolout){
	PathsD paths;
	for(int i = 0; i < solution.size(); i++){
		paths.push_back(TranslatePath(polygons[solution[i][0]], solution[i][1], solution[i][2]));
	}
	ofstream outFile(pathAndSolout);
	outFile << infos.ObjVal << " " << H << "\n";
    for (int i = 0; i < paths.size(); i++) {
		outFile << solution[i][0] << " ";
		for (int j = 0; j < paths[i].size(); j++) {
            outFile << paths[i][j].x << " " << paths[i][j].y << " ";
        }
        outFile << "\n"; 
    }
    outFile.close();
}

void Instance::printInfo(const string& pathAndFileout){
	string nameFile = pathAndFileout;
	ofstream file(nameFile.c_str(), ios::out | ios::app);
	file << name << "\t" << infos.opt << "\t";
	for (int i = 0; i < infos.timeCPU.size();i++)
		file << infos.timeCPU[i] << "\t";
	file << infos.ObjBound << "\t" << infos.ObjVal <<  "\t" << infos.nbVar << "\t" << infos.nbCons << "\t" << infos.nbNZ << "\t" << infos.nbVarPP << "\t" << infos.nbConsPP << "\t" << infos.nbNZPP << "\t"  << infos.correct << "\t"  << infos.effect << endl;
	file.close();
}
