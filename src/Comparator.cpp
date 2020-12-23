#include "HetrickCV.hpp"

struct Comparator : Module
{
	enum ParamIds
	{
		AMOUNT_PARAM,
        SCALE_PARAM,
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
		GT_GATE_OUTPUT,
		GT_TRIG_OUTPUT,
		LT_GATE_OUTPUT,
		LT_TRIG_OUTPUT,
		ZEROX_OUTPUT,
		NUM_OUTPUTS
	};

	 enum LightIds
    {
        GT_LIGHT,
        LT_LIGHT,
		ZEROX_LIGHT,
        NUM_LIGHTS
	};

	Comparator()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(Comparator::AMOUNT_PARAM, -5.0, 5.0, 0.0, "Amount", "V");
		configParam(Comparator::SCALE_PARAM, -1.0, 1.0, 1.0, "Scale");
	}

	TriggerGenWithSchmitt ltTrig, gtTrig;

	void process(const ProcessArgs &args) override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - dataToJson, dataFromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - reset, randomize: implements special behavior when user clicks these from the context menu
};


void Comparator::process(const ProcessArgs &args)
{
	float input = inputs[MAIN_INPUT].getVoltage();

	float compare = params[AMOUNT_PARAM].getValue() + (inputs[AMOUNT_INPUT].getVoltage() * params[SCALE_PARAM].getValue());
	compare = clamp(compare, -5.0f, 5.0f);

	const bool greaterThan = (input > compare);
	const bool lessThan = (input < compare);

	outputs[GT_TRIG_OUTPUT].setVoltage(gtTrig.process(greaterThan) ? 10 : 0);
	outputs[LT_TRIG_OUTPUT].setVoltage(ltTrig.process(lessThan) ? 10 : 0);
	outputs[GT_GATE_OUTPUT].setVoltage(greaterThan ? 10 : 0);
	outputs[LT_GATE_OUTPUT].setVoltage(lessThan ? 10 : 0);

	int allTrigs = outputs[GT_TRIG_OUTPUT].value + outputs[LT_TRIG_OUTPUT].value;
	allTrigs = clamp(allTrigs, 0, 10);

	outputs[ZEROX_OUTPUT].setVoltage(allTrigs);

	lights[GT_LIGHT].setSmoothBrightness(outputs[GT_GATE_OUTPUT].value, 10);
	lights[LT_LIGHT].setSmoothBrightness(outputs[LT_GATE_OUTPUT].value, 10);
	lights[ZEROX_LIGHT].setSmoothBrightness(allTrigs, 10);
}


struct ComparatorWidget : ModuleWidget { ComparatorWidget(Comparator *module); };

ComparatorWidget::ComparatorWidget(Comparator* module)
{
	setModule(module);
	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		auto *panel = new SvgPanel();
		panel->box.size = box.size;
		panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Comparator.svg")));
		addChild(panel);
	}

	addChild(createWidget<ScrewSilver>(Vec(15, 0)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(createWidget<ScrewSilver>(Vec(15, 365)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 365)));

	//////PARAMS//////
	addParam(createParam<Davies1900hBlackKnob>(Vec(27, 62), module, Comparator::AMOUNT_PARAM));
    addParam(createParam<Trimpot>(Vec(36, 112), module, Comparator::SCALE_PARAM));

	//////INPUTS//////
    addInput(createInput<PJ301MPort>(Vec(33, 195), module, Comparator::MAIN_INPUT));
    addInput(createInput<PJ301MPort>(Vec(33, 145), module, Comparator::AMOUNT_INPUT));

	//////OUTPUTS//////
	addOutput(createOutput<PJ301MPort>(Vec(12, 285), module, Comparator::LT_GATE_OUTPUT));
    addOutput(createOutput<PJ301MPort>(Vec(53, 285), module, Comparator::GT_GATE_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(12, 315), module, Comparator::LT_TRIG_OUTPUT));
    addOutput(createOutput<PJ301MPort>(Vec(53, 315), module, Comparator::GT_TRIG_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(32.5, 245), module, Comparator::ZEROX_OUTPUT));

	//////BLINKENLIGHTS//////
	addChild(createLight<SmallLight<RedLight>>(Vec(22, 275), module, Comparator::LT_LIGHT));
    addChild(createLight<SmallLight<GreenLight>>(Vec(62, 275), module, Comparator::GT_LIGHT));
    addChild(createLight<SmallLight<RedLight>>(Vec(42, 275), module, Comparator::ZEROX_LIGHT));
}

Model *modelComparator = createModel<Comparator, ComparatorWidget>("Comparator");
