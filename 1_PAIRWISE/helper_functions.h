#ifndef ALLOCATION_H
	#define ALLOCATION_H
	
	#include <iostream> 
	#include <iomanip> 
	#include <fstream>
	#include <sstream>
	#include <vector>
	#include <string>
	#include <set>
	#include <iostream> 
	#include <math.h> 
	#include <cstdlib>
	#include <algorithm>
	#include <bits/stdc++.h>
	#include <clipper2/clipper.h>
	#include "gurobi_c++.h"
	#include <map>
	#include <tuple>
	
	using namespace std;
	using namespace Clipper2Lib;
	
	class Info;
	class Itempos;
	class Item;
	class Instance;

/*	*************************************************************************************
	************************************* INFO ******************************************
	************************************************************************************* */

	class Info{
	public:
		bool opt;
		vector<double> timeCPU;
		int ObjVal;
		int ObjBound;
		int nbCons;
		int nbVar;
		int nbNZ;
		int nbConsPP;
		int nbVarPP;
		int nbNZPP;
		int correct;
	};

/*	*************************************************************************************
	********************************** ITEMPOS ******************************************
	************************************************************************************* */

	class Itempos{
	public:
		int id; 
		int i;	// item
		int x; 	// abcissa
		int y;	// ordinate
	};

/*	*************************************************************************************
	*********************************** ITEM ********************************************
	************************************************************************************* */

	class Item{
	public:
		int id; 
		int nbVertices;
		int demand;
		int l, h;
		vector<vector<int> > coordinates;
	};
	
/*	*************************************************************************************
	*********************************** INSTANCE ****************************************
	************************************************************************************* */

	class Instance{
	public:	
		// Data read from the file
		string name;
		int L, H, mosL, mosH;
		int nbItems;
		vector<Item> items;
		
		// For the geometry
		vector<PathD> polygons;
		
		// Set precision for Clipper2Lib
		int precision = 8;

		// For the the pairwise and clique constraints
		int nbPairwiseConflicts;
		vector<vector<vector<int> > > pairwiseConflictsRel;		
		vector<vector<int> > pairwiseConflicts;
		vector<vector<int> > pairwiseConflictsToId;	
		
		int nbItempos;
		vector<Itempos> itempos;
		vector<vector<vector<int> > > itemposToId;	
		
		// For the mosaic
		int tileX, tileY;
		PathsD mos;

		int nbItemposMos;
		vector<Itempos> itemposMos;
		vector<vector<vector<int> > > itemposMosToId;
		vector<vector<int> > itemposMosInFrag;
		vector<int> itemposMosInAllFrags;
		
		int nbPairwiseConflictsMos;
		vector<vector<int> > pairwiseConflictsMos;
		vector<int> pairwiseConflictsMosInAllFrags;
		vector<vector<int> > pairwiseConflictsMosToId;	
		vector<vector<int> > conflictsMos;
		vector<vector<int> > conflictsMosVertex;
		
		vector<vector<int> > pairwiseConflictsFragAdj;
		vector<vector<int> > pairwiseConflictsFrag;
		
		int nbConflictFam;
		vector<int> pairwiseConflictFamMos;
		vector<int> conflictFamMosInAllFrags;
	
		// For the preprocessing
		vector<bool> isFragDom;
		
		// For the model 
		Info infos;
		vector<vector<int> > conflicts, conflictsVertex, conflictsInfo, conflictsVertexInfo;	
		vector<vector<int> > solution;
		
		// Functions (general) -- in helper_functions_part1.cpp
		void load(const string& path, const string& filein);
		void computeItempos();	
		void printProb();
		void checkSol();
		void printSol(const string& pathAndSolout);
		void printInfo(const string& pathAndFileout);

		// Functions (for the pairwise and clique constraints) -- in helper_functions_part2.cpp
		void computePairwiseConflictsRel();
		void computePairwiseConflits();	
		void computeCliqueConflictsECC8(const string& pathAndFileout, const int& nbRuns);		

		// Functions (for the mosaic) -- in helper_functions_part3.cpp
		void computeFullMosaic();
		void computeMosaic(const int& moL, const int& moH);
		void computePairwiseConflictsMos(const int& type);
		void computeExtendedConflicts();
		void printMosaic();	
		void exportMosaic(const string& pathAndMosout);
		void computePairwiseConflictFamiliesMos();
		void computeConflictsFrag();
		void computeSetCoveringExact();
		void computeSetCoveringExactVertex(const int& type, const int& onlyRightMost);
		void reduceNZs(const int& scoreType, const char& order, const int& nbFragmentKeptFull);
	};
	
	
#endif 