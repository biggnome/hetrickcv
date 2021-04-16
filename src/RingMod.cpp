#include "HetrickCV.hpp"
#include "dsp/digital.hpp"

struct RingMod : Module
{
	enum ParamIds
	{
		NUM_PARAMS
	};
	enum InputIds
	{
        INC_INPUT,
        INM_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
        OUT1_OUTPUT,
		NUM_OUTPUTS
	};

	RingMod()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
	}

	void process(const ProcessArgs &args) override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - dataToJson, dataFromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - reset, randomize: implements special behavior when user clicks these from the context menu
};

double diode(double x) {
	x = x * 0.2;
	int sign = (x < 0) ? -1 : ((x > 0) ? 1 : 0);
	x = abs(x) - 0.667;
	x = x + abs(x);
	return (0.864954 * sign * x * x);
}

float parasat(float x) {
	x = clamp(x, -10.0f, 10.0f);
	return (x * (1 - abs(x) * 0.25));
}

void RingMod::process(const ProcessArgs &args)
{
    const float inC = inputs[INC_INPUT].getVoltage();
    const float inM = inputs[INM_INPUT].getVoltage();

    float ring = parasat(diode(inC + inM) + diode(inC - inM));

    outputs[OUT1_OUTPUT].setVoltage(clamp(ring, -10.0f, 10.0f));
}

struct RingModWidget : ModuleWidget { RingModWidget(RingMod *module); };

RingModWidget::RingModWidget(RingMod *module)
{
	setModule(module);
	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		auto *panel = new SvgPanel();
		panel->box.size = box.size;
		panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/RingMod.svg")));
		addChild(panel);
	}

	addChild(createWidget<ScrewSilver>(Vec(15, 0)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(createWidget<ScrewSilver>(Vec(15, 365)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 365)));

    //////INPUTS//////
    addInput(createInput<PJ301MPort>(Vec(33, 150), module, RingMod::INC_INPUT));
    addInput(createInput<PJ301MPort>(Vec(33, 195), module, RingMod::INM_INPUT));

    addOutput(createOutput<PJ301MPort>(Vec(33, 285), module, RingMod::OUT1_OUTPUT));
}

Model *modelRingMod = createModel<RingMod, RingModWidget>("RingMod");

