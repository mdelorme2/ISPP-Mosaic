#include "helper_functions.h" 

void Instance::computeFullMosaic(){
	// Set tileX, tileY, mosL, and mosH
	tileX = 0; tileY = 0; mosL = L; mosH = H;
	
	// Create polygons
	for(int i = 0; i < nbItems; i++){
		PathD polyI; 
		for(int v = 0; v < items[i].nbVertices; v++){
			polyI.push_back(PointD(items[i].coordinates[v][0], items[i].coordinates[v][1]));
		}
		polygons.push_back(polyI);
	}
	
	// Initialize itemposMosToId
	itemposMosToId.resize(nbItems);
	for(int i = 0; i < nbItems; i++){
		itemposMosToId[i].resize(L);
		for(int l = 0; l < L; l++){
			itemposMosToId[i][l].resize(H,-1);
		}
	}
	
	// Initialize mosaic
	PathD tile = MakePathD({0,0, 0,H, L,H, L,0});
	mos = {tile};
	nbItemposMos = 0;
	itemposMosInFrag.push_back({});

	for(int i = 0; i < nbItems; i++){
		cout << "Test " << i << " " << mos.size() << endl;
		for(int l = 0; l < L-items[i].l+1; l++){
			for(int h = 0; h < H-items[i].h+1; h++){
				// cout << "Test " << i << " " << l << " " << h << " " << mos.size() << endl;
				PathsD clip = {TranslatePath(polygons[i], l, h)};
				// First, test if the item intersects the tile
				PathsD result = BooleanOp(ClipType::Intersection, FillRule::NonZero, {tile}, clip, precision);
				if(result.empty() || Area(result) < 1e-8){
					// cout << "The item does not intersect the tile" << endl;
					continue;
				}			
				// Create the itemposMos
				Itempos Ip; 
				Ip.id = nbItemposMos; 
				Ip.i = i; 
				Ip.x = l; 
				Ip.y = h; 
				itemposMos.push_back(Ip);
				nbItemposMos++;	
				itemposMosToId[i][l][h] = Ip.id;							
				// Second, test if the item contains the tile
				result = BooleanOp(ClipType::Difference, FillRule::NonZero, {tile}, clip, precision);
				if(result.empty() || Area(result) < 1e-8){
					for(int m = 0; m < mos.size(); m++){
						itemposMosInFrag[m].push_back(Ip.id);
					}
					// cout << "The item contains the tile" << endl;
					continue;
				}
				// Last case, the items intersects but does not contain the tile
				for(int m = 0; m < mos.size(); m++){
					// First, test if the item intersects the mosaic fragment
					PathsD resultI = BooleanOp(ClipType::Intersection, FillRule::NonZero, {mos[m]}, clip, precision);
					if(resultI.empty() || Area(resultI) < 1e-8){
						// cout << "The item does not intesect with the mosaic fragment" << m << endl;
						continue;
					}	
					// Second, test if the item contains the mosaic fragment
					PathsD resultM = BooleanOp(ClipType::Difference, FillRule::NonZero, {mos[m]}, clip, precision);
					if(resultM.empty() || Area(resultM) < 1e-8){
						itemposMosInFrag[m].push_back(Ip.id);
						// cout << "The item contains the mosaic fragment" << m << endl;
						continue;
					}
					// Last case, the item intersects but does not contain the mosaic fragment
					mos[m] = resultM[0];
					vector<int> tempId = itemposMosInFrag[m]; 
					for(int inter = 1; inter < resultM.size(); inter++){
						mos.push_back(resultM[inter]);
						itemposMosInFrag.push_back(tempId);
					}
					for(int inter = 0; inter < resultI.size(); inter++){
						mos.push_back(resultI[inter]);
						itemposMosInFrag.push_back(tempId);
					}			
				}
				if(mos.size() >= 10000000)
					return;
			}
		}
	}
	itemposMosInAllFrags.resize(nbItemposMos,0);
	isFragDom.resize(mos.size(),false);
	// printMosaic();
}

void Instance::computeMosaic(const int& moL, const int& moH){
	// Set tileX, tileY, mosL, and mosH
	tileX = 0; tileY = 0; mosL = moL; mosH = moH;
	
	// Create polygons	
	int sumA = 0;
	for(int i = 0; i < nbItems; i++){
		tileX = max(tileX,items[i].l);
		tileY = max(tileY,items[i].h);
		PathD polyI; 
		for(int v = 0; v < items[i].nbVertices; v++){
			polyI.push_back(PointD(items[i].coordinates[v][0], items[i].coordinates[v][1]));
		}
		polygons.push_back(polyI);
		sumA += abs(Area(polyI)) * items[i].demand;
	}
	infos.ObjBound = ceil(sumA/H);

	// Initialize itemposMosToId
	itemposMosToId.resize(nbItems);
	for(int i = 0; i < nbItems; i++){
		itemposMosToId[i].resize(tileX + mosL);
		for(int l = 0; l < tileX + mosL; l++){
			itemposMosToId[i][l].resize(tileY + mosH,-1);
		}
	}
	
	// Initialize mosaic
	PathD tile = MakePathD({tileX,tileY, tileX,tileY+mosH, tileX+mosL,tileY+mosH, tileX+mosL,tileY});
	mos = {tile};
	nbItemposMos = 0;
	itemposMosInFrag.push_back({});

	for(int i = 0; i < nbItems; i++){
		for(int l = tileX - items[i].l; l < tileX + mosL; l++){
			for(int h = tileY - items[i].h; h < tileY + mosH; h++){
				// cout << "Test " << i << " " << l << " " << h << " " << mos.size() << endl;
				PathsD clip = {TranslatePath(polygons[i], l, h)};
				// First, test if the item intersects the tile
				PathsD result = BooleanOp(ClipType::Intersection, FillRule::NonZero, {tile}, clip, precision);
				if(result.empty() || Area(result) < 1e-8){
					// cout << "The item does not intersect the tile" << endl;
					continue;
				}			
				// Create the itemposMos
				Itempos Ip; 
				Ip.id = nbItemposMos; 
				Ip.i = i; 
				Ip.x = l; 
				Ip.y = h; 
				itemposMos.push_back(Ip);
				itemposMosInAllFrags.push_back(0);
				nbItemposMos++;	
				itemposMosToId[i][l][h] = Ip.id;							
				// Second, test if the item contains the tile
				result = BooleanOp(ClipType::Difference, FillRule::NonZero, {tile}, clip, precision);
				if(result.empty() || Area(result) < 1e-8){
					for(int m = 0; m < mos.size(); m++){
						itemposMosInFrag[m].push_back(Ip.id);
					}
					itemposMosInAllFrags.back() = 1;
					// cout << "The item contains the tile" << endl;
					continue;
				}
				// Last case, the items intersects but does not contain the tile
				for(int m = 0; m < mos.size(); m++){
					// First, test if the item intersects the mosaic fragment
					PathsD resultI = BooleanOp(ClipType::Intersection, FillRule::NonZero, {mos[m]}, clip, precision);
					if(resultI.empty() || Area(resultI) < 1e-8){
						// cout << "The item does not intesect with the mosaic fragment" << m << endl;
						continue;
					}	
					// Second, test if the item contains the mosaic fragment
					PathsD resultM = BooleanOp(ClipType::Difference, FillRule::NonZero, {mos[m]}, clip, precision);
					if(resultM.empty() || Area(resultM) < 1e-8){
						itemposMosInFrag[m].push_back(Ip.id);
						// cout << "The item contains the mosaic fragment" << m << endl;
						continue;
					}
					// Last case, the item intersects but does not contain the mosaic fragment
					mos[m] = resultM[0];
					vector<int> tempId = itemposMosInFrag[m]; 
					for(int inter = 1; inter < resultM.size(); inter++){
						mos.push_back(resultM[inter]);
						itemposMosInFrag.push_back(tempId);
					}
					for(int inter = 0; inter < resultI.size(); inter++){
						mos.push_back(resultI[inter]);
						itemposMosInFrag.push_back(tempId);
					}			
				}
				if(mos.size() >= 10000000)
					return;
			}
		}
	}
	isFragDom.resize(mos.size(),false);
	// printMosaic();
}

void Instance::printMosaic(){
	for(int m = 0; m < mos.size(); m++){
		if(isFragDom[m] == true) continue;
		cout << "Fragment " << m << ": ";
		for(int v = 0; v < mos[m].size();v++){
			cout << "(" << mos[m][v].x << "," << mos[m][v].y << ")";
		}
		cout << " {";
		for(int v = 0; v < itemposMosInFrag[m].size();v++){
			cout << "[" << itemposMos[itemposMosInFrag[m][v]].i << "(" << itemposMos[itemposMosInFrag[m][v]].x  << "," << itemposMos[itemposMosInFrag[m][v]].y  << ")]"; 
			cout << itemposMosInFrag[m][v] << " ";
		}
		cout << "}" << endl;
	}
}	

void Instance::exportMosaic(const string& tempMos){
	// Export mosaic
	ofstream outFile(tempMos);
	outFile << mosL << " " << mosH << "\n";
    for(int i = 0; i < mos.size(); i++) {
		outFile << i << " ";
		for(int j = 0; j < mos[i].size(); j++) {
            outFile << mos[i][j].x - tileX << " " << mos[i][j].y - tileY << " ";
        }
        outFile << "\n"; 
    }
    outFile.close();
}

void Instance::computePairwiseConflictsMos(const int& type){
	nbPairwiseConflictsMos = 0;
	pairwiseConflictsMosToId.resize(nbItemposMos,vector<int>(nbItemposMos,-1));
	
	// Compute pairwise conflicts between itemposMos
	for(int m = 0; m < mos.size(); m++){
		for(int v = 0; v < itemposMosInFrag[m].size(); v++){
			int itemposMosID1 = itemposMosInFrag[m][v];
			for(int w = v+1; w < itemposMosInFrag[m].size(); w++){
				int itemposMosID2 = itemposMosInFrag[m][w];
				if(type && itemposMos[itemposMosID1].i == itemposMos[itemposMosID2].i && items[itemposMos[itemposMosID1].i].demand == 1){
					pairwiseConflictsMosToId[itemposMosID1][itemposMosID2] = -2;
				}
				if(pairwiseConflictsMosToId[itemposMosID1][itemposMosID2] == -1){
					pairwiseConflictsMosToId[itemposMosID1][itemposMosID2] = nbPairwiseConflictsMos;
					pairwiseConflictsMos.push_back({itemposMosID1,itemposMosID2});
					pairwiseConflictFamMos.push_back(nbPairwiseConflictsMos);
					pairwiseConflictsMosInAllFrags.push_back(itemposMosInAllFrags[itemposMosID1] * itemposMosInAllFrags[itemposMosID2]);
					conflictFamMosInAllFrags.push_back(itemposMosInAllFrags[itemposMosID1] * itemposMosInAllFrags[itemposMosID2]);
					nbPairwiseConflictsMos++;						
				}			
			}
		}
	}	
	
	nbConflictFam = nbPairwiseConflictsMos;
}

void Instance::computePairwiseConflictFamiliesMos(){
	nbConflictFam = 0;
	pairwiseConflictFamMos.resize(0);
	pairwiseConflictFamMos.resize(nbPairwiseConflictsMos, -1);
	conflictFamMosInAllFrags.resize(0);
	for(int i = 0; i < nbPairwiseConflictsMos; i++){
		if(pairwiseConflictFamMos[i] >= 0) continue;
		cout << "PairwiseConflictFamMos " << i << " [" << itemposMos[pairwiseConflictsMos[i][0]].i << "("<< itemposMos[pairwiseConflictsMos[i][0]].x << "," << itemposMos[pairwiseConflictsMos[i][0]].y << ")," << itemposMos[pairwiseConflictsMos[i][1]].i << "("<< itemposMos[pairwiseConflictsMos[i][1]].x << "," << itemposMos[pairwiseConflictsMos[i][1]].y << ")] is of the same family as: ";
		pairwiseConflictFamMos[i] = nbConflictFam;
		conflictFamMosInAllFrags.push_back(pairwiseConflictsMosInAllFrags[i]);
		for(int j = i+1; j < nbPairwiseConflictsMos; j++){
			if(itemposMos[pairwiseConflictsMos[i][0]].i != itemposMos[pairwiseConflictsMos[j][0]].i) continue;
			if(itemposMos[pairwiseConflictsMos[i][1]].i != itemposMos[pairwiseConflictsMos[j][1]].i) continue;
			if(itemposMos[pairwiseConflictsMos[i][1]].x - itemposMos[pairwiseConflictsMos[i][0]].x != itemposMos[pairwiseConflictsMos[j][1]].x - itemposMos[pairwiseConflictsMos[j][0]].x) continue; 
			if(itemposMos[pairwiseConflictsMos[i][1]].y - itemposMos[pairwiseConflictsMos[i][0]].y != itemposMos[pairwiseConflictsMos[j][1]].y - itemposMos[pairwiseConflictsMos[j][0]].y) continue;  	
			if((itemposMos[pairwiseConflictsMos[i][1]].x - itemposMos[pairwiseConflictsMos[j][1]].x) % mosL != 0) continue; 	
			if((itemposMos[pairwiseConflictsMos[i][1]].y - itemposMos[pairwiseConflictsMos[j][1]].y) % mosH != 0) continue; 	
			cout << "pairwiseConflictFamMos " << j << " [" << itemposMos[pairwiseConflictsMos[j][0]].i << "("<< itemposMos[pairwiseConflictsMos[j][0]].x << "," << itemposMos[pairwiseConflictsMos[j][0]].y << ")," << itemposMos[pairwiseConflictsMos[j][1]].i << "("<< itemposMos[pairwiseConflictsMos[j][1]].x << "," << itemposMos[pairwiseConflictsMos[j][1]].y << ")] -- ";
			pairwiseConflictFamMos[j] = nbConflictFam;
			conflictFamMosInAllFrags.back() = max(conflictFamMosInAllFrags.back(),pairwiseConflictsMosInAllFrags[j]);
		}
		cout << endl;
		nbConflictFam++;
	}
}

void Instance::computeConflictsFrag(){
	pairwiseConflictsFragAdj.resize(mos.size());
	pairwiseConflictsFrag.resize(mos.size());
	
	// Compute pairwise conflicts between itemposMos
	for(int m = 0; m < mos.size(); m++){
		pairwiseConflictsFragAdj[m].resize(nbConflictFam,0);
		for(int v = 0; v < itemposMosInFrag[m].size(); v++){
			int itemposMosID1 = itemposMosInFrag[m][v];
			for(int w = v+1; w < itemposMosInFrag[m].size(); w++){
				int itemposMosID2 = itemposMosInFrag[m][w];
				if(pairwiseConflictsMosToId[itemposMosID1][itemposMosID2] >= 0){
					pairwiseConflictsFragAdj[m][pairwiseConflictFamMos[pairwiseConflictsMosToId[itemposMosID1][itemposMosID2]]] += 1;			
				}			
			}
		}
	}	
	for(int m = 0; m < mos.size(); m++){
		for(int f = 0; f < nbConflictFam; f++){
			if(pairwiseConflictsFragAdj[m][f] >= 1){
				pairwiseConflictsFrag[m].push_back(f);
			}
		}
	}	
}

void Instance::computeSetCoveringExact(){
	
	// First, detect fragments included in other fragments
	for(int m = 0; m < mos.size(); m++){
		if(isFragDom[m] == true) continue;
		for(int m2 = m+1; m2 < mos.size(); m2++){
			if(includes(itemposMosInFrag[m].begin(), itemposMosInFrag[m].end(), itemposMosInFrag[m2].begin(), itemposMosInFrag[m2].end())){
				// cout << "Fragment " << m << " dominates fragment " << m2 << endl;
				isFragDom[m2]=true;
			}
		}
	}
	
	// Then launch the ILP
	GRBEnv env = GRBEnv();
	GRBModel model = GRBModel(env);

	vector<GRBVar> isFragUsed(mos.size());	
	vector<GRBLinExpr> isConfCovered (nbConflictFam,0);
	GRBLinExpr objFun = 0;
	
	// Initialize variables
	for(int m = 0; m < mos.size(); m++){
		if(isFragDom[m] == true) continue;
		isFragUsed[m] = model.addVar(0, 1, 0, GRB_BINARY);
	}			
	model.update();
	
	// Perform values
	for(int m = 0; m < mos.size(); m++){
		if(isFragDom[m] == true) continue;
		objFun += isFragUsed[m];
		//cout << "Frag " << m << " covers edge/family ";
		for(int v = 0; v < pairwiseConflictsFrag[m].size(); v++){
			if(conflictFamMosInAllFrags[pairwiseConflictsFrag[m][v]] == 0)
				isConfCovered[pairwiseConflictsFrag[m][v]] += isFragUsed[m];
			//cout << pairwiseConflictsFrag[m][v] << " ";
		}
		//cout << endl;
	}
			
	// Covering
	for(int f = 0; f < nbConflictFam; f++){
		model.addConstr(isConfCovered[f] + conflictFamMosInAllFrags[f] >= 1);
	}
	
	// At least one fragment
	model.addConstr(objFun >= 1);
	
	// Objective function
	model.setObjective(objFun, GRB_MINIMIZE); 
		
	// Setting of Gurobi
	model.getEnv().set(GRB_DoubleParam_TimeLimit,  3600);
	// model.getEnv().set(GRB_IntParam_Method, 2);
	model.getEnv().set(GRB_IntParam_Threads, 1);
	model.getEnv().set(GRB_DoubleParam_MIPGap, 0);
	model.optimize();
		
	// Return solution
	conflictsMos.resize(0);
	for(int m = 0; m < mos.size(); m++){
		if(isFragDom[m] == true) continue;
		if(ceil(isFragUsed[m].get(GRB_DoubleAttr_X) - 0.00001) == 0){
			isFragDom[m] = true;
			continue;
		}
		vector<int> clique;
		for(int ip = 0; ip < itemposMosInFrag[m].size(); ip++){
			clique.push_back(itemposMosInFrag[m][ip]);
		}
		conflictsMos.push_back(clique);
	}
	
	cout << "After preprocessing" << endl;
	printMosaic();
}

void Instance::computeSetCoveringExactVertex(const int& type, const int& onlyRightMost){
	// Handle Itempos families
	vector<int> itemposFamMos(nbItemposMos, -1);
	int nbItemposFam;
	if(type == 2){ // If families are not considered
		for(int i = 0; i < nbItemposMos; i++){
			itemposFamMos[i] = i;
		}
		nbItemposFam = nbItemposMos;
	}
	else{
		for(int i = 0; i < nbItemposMos; i++){
			itemposFamMos[i] = itemposMos[i].i;
		}
		nbItemposFam = nbItems;
	}

	GRBEnv env = GRBEnv();
	GRBModel model = GRBModel(env);

	vector<GRBVar> isFragUsed(mos.size());	
	vector<GRBLinExpr> isItemCovered (nbItemposFam,0);
	GRBLinExpr objFun = 0;
	
	// Initialize variables
	for(int m = 0; m < mos.size(); m++){
		if(isFragDom[m] == true) continue;
		isFragUsed[m] = model.addVar(0, 1, 0, GRB_BINARY);
	}			
	model.update();
	
	// Perform values
	for(int m = 0; m < mos.size(); m++){
		if(isFragDom[m] == true) continue;
		objFun += isFragUsed[m];
		//cout << "Frag " << m << " covers edge/family ";
		for(int v = 0; v < itemposMosInFrag[m].size(); v++){
			if(!onlyRightMost || itemposMos[itemposMosInFrag[m][v]].x <= tileX - items[itemposMos[itemposMosInFrag[m][v]].i].l + 1){
				isItemCovered[itemposFamMos[itemposMosInFrag[m][v]]] += isFragUsed[m];
			}
			//cout << pairwiseConflictsFrag[m][v] << " ";
		}
		//cout << endl;
	}
			
	// Covering
	for(int f = 0; f < nbItemposFam; f++){
		model.addConstr(isItemCovered[f] >= 1);
	}
	
	// Objective function
	model.setObjective(objFun, GRB_MINIMIZE); 
		
	// Setting of Gurobi
	model.getEnv().set(GRB_DoubleParam_TimeLimit,  3600);
	// model.getEnv().set(GRB_IntParam_Method, 2);
	model.getEnv().set(GRB_IntParam_Threads, 1);
	model.getEnv().set(GRB_DoubleParam_MIPGap, 0);
	model.optimize();
		
	// Return solution
	conflictsMos.resize(0);
	vector<int> isItemposFamDone(nbItemposFam,0);
	for(int m = 0; m < mos.size(); m++){
		if(isFragDom[m] == true) continue;
		if(ceil(isFragUsed[m].get(GRB_DoubleAttr_X) - 0.00001) == 0){
			continue;
		}
		cout << "Fragment " << m << ":";
		vector<int> clique;
		for(int ip = 0; ip < itemposMosInFrag[m].size(); ip++){
			if(!type && ((onlyRightMost && itemposMos[itemposMosInFrag[m][ip]].x > tileX - items[itemposMos[itemposMosInFrag[m][ip]].i].l + 1) || isItemposFamDone[itemposFamMos[itemposMosInFrag[m][ip]]] == 1)) continue;
			isItemposFamDone[itemposFamMos[itemposMosInFrag[m][ip]]] = 1;
			clique.push_back(itemposMosInFrag[m][ip]);
			cout << itemposMosInFrag[m][ip] << " ";
		}
		conflictsMosVertex.push_back(clique);
		cout << endl;
	}
}

void Instance::reduceNZs(const int& scoreType, const char& order, const int& nbFragmentKeptFull){
	// First compute the number of times each edge/family is covered
	vector<int> pairwiseConflictsCov(nbConflictFam,0);
	for(int m = 0; m < mos.size(); m++){
		if(isFragDom[m] == true) continue;
		for(int f = 0; f < nbConflictFam; f++){
			pairwiseConflictsCov[f] += pairwiseConflictsFragAdj[m][f];
		}
	}
	
	// Then check whether each packing candidate of each fragment can be removed
	vector<int> pairwiseConflictsIsCov(nbConflictFam,0);
	vector<bool> dealtWith(conflictsMos.size(),false);
	int target = conflictsMos.size(); int done = 0;
	vector<vector<int> > isKept(conflictsMos.size());
	for(int m = 0; m < conflictsMos.size(); m++){
		isKept[m].resize(conflictsMos[m].size(),1);
	}
	
	while(done + nbFragmentKeptFull < target){
		// Compute the score of each fragment
		vector<int> scores;
		for(int m = 0; m < mos.size(); m++){
			if(isFragDom[m] == true) continue;
			int score;
			if(scoreType == 0){ 
				score = itemposMosInFrag[m].size(); // The number of packing candidates involved
			}
			else{
				score = 0; // The number of pairwise conflicts not yet covered
				for(int f = 0; f < nbConflictFam; f++){
					if(pairwiseConflictsIsCov[f] == 0 && pairwiseConflictsFragAdj[m][f] > 0) score++;
				}
			}
			scores.push_back(score);
		}
		// look for the fittest fragment
		int indexMax = -1;
		int valueMax;
		if(order == 'i'){
			valueMax = 99999;
		}
		else{
			valueMax = 0;
		}
		cout << "Scores: ";
		for(int m = 0; m < conflictsMos.size(); m++){
			cout << scores[m] << " ";
			if(!dealtWith[m] && ((order != 'i' && scores[m] > valueMax) || (order == 'i' && scores[m] < valueMax))){
				indexMax = m;
				valueMax = scores[m];
			}
		}
		cout << endl;
		
		// Check if its packing candidates can be removed
		for(int v = 0; v < conflictsMos[indexMax].size(); v++){
			bool canBeRemoved = true;
			int itemposMosID1 = conflictsMos[indexMax][v];
			vector<int> impact(nbConflictFam,0);
			for(int w = 0; w < conflictsMos[indexMax].size(); w++){
				int itemposMosID2 = conflictsMos[indexMax][w];
				if(isKept[indexMax][w] == 0) continue;
				if(pairwiseConflictsMosToId[itemposMosID1][itemposMosID2] >= 0) impact[pairwiseConflictFamMos[pairwiseConflictsMosToId[itemposMosID1][itemposMosID2]]] += 1;
				if(pairwiseConflictsMosToId[itemposMosID2][itemposMosID1] >= 0) impact[pairwiseConflictFamMos[pairwiseConflictsMosToId[itemposMosID2][itemposMosID1]]] += 1;
			}
			for(int f = 0; f < nbConflictFam; f++){
				if(pairwiseConflictsCov[f] - impact[f] <= 0){
					canBeRemoved = false;					
					break;
				}
			}
			if(canBeRemoved){
				isKept[indexMax][v] = 0;
				for(int f = 0; f < nbConflictFam; f++) pairwiseConflictsCov[f] -= impact[f];
			}
		}
		// Compute the pairwiseConflictsIsCov
		for(int v = 0; v < conflictsMos[indexMax].size(); v++){
			int itemposMosID1 = conflictsMos[indexMax][v];
			for(int w = v+1; w < conflictsMos[indexMax].size(); w++){
				int itemposMosID2 = conflictsMos[indexMax][w];
				if(isKept[indexMax][v] == 1 && isKept[indexMax][w] == 1){
					if(pairwiseConflictsMosToId[itemposMosID1][itemposMosID2] >= 0)
						pairwiseConflictsIsCov[pairwiseConflictFamMos[pairwiseConflictsMosToId[itemposMosID1][itemposMosID2]]] = 1;						
				}
			}
		}
		dealtWith[indexMax] = true;
		done++;
	}
	for(int m = 0; m < conflictsMos.size(); m++){
		cout << m << " ";
		vector<int> replacement;
		for(int v = 0; v < conflictsMos[m].size(); v++){
			if(isKept[m][v]){
				
				replacement.push_back(conflictsMos[m][v]);
			}
		}
		cout << replacement.size() << ":";
		for(int v = 0; v < replacement.size(); v++)
			cout << replacement[v] << " ";
		cout << endl;
		// Return solution
		conflictsMos[m] = replacement;
	}
}

void Instance::computeExtendedConflicts(){
	conflicts.resize(0);
	conflictsVertex.resize(0);
	for(int l = 0; l < L; l = l + mosL){
		for(int h = 0; h < H; h = h + mosH){
			for(int c = 0; c < conflictsMos.size(); c++){
				vector<int> clique;
				for(int ip = 0; ip < conflictsMos[c].size(); ip++){
					int itemID = itemposMos[conflictsMos[c][ip]].i;
					int itemX = l + itemposMos[conflictsMos[c][ip]].x - tileX;
					int itemY = h + itemposMos[conflictsMos[c][ip]].y - tileY;
					if(itemX < 0) continue;
					if(itemX >= L) continue;
					if(itemY < 0) continue;
					if(itemY >= H) continue; 
					if(itemposToId[itemID][itemX][itemY] == -1) continue;
					clique.push_back(itemposToId[itemID][itemX][itemY]);
				}
				conflicts.push_back(clique); conflictsInfo.push_back({l,h,c});
			}
			for(int c = 0; c < conflictsMosVertex.size(); c++){
				vector<int> clique;
				for(int ip = 0; ip < conflictsMosVertex[c].size(); ip++){
					int itemID = itemposMos[conflictsMosVertex[c][ip]].i;
					int itemX = l + itemposMos[conflictsMosVertex[c][ip]].x - tileX;
					int itemY = h + itemposMos[conflictsMosVertex[c][ip]].y - tileY;
					if(itemX < 0) continue;
					if(itemX >= L) continue;
					if(itemY < 0) continue;
					if(itemY >= H) continue; 
					if(itemposToId[itemID][itemX][itemY] == -1) continue;
					clique.push_back(itemposToId[itemID][itemX][itemY]);
				}
				conflictsVertex.push_back(clique); conflictsVertexInfo.push_back({l,h,c});
			}
		}
	}
}