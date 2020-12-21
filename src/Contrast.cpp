#include "HetrickCV.hpp"

struct Contrast : Module
{
	enum ParamIds
	{
		AMOUNT_PARAM,
        SCALE_PARAM,
        RANGE_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
        MAIN_INPUT,
        AMOUNT_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
		MAIN_OUTPUT,
		NUM_OUTPUTS
	};

	Contrast()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
		configParam(Contrast::AMOUNT_PARAM, 0, 5.0, 0.0, "Amount");
		configParam(Contrast::SCALE_PARAM, -1.0, 1.0, 1.0, "Mod");
		configParam(Contrast::RANGE_PARAM, 0.0, 1.0, 0.0, "Range", "V", 0.0, 5.0, 5.0);
	}

	void process(const ProcessArgs &args) override;

	const float twoPi = 2 * M_PI;
	const float elevenOverSeven = 11 / 7;

	//fast sine approximation pinched from gist.github.com/geraldyeo/988116
	//1.27323954  = 4/pi
	//0.405284735 =-4/(pi^2)
	float fastSin(float x) {
		float fsin = 0;
		if (x < -3.14159265) x += 6.28318531;
		else if (x >  3.14159265) x -= 6.28318531;

		if (x < 0) {
		    fsin = 1.27323954 * x + 0.405284735 * x * x;
		    if (fsin < 0) fsin = .225 * (fsin *-fsin - fsin) + fsin;
		    else fsin = .225 * (fsin * fsin - fsin) + fsin;
		}
		else {
		    fsin = 1.27323954 * x - 0.405284735 * x * x;
		    if (fsin < 0) fsin = .225 * (fsin *-fsin - fsin) + fsin;
		    else fsin = .225 * (fsin * fsin - fsin) + fsin;
		}
		return fsin;
	}

	// For more advanced Module features, read Rack's engine.hpp header file
	// - dataToJson, dataFromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - reset, randomize: implements special behavior when user clicks these from the context menu
};

void Contrast::process(const ProcessArgs &args)
{
    bool mode5V = (params[RANGE_PARAM].getValue() == 0.0f);

    int channels = inputs[MAIN_INPUT].getChannels();
    for (int c = 0; c < channels; c++) {
    	float input = inputs[MAIN_INPUT].getPolyVoltage(c);

	    if (mode5V) input = clamp(input, -5.0f, 5.0f) * 0.2f;
	    else input = clamp(input, -10.0f, 10.0f) * 0.1f;

	    float contrast = params[AMOUNT_PARAM].getValue() + (inputs[AMOUNT_INPUT].getPolyVoltage(c) * params[SCALE_PARAM].getValue());
	    contrast = clamp(contrast, 0.0f, 5.0f) * 0.2f;

	    float factor1 = input * elevenOverSeven;
	    float factor2 = fastSin(input * twoPi) * contrast;

	    float output = fastSin(factor1 + factor2);

	    if (mode5V) output *= 5.0f;
	    else output *= 10.0f;

	    outputs[MAIN_OUTPUT].setVoltage(output, c);
	}
	outputs[MAIN_OUTPUT].setChannels(channels);
}

struct CKSSRot : SvgSwitch {
	CKSSRot() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CKSS_rot_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CKSS_rot_1.svg")));
	}
};


struct ContrastWidget : ModuleWidget { ContrastWidget(Contrast *module); };

ContrastWidget::ContrastWidget(Contrast *module)
{
	setModule(module);
	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		auto *panel = new SvgPanel();
		panel->box.size = box.size;
		panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Contrast.svg")));
		addChild(panel);
	}

	addChild(createWidget<ScrewSilver>(Vec(15, 0)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(createWidget<ScrewSilver>(Vec(15, 365)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 365)));

	//////PARAMS//////
	addParam(createParam<Davies1900hBlackKnob>(Vec(27, 62), module, Contrast::AMOUNT_PARAM));
    addParam(createParam<Trimpot>(Vec(36, 112), module, Contrast::SCALE_PARAM));
    addParam(createParam<CKSSRot>(Vec(35, 200), module, Contrast::RANGE_PARAM));

	//////INPUTS//////
    addInput(createInput<PJ301MPort>(Vec(33, 235), module, Contrast::MAIN_INPUT));
    addInput(createInput<PJ301MPort>(Vec(33, 145), module, Contrast::AMOUNT_INPUT));

	//////OUTPUTS//////
	addOutput(createOutput<PJ301MPort>(Vec(33, 285), module, Contrast::MAIN_OUTPUT));
}

Model *modelContrast = createModel<Contrast, ContrastWidget>("Contrast");
