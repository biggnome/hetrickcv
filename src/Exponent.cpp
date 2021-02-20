#include "HetrickCV.hpp"

struct Exponent : Module
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

	Exponent()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
		configParam(Exponent::AMOUNT_PARAM, -1.0, 1.0, 0.0, "Amount", "", 2);
		configParam(Exponent::SCALE_PARAM, -1.0, 1.0, 0.0, "Mod");
		configParam(Exponent::RANGE_PARAM, 0.0, 1.0, 0.0, "Range", "V", 0.0, 5.0, 5.0);
	}

	void process(const ProcessArgs &args) override;

	// faster powf() approximation cribbed from martin.ankerl.com/2007/10/04/optimized-pow-approximation-for-java-and-c-c/
	double fastPow(double a, double b) {
	    if (a == 0) {
	    	return 0;
	    }
	    else {
	    	union {
		        double d;
		        int x[2];
		    } u = { a };
		    u.x[1] = (int)(b * (u.x[1] - 1072632447) + 1072632447);
		    u.x[0] = 0;
		    return u.d;
		}
	}

	// For more advanced Module features, read Rack's engine.hpp header file
	// - dataToJson, dataFromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - reset, randomize: implements special behavior when user clicks these from the context menu
};


void Exponent::process(const ProcessArgs &args)
{
    bool mode5V = (params[RANGE_PARAM].getValue() == 0.0f);

	int channels = inputs[MAIN_INPUT].getChannels();
	for (int c = 0; c < channels; c++) {
		float input = inputs[MAIN_INPUT].getPolyVoltage(c);
		const bool negativeInput = input < 0.0f;

	    if(mode5V) input = clamp(input, -5.0f, 5.0f) * 0.2f;
		else input = clamp(input, -10.0f, 10.0f) * 0.1f;
		input = std::abs(input);

	    float exponent = params[AMOUNT_PARAM].getValue() + (inputs[AMOUNT_INPUT].getPolyVoltage(c) * (params[SCALE_PARAM].getValue() * 0.2));

		if(exponent < 0) exponent = 1.0f - (exponent * -0.5f);
		else exponent += 1.0f;

	    float output = fastPow(input, exponent);

		if (negativeInput) output *= -1.0f;
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


struct ExponentWidget : ModuleWidget { ExponentWidget(Exponent *module); };

ExponentWidget::ExponentWidget(Exponent *module)
{
	setModule(module);
	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		auto *panel = new SvgPanel();
		panel->box.size = box.size;
		panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Exponent.svg")));
		addChild(panel);
	}

	addChild(createWidget<ScrewSilver>(Vec(15, 0)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(createWidget<ScrewSilver>(Vec(15, 365)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 365)));

	//////PARAMS//////
	addParam(createParam<Davies1900hBlackKnob>(Vec(27, 62), module, Exponent::AMOUNT_PARAM));
    addParam(createParam<Trimpot>(Vec(36, 112), module, Exponent::SCALE_PARAM));
    addParam(createParam<CKSSRot>(Vec(35, 200), module, Exponent::RANGE_PARAM));

	//////INPUTS//////
    addInput(createInput<PJ301MPort>(Vec(33, 235), module, Exponent::MAIN_INPUT));
    addInput(createInput<PJ301MPort>(Vec(33, 145), module, Exponent::AMOUNT_INPUT));

	//////OUTPUTS//////
	addOutput(createOutput<PJ301MPort>(Vec(33, 285), module, Exponent::MAIN_OUTPUT));
}

Model *modelExponent = createModel<Exponent, ExponentWidget>("Exponent");
