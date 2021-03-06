#include "HetrickCV.hpp"

struct Rotator : Module
{
	enum ParamIds
	{
        ROTATE_PARAM,
        STAGES_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
        IN1_INPUT,
        IN2_INPUT,
        IN3_INPUT,
        IN4_INPUT,
        IN5_INPUT,
        IN6_INPUT,
        IN7_INPUT,
        IN8_INPUT,

        ROTATE_INPUT,
        STAGES_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
        OUT1_OUTPUT,
        OUT2_OUTPUT,
        OUT3_OUTPUT,
        OUT4_OUTPUT,
        OUT5_OUTPUT,
        OUT6_OUTPUT,
        OUT7_OUTPUT,
        OUT8_OUTPUT,
		NUM_OUTPUTS
    };
    enum LightIds
	{
        IN1_POS_LIGHT, IN1_NEG_LIGHT,
        IN2_POS_LIGHT, IN2_NEG_LIGHT,
        IN3_POS_LIGHT, IN3_NEG_LIGHT,
        IN4_POS_LIGHT, IN4_NEG_LIGHT,
        IN5_POS_LIGHT, IN5_NEG_LIGHT,
        IN6_POS_LIGHT, IN6_NEG_LIGHT,
        IN7_POS_LIGHT, IN7_NEG_LIGHT,
        IN8_POS_LIGHT, IN8_NEG_LIGHT,

        OUT1_POS_LIGHT, OUT1_NEG_LIGHT,
        OUT2_POS_LIGHT, OUT2_NEG_LIGHT,
        OUT3_POS_LIGHT, OUT3_NEG_LIGHT,
        OUT4_POS_LIGHT, OUT4_NEG_LIGHT,
        OUT5_POS_LIGHT, OUT5_NEG_LIGHT,
        OUT6_POS_LIGHT, OUT6_NEG_LIGHT,
        OUT7_POS_LIGHT, OUT7_NEG_LIGHT,
        OUT8_POS_LIGHT, OUT8_NEG_LIGHT,

		NUM_LIGHTS
	};

	Rotator()
	{
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(Rotator::ROTATE_PARAM, 0, 7, 0, "Rotate", "", 0);
        configParam(Rotator::STAGES_PARAM, 0, 6, 6, "Stages", "", 0, 1, 2);
	}

    void process(const ProcessArgs &args) override;

    int clampInt(const int _in, const int min = 0, const int max = 7)
    {
        if (_in > max) return max;
        if (_in < min) return min;
        return _in;
    }

	// For more advanced Module features, read Rack's engine.hpp header file
	// - dataToJson, dataFromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - reset, randomize: implements special behavior when user clicks these from the context menu
};


void Rotator::process(const ProcessArgs &args)
{
    int rotation = round(params[ROTATE_PARAM].getValue() + inputs[ROTATE_INPUT].getVoltage());
    int stages = round(params[STAGES_PARAM].getValue() + inputs[STAGES_INPUT].getVoltage());

    stages = clampInt(stages,0,6) + 2;
    rotation = clampInt(rotation);

    for(int i = 0; i < 8; i++)
    {
        int input = (stages - rotation + i) % stages;
        outputs[i].setVoltage(inputs[input].getVoltage());

        lights[IN1_POS_LIGHT + 2*i].setSmoothBrightness(fmaxf(0.0, inputs[i].getVoltage() / 5.0), 10);
		lights[IN1_NEG_LIGHT + 2*i].setSmoothBrightness(fmaxf(0.0, inputs[i].getVoltage() / -5.0), 10);

        lights[OUT1_POS_LIGHT + 2*i].setSmoothBrightness(fmaxf(0.0, outputs[i].value / 5.0), 10);
		lights[OUT1_NEG_LIGHT + 2*i].setSmoothBrightness(fmaxf(0.0, outputs[i].value / -5.0), 10);
    }
}


struct RotatorWidget : ModuleWidget { RotatorWidget(Rotator *module); };

RotatorWidget::RotatorWidget(Rotator *module)
{
    setModule(module);
	box.size = Vec(12 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		auto *panel = new SvgPanel();
		panel->box.size = box.size;
		panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Rotator.svg")));
		addChild(panel);
	}

	addChild(createWidget<ScrewSilver>(Vec(15, 0)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(createWidget<ScrewSilver>(Vec(15, 365)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 365)));

    //////PARAMS//////
    addParam(createParam<Davies1900hBlackSnapKnob>(Vec(70, 85), module, Rotator::ROTATE_PARAM));
    addParam(createParam<Davies1900hBlackSnapKnob>(Vec(70, 245), module, Rotator::STAGES_PARAM));

    addInput(createInput<PJ301MPort>(Vec(75, 150), module, Rotator::ROTATE_INPUT));
    addInput(createInput<PJ301MPort>(Vec(75, 310), module, Rotator::STAGES_INPUT));

    const int inXPos = 10;
    const int outXPos = 145;
    const int inLightX = 50;
    const int outLightX = 120;
    for(int i = 0; i < 8; i++)
    {
        const int yPos = 50 + (40 * i);
        const int lightY = 59 + (40 * i);

        //////INPUTS//////
        addInput(createInput<PJ301MPort>(Vec(inXPos, yPos), module, i));

        //////OUTPUTS//////
        addOutput(createOutput<PJ301MPort>(Vec(outXPos, yPos), module, i));

        //////BLINKENLIGHTS//////
        addChild(createLight<SmallLight<GreenRedLight>>(Vec(inLightX, lightY), module, Rotator::IN1_POS_LIGHT + 2*i));
        addChild(createLight<SmallLight<GreenRedLight>>(Vec(outLightX, lightY), module, Rotator::OUT1_POS_LIGHT + 2*i));
    }
}

Model *modelRotator = createModel<Rotator, RotatorWidget>("Rotator");
