#pragma once

#include "HetrickUtilities.hpp"

using namespace rack;


extern Plugin *pluginInstance;

extern Model *modelTwoToFour;
extern Model *modelAnalogToDigital;
extern Model *modelASR;
extern Model *modelBitshift;
extern Model *modelBlankPanel;
extern Model *modelBoolean3;
extern Model *modelComparator;
extern Model *modelContrast;
extern Model *modelCrackle;
extern Model *modelDelta;
extern Model *modelDigitalToAnalog;
extern Model *modelDust;
extern Model *modelExponent;
extern Model *modelFlipFlop;
extern Model *modelFlipPan;
extern Model *modelGateJunction;
extern Model *modelLogicCombine;
extern Model *modelRandomGates;
extern Model *modelRingMod;
extern Model *modelRotator;
extern Model *modelScanner;
extern Model *modelWaveshape;

struct Davies1900hBlackSnapKnob : Davies1900hBlackKnob {
	Davies1900hBlackSnapKnob() {
		snap = true;
	}
};
