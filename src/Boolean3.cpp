#include "HetrickCV.hpp"

struct Boolean3 : Module
{
	enum ParamIds
	{
		NUM_PARAMS
	};
	enum InputIds
	{
        INA_INPUT,
        INB_INPUT,
        INC_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
        OR_OUTPUT,
        AND_OUTPUT,
        XOR_OUTPUT,
        NOR_OUTPUT,
        NAND_OUTPUT,
        XNOR_OUTPUT,
		NUM_OUTPUTS
    };

    enum LightIds
    {
        OR_LIGHT,
        AND_LIGHT,
        XOR_LIGHT,
        NOR_LIGHT,
        NAND_LIGHT,
        XNOR_LIGHT,
		INA_LIGHT,
        INB_LIGHT,
        INC_LIGHT,
        NUM_LIGHTS
	};

    HysteresisGate ins[3];
    bool inA = false;
    bool inB = false;
    bool inC = false;
    int outs[6] = {};

	Boolean3()
	{
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void process(const ProcessArgs &args) override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - dataToJson, dataFromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - reset, randomize: implements special behavior when user clicks these from the context menu
};


void Boolean3::process(const ProcessArgs &args)
{
    int channels = std::max(std::max(inputs[INA_INPUT].getChannels(), inputs[INB_INPUT].getChannels()), inputs[INC_INPUT].getChannels());
    for (int c = 0; c < channels; c++) {
        inA = ins[0].process(inputs[INA_INPUT].getPolyVoltage(c));
        inB = ins[1].process(inputs[INB_INPUT].getPolyVoltage(c));
        inC = ins[2].process(inputs[INC_INPUT].getPolyVoltage(c));


        if(inputs[INC_INPUT].isConnected())
        {
            outs[0] = ((inA || inB) || inC) ? 10 : 0;
            outs[1] = ((inA && inB) && inC) ? 10 : 0;
            outs[2] = (!inA && (inB ^ inC)) || (inA && !(inB || inC)) ? 10 : 0;
            outs[3] = 10 - outs[0];
            outs[4] = 10 - outs[1];
            outs[5] = 10 - outs[2];
        }
        else
        {
            outs[0] = (inA || inB) ? 10 : 0;
            outs[1] = (inA && inB) ? 10 : 0;
            outs[2] = (inA != inB) ? 10 : 0;
            outs[3] = 10 - outs[0];
            outs[4] = 10 - outs[1];
            outs[5] = 10 - outs[2];
        }

        outputs[OR_OUTPUT].setVoltage(outs[0], c);
        outputs[AND_OUTPUT].setVoltage(outs[1], c);
        outputs[XOR_OUTPUT].setVoltage(outs[2], c);
        outputs[NOR_OUTPUT].setVoltage(outs[3], c);
        outputs[NAND_OUTPUT].setVoltage(outs[4], c);
        outputs[XNOR_OUTPUT].setVoltage(outs[5], c);
    }

    outputs[OR_OUTPUT].setChannels(channels);
    outputs[AND_OUTPUT].setChannels(channels);
    outputs[XOR_OUTPUT].setChannels(channels);
    outputs[NOR_OUTPUT].setChannels(channels);
    outputs[NAND_OUTPUT].setChannels(channels);
    outputs[XNOR_OUTPUT].setChannels(channels);

    lights[INA_LIGHT].value = ins[0].process(inputs[INA_INPUT].getVoltage(0)) ? 10 : 0;
    lights[INB_LIGHT].value = ins[1].process(inputs[INA_INPUT].getVoltage(0)) ? 10 : 0;
    lights[INC_LIGHT].value = ins[2].process(inputs[INA_INPUT].getVoltage(0)) ? 10 : 0;

    lights[OR_LIGHT].value = outs[0];
    lights[AND_LIGHT].value = outs[1];
    lights[XOR_LIGHT].value = outs[2];
    lights[NOR_LIGHT].value = outs[3];
    lights[NAND_LIGHT].value = outs[4];
    lights[XNOR_LIGHT].value = outs[5];
}

struct Boolean3Widget : ModuleWidget { Boolean3Widget(Boolean3 *module); };

Boolean3Widget::Boolean3Widget(Boolean3 *module)
{
    setModule(module);
	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		auto *panel = new SvgPanel();
		panel->box.size = box.size;
		panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Boolean3.svg")));
		addChild(panel);
	}

	addChild(createWidget<ScrewSilver>(Vec(15, 0)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(createWidget<ScrewSilver>(Vec(15, 365)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 365)));

    //////INPUTS//////
    addInput(createInput<PJ301MPort>(Vec(10, 105), module, Boolean3::INA_INPUT));
    addInput(createInput<PJ301MPort>(Vec(10, 195), module, Boolean3::INB_INPUT));
    addInput(createInput<PJ301MPort>(Vec(10, 285), module, Boolean3::INC_INPUT));
    addChild(createLight<SmallLight<RedLight>>(Vec(18, 92), module, Boolean3::INA_LIGHT));
    addChild(createLight<SmallLight<RedLight>>(Vec(18, 182), module, Boolean3::INB_LIGHT));
    addChild(createLight<SmallLight<RedLight>>(Vec(18, 272), module, Boolean3::INC_LIGHT));

    //////OUTPUTS//////
    for(int i = 0; i < 6; i++)
    {
        const int yPos = i*45;
        addOutput(createOutput<PJ301MPort>(Vec(45, 60 + yPos), module, Boolean3::OR_OUTPUT + i));
        addChild(createLight<SmallLight<RedLight>>(Vec(74, 68 + yPos), module, Boolean3::OR_LIGHT + i));
    }

}

Model *modelBoolean3 = createModel<Boolean3, Boolean3Widget>("Boolean3");
