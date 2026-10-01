#include "main.h"

/*	*************************************************************************************
	*************************************  MAIN *****************************************
	************************************************************************************* */

double initTimeModelCPU;

int main(int argc, char **argv){
	
	initTimeModelCPU = getCPUTime();
	
	// local variables
	Instance inst;
	string filein = argv[2];
	string path = argv[1];
	string pathAndFileout = argv[3];
	string pathAndSolout = argv[4];
	
	string tempPath = pathAndSolout.substr(0, pathAndSolout.size() - 3) +  "nde";
	string tempMos = pathAndSolout.substr(0, pathAndSolout.size() - 4) +  "_moz.fig";
	
	// functions
	inst.infos.timeCPU.push_back(0); // Total time
	inst.infos.timeCPU.push_back(0); // Preprocessing time	
	inst.infos.timeCPU.push_back(0); // External time 
	inst.load(path,filein);		
	inst.computeItempos();		
	inst.printProb();	

//	inst.computePairwiseConflictsRel();
//	inst.computePairwiseConflits();
//	inst.computeCliqueConflictsECC8(tempPath, 1);
	
	inst.computeMosaic(1,1);		
/*	inst.computeFullMosaic(); 
	if(inst.mos.size() >= 1000000){
		inst.infos.opt = 0; 
		inst.infos.timeCPU[0] = (getCPUTime() - initTimeModelCPU);
		inst.infos.ObjVal = -1;
		inst.infos.ObjBound = -1;
		inst.infos.nbCons = -1;
		inst.infos.nbVar = -1;
		inst.infos.nbNZ = -1;
		inst.infos.nbConsPP = -1;
		inst.infos.nbVarPP = -1;
		inst.infos.nbNZPP = -1;
		inst.infos.correct = 1;
		inst.printInfo(pathAndFileout);
		return -1;
	}*/
	
	inst.exportMosaic(tempMos);	
	inst.computePairwiseConflictsMos(1);	// 0 for no optional edges, 1 for optional edges	
	inst.computePairwiseConflictFamiliesMos();		
	inst.computeConflictsFrag();				
	inst.computeSetCoveringExactVertex(0,1);// First parameter is 0 for NZ reduction and families, 1 for NO NZ reduction and families, 2 for NO NZ reduction and NO families; Second parameter should be 0 for the baseline objective function and 1 for the objective function with fake items; Combination (2,1) does not make sense
	inst.computeSetCoveringExact(); 		
	inst.reduceNZs(0,'i',1); 				// First parameter is 0 for score based on number of packing positions, 1 for number of remaining conflicts; Second parameter is 'd' for decreasing, 'i' for increasing, and third parameter is the number of fragments left untouched (from the end)
	inst.computeExtendedConflicts();		

	inst.infos.timeCPU[1] = (getCPUTime() - initTimeModelCPU);
	int ret = ILP(inst);
	if(ret != -1){
		inst.checkSol();
		inst.printSol(pathAndSolout);
	}
	inst.printInfo(pathAndFileout);
	return 0;
}

int ILP(Instance& inst){
		
	GRBEnv env = GRBEnv();
	// Model
	try{
		GRBModel model = GRBModel(env);
		
		// Declaration of the variables for the model
		vector<GRBVar> isItemposUsed(inst.nbItempos);
		vector<GRBLinExpr> nbItemPacked(inst.nbItems);
		vector<GRBVar> isFakeItemUsed(inst.L);
		GRBLinExpr objFun = inst.L;		

		for(int l = 0; l < inst.L;l++){
			isFakeItemUsed[l] = model.addVar(0, 1, 0, GRB_BINARY);
		}
				
		// Initialization
		for (int p = 0; p < inst.nbItempos; p++){
			isItemposUsed[p] = model.addVar(0, 1, 0, GRB_BINARY);
		}
		
		for(int i = 0; i < inst.nbItems;i++){
			nbItemPacked[i] = 0;
		}
						
		model.update();
		
		// Perform values
		for (int p = 0; p < inst.nbItempos; p++){
			nbItemPacked[inst.itempos[p].i] += isItemposUsed[p];			 
		}
				
		// Incompatibility 
		for (int c = 0; c < inst.conflicts.size(); c++){
			GRBLinExpr tempExpr = 0;
			for (int c2 = 0; c2 < inst.conflicts[c].size(); c2++){
				tempExpr += isItemposUsed[inst.conflicts[c][c2]];
			}
			model.addConstr(tempExpr <= 1);
		}

		// Link with objective function 
		for (int c = 0; c < inst.conflictsVertex.size(); c++){
			GRBLinExpr LHS = 0;
			for (int c2 = 0; c2 < inst.conflictsVertex[c].size(); c2++){
				LHS += isItemposUsed[inst.conflictsVertex[c][c2]];
			}
			model.addConstr(LHS <= 1 - isFakeItemUsed[inst.conflictsVertexInfo[c][0]]);
		}

		// Strip length 		
		for (int l = 0; l < inst.L ; l++){
			objFun -= isFakeItemUsed[l];
			if(l > 0) model.addConstr(isFakeItemUsed[l] >= isFakeItemUsed[l-1]); 
		}

		// Demand
		for(int i = 0; i < inst.nbItems;i++){
			model.addConstr(nbItemPacked[i] == inst.items[i].demand); 
		}
		
		// Objective function
		model.setObjective(objFun, GRB_MINIMIZE); 
			
		// Setting of Gurobi
		model.getEnv().set(GRB_DoubleParam_TimeLimit,  3600);
		// model.getEnv().set(GRB_IntParam_Method, 2);
		model.getEnv().set(GRB_IntParam_Threads, 1);
		model.getEnv().set(GRB_DoubleParam_MIPGap, 0);;
		model.optimize();
			
		// Filling Info
		inst.infos.timeCPU[0] = getCPUTime() - initTimeModelCPU;
		inst.infos.ObjBound = ceil(model.get(GRB_DoubleAttr_ObjBound) - EPSILON);
		inst.infos.opt = false;

		// Get Info 
		inst.infos.nbVar =  model.get(GRB_IntAttr_NumVars);
		inst.infos.nbCons = model.get(GRB_IntAttr_NumConstrs);
		inst.infos.nbNZ = model.get(GRB_IntAttr_NumNZs);
		
		// If no solution found
		if (model.get(GRB_IntAttr_SolCount) < 1){
			cout << "Failed to optimize ILP. " << endl;
			inst.infos.ObjVal  = inst.L;
			inst.infos.correct  = 1001;
			// Get Info post-preprocessing 
			GRBModel presolvedModel = model.presolve();
			inst.infos.nbVarPP  = presolvedModel.get(GRB_IntAttr_NumVars);
			inst.infos.nbConsPP = presolvedModel.get(GRB_IntAttr_NumConstrs);
			inst.infos.nbNZPP   = presolvedModel.get(GRB_IntAttr_NumNZs);
			return -1;
		}

		// If solution found
		inst.infos.ObjVal = ceil(model.get(GRB_DoubleAttr_ObjVal) - EPSILON);	
		if(inst.infos.ObjVal == inst.infos.ObjBound) inst.infos.opt = true;

		// Print and save solution
		inst.solution.resize(0);
		for (int p = 0; p < inst.nbItempos; p++){
			if(ceil(isItemposUsed[p].get(GRB_DoubleAttr_X) - EPSILON) == 1){
				cout << "Item " << inst.itempos[p].i << " position " << inst.itempos[p].x << " " << inst.itempos[p].y << endl;
				inst.solution.push_back({inst.itempos[p].i,inst.itempos[p].x,inst.itempos[p].y});
			}
		}

		// Get Info post-preprocessing 
		GRBModel presolvedModel = model.presolve();
		inst.infos.nbVarPP  = presolvedModel.get(GRB_IntAttr_NumVars);
		inst.infos.nbConsPP = presolvedModel.get(GRB_IntAttr_NumConstrs);
		inst.infos.nbNZPP   = presolvedModel.get(GRB_IntAttr_NumNZs);
	}	
	// Exceptions
	catch (GRBException e) {
		cout << "Error code = " << e.getErrorCode() << endl;
		cout << e.getMessage() << endl;
	}
	catch (...) {
		cout << "Exception during optimization" << endl;
	}

	// End
	return 0;
}
