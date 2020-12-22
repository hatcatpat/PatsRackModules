#include "plugin.hpp"

struct Chance : Module
{
	enum ParamIds
	{
		INPUT1_PARAM,
		INPUT2_PARAM,
		INPUT3_PARAM,
		INPUT4_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
		GATE_INPUT,
		CV_1_INPUT,
		CV_2_INPUT,
		CV_3_INPUT,
		CV_4_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
		OUTPUT1_OUTPUT,
		OUTPUT2_OUTPUT,
		OUTPUT3_OUTPUT,
		OUTPUT4_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		LIGHT3_LIGHT,
		LIGHT2_LIGHT,
		LIGHT4_LIGHT,
		LIGHT1_LIGHT,
		NUM_LIGHTS
	};

	int open[4] = {0};
	dsp::SchmittTrigger trigger;

	Chance()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(INPUT1_PARAM, 0.f, 1.f, 0.5f, "Chance For Output 1");
		configParam(INPUT2_PARAM, 0.f, 1.f, 0.5f, "Chance For Output 2");
		configParam(INPUT3_PARAM, 0.f, 1.f, 0.5f, "Chance For Output 3");
		configParam(INPUT4_PARAM, 0.f, 1.f, 0.5f, "Chance For Output 4");
	}

	void process(const ProcessArgs &args) override
	{
		if (!inputs[GATE_INPUT].isConnected())
			return;

		const float v = inputs[GATE_INPUT].getVoltage();

		if (trigger.process(rescale(v, 0.1f, 2.0f, 0.0f, 1.0f)))
		{
			for (int i = 0; i < 4; ++i)
			{
				open[i] = random::uniform() < params[i].getValue() * abs(inputs[i + 1].getNormalVoltage(10.0f)) / 10.0f ? 1 : 0;
				if(open[i] == 1)
					lights[i].setSmoothBrightness(1.0,0.1);
				else
					lights[i].setSmoothBrightness(0.0,0.1);
			}
		}

		for (int i = 0; i < 4; ++i)
		{
			if (outputs[i].isConnected())
				outputs[i].setVoltage(v * open[i]);
		}
	}
};

struct ChanceWidget : ModuleWidget
{
	ChanceWidget(Chance *module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Chance.svg")));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(5.08, 24.094)), module, Chance::INPUT1_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(5.08, 48.187)), module, Chance::INPUT2_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(5.08, 72.281)), module, Chance::INPUT3_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(5.08, 96.375)), module, Chance::INPUT4_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.966, 12.047)), module, Chance::GATE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.966, 24.094)), module, Chance::CV_1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.966, 48.187)), module, Chance::CV_2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.966, 72.281)), module, Chance::CV_3_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.966, 96.375)), module, Chance::CV_4_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.966, 36.141)), module, Chance::OUTPUT1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.966, 60.234)), module, Chance::OUTPUT2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.966, 84.328)), module, Chance::OUTPUT3_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.966, 108.422)), module, Chance::OUTPUT4_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(5.644, 117.511)), module, Chance::LIGHT3_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(12.041, 117.511)), module, Chance::LIGHT2_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(18.439, 117.511)), module, Chance::LIGHT4_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(24.836, 117.511)), module, Chance::LIGHT1_LIGHT));
	}
};

Model *modelChance = createModel<Chance, ChanceWidget>("Chance");