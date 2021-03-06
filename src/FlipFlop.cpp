#include "HetrickCV.hpp"

struct FlipFlop : Module
{
	enum ParamIds
	{
		NUM_PARAMS
	};
	enum InputIds
	{
        INT_INPUT,
        IND_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
        FFT_OUTPUT,
        FFD_OUTPUT,
        FFTNOT_OUTPUT,
        FFDNOT_OUTPUT,
		NUM_OUTPUTS
    };

    enum LightIds
    {
        FFT_LIGHT,
        FFD_LIGHT,
        FFTNOT_LIGHT,
        FFDNOT_LIGHT,
		TOGGLE_LIGHT,
        DATA_LIGHT,
        NUM_LIGHTS
	};

    dsp::SchmittTrigger clockTrigger;
    int outs[4] = {};
    bool toggle = false;
    bool dataIn = false;

	FlipFlop()
	{
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		onReset();
	}

	void process(const ProcessArgs &args) override;

    void onReset() override
    {
        lights[DATA_LIGHT].value = 0.0f;
        outs[0] = 0;
        outs[1] = lights[DATA_LIGHT].value;
        outs[2] = 10;
        outs[3] = 10;
    }

	// For more advanced Module features, read Rack's engine.hpp header file
	// - dataToJson, dataFromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - reset, randomize: implements special behavior when user clicks these from the context menu
};


void FlipFlop::process(const ProcessArgs &args)
{
    dataIn = (inputs[IND_INPUT].getVoltage(0) >= 1.0f);
    lights[DATA_LIGHT].value = dataIn ? 10 : 0;
    lights[TOGGLE_LIGHT].value = (inputs[INT_INPUT].getVoltage(0) >= 1.0f) ? 10 : 0;

    if (clockTrigger.process(inputs[INT_INPUT].getVoltage()))
    {
        toggle = !toggle;

        outs[0] = toggle ? 10 : 0;
        outs[1] = dataIn ? 10 : 0;
        outs[2] = 10 - outs[0];
        outs[3] = 10 - outs[1];
    }

    outputs[FFT_OUTPUT].setVoltage(outs[0]);
    outputs[FFD_OUTPUT].setVoltage(outs[1]);
    outputs[FFTNOT_OUTPUT].setVoltage(outs[2]);
    outputs[FFDNOT_OUTPUT].setVoltage(outs[3]);

    lights[FFT_LIGHT].value = outs[0];
    lights[FFD_LIGHT].value = outs[1];
    lights[FFTNOT_LIGHT].value = outs[2];
    lights[FFDNOT_LIGHT].value = outs[3];
}

struct FlipFlopWidget : ModuleWidget { FlipFlopWidget(FlipFlop *module); };

FlipFlopWidget::FlipFlopWidget(FlipFlop *module)
{
    setModule(module);
	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		auto *panel = new SvgPanel();
		panel->box.size = box.size;
		panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/FlipFlop.svg")));
		addChild(panel);
	}

	addChild(createWidget<ScrewSilver>(Vec(15, 0)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(createWidget<ScrewSilver>(Vec(15, 365)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 365)));

    //////PARAMS//////

    //////INPUTS//////
    addInput(createInput<PJ301MPort>(Vec(10, 96), module, FlipFlop::INT_INPUT));
    addInput(createInput<PJ301MPort>(Vec(55, 96), module, FlipFlop::IND_INPUT));
    addChild(createLight<SmallLight<RedLight>>(Vec(19, 83), module, FlipFlop::TOGGLE_LIGHT));
    addChild(createLight<SmallLight<RedLight>>(Vec(64, 83), module, FlipFlop::DATA_LIGHT));

    for(int i = 0; i < 4; i++)
    {
        const int yPos = i*45;
        addOutput(createOutput<PJ301MPort>(Vec(33, 150 + yPos), module, FlipFlop::FFT_OUTPUT + i));
        addChild(createLight<SmallLight<RedLight>>(Vec(72, 158 + yPos), module, FlipFlop::FFT_LIGHT + i));
    }
}

Model *modelFlipFlop = createModel<FlipFlop, FlipFlopWidget>("FlipFlop");
