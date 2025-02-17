#pragma once

struct CellParams {
	int birth;
	int survivalMin;
	int survivalMax;
};

struct GridParams {
	int isToroidal;
	int GW;
	int GH;
};

extern "C" void asmUpdateGrid(int* currentState,
	int* nextState,
	struct CellParams* cellParams,
	struct GridParams* gridParams);

