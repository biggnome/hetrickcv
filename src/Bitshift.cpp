#include "HetrickCV.hpp"

struct Bitshift : Module
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

	Bitshift()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
		configParam(Bitshift::AMOUNT_PARAM, -31.0, 31.0, 0.0, "Shift");
		configParam(Bitshift::SCALE_PARAM, -1.0, 1.0, 0.0, "Mod");
		configParam(Bitshift::RANGE_PARAM, 0.0, 1.0, 0.0, "Range", "V", 0.0, 5.0, 5.0);
	}

	void process(const ProcessArgs &args) override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - dataToJson, dataFromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - reset, randomize: implements special behavior when user clicks these from the context menu
};


void Bitshift::process(const ProcessArgs &args)
{
	float bshift = params[AMOUNT_PARAM].getValue() / 31 * 5.0f;
	bool mode5V = (params[RANGE_PARAM].getValue() == 0.0f);

	int channels = inputs[MAIN_INPUT].getChannels();
	for (int c = 0; c < channels; c++) {
		float input = inputs[MAIN_INPUT].getPolyVoltage(c);

	    if(mode5V) input = clamp(input, -5.0f, 5.0f) * 0.2f;
		else input = clamp(input, -10.0f, 10.0f) * 0.1f;

		float shift = bshift + (inputs[AMOUNT_INPUT].getPolyVoltage(c) * params[SCALE_PARAM].getValue());
		shift = clamp(shift, -5.0f, 5.0f) * 0.2f;
		shift *= 31.0f;

		int finalShift = round(shift);
		int intInput = round(input * 2147483647.0f);
		int shiftedInput;

		if(finalShift > 0) shiftedInput = intInput >> finalShift;
		else
		{
			finalShift *= -1;
			shiftedInput = intInput << finalShift;
		}

		float output = shiftedInput/2147483647.0f;
		output = clamp(output, -1.0f, 1.0f);

	    if(mode5V) output *= 5.0f;
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


struct BitshiftWidget : ModuleWidget { BitshiftWidget(Bitshift *module); };

BitshiftWidget::BitshiftWidget(Bitshift *module)
{
	setModule(module);
	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		auto *panel = new SvgPanel();
		panel->box.size = box.size;
		panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Bitshift.svg")));
		addChild(panel);
	}

	addChild(createWidget<ScrewSilver>(Vec(15, 0)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(createWidget<ScrewSilver>(Vec(15, 365)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 365)));

	//////PARAMS//////
	addParam(createParam<Davies1900hBlackSnapKnob>(Vec(27, 62), module, Bitshift::AMOUNT_PARAM));
    addParam(createParam<Trimpot>(Vec(36, 112), module, Bitshift::SCALE_PARAM));
    addParam(createParam<CKSSRot>(Vec(35, 200), module, Bitshift::RANGE_PARAM));

	//////INPUTS//////
    addInput(createInput<PJ301MPort>(Vec(33, 235), module, Bitshift::MAIN_INPUT));
    addInput(createInput<PJ301MPort>(Vec(33, 145), module, Bitshift::AMOUNT_INPUT));

	//////OUTPUTS//////
	addOutput(createOutput<PJ301MPort>(Vec(33, 285), module, Bitshift::MAIN_OUTPUT));
}

Model *modelBitshift = createModel<Bitshift, BitshiftWidget>("Bitshift");
