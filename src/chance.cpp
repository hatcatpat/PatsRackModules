#include "plugin.hpp"
#include "common.h"

struct Chance : Module
{
	enum ParamIds
	{
		ONE_PARAM,
		TWO_PARAM,
		THREE_PARAM,
		FOUR_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
		GATE_INPUT,
		ONE_INPUT,
		TWO_INPUT,
		THREE_INPUT,
		FOUR_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
		ONE_OUTPUT,
		TWO_OUTPUT,
		THREE_OUTPUT,
		FOUR_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds
	{
		ONE_LIGHT,
		TWO_LIGHT,
		THREE_LIGHT,
		FOUR_LIGHT,
		NUM_LIGHTS
	};

	int open[4] = {0};
	dsp::SchmittTrigger trigger;

	Chance()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(ONE_PARAM, 0.f, 1.f, 0.5f, "Chance for output 1");
		configParam(TWO_PARAM, 0.f, 1.f, 0.5f, "Chance for output 2");
		configParam(THREE_PARAM, 0.f, 1.f, 0.5f, "Chance for output 3");
		configParam(FOUR_PARAM, 0.f, 1.f, 0.5f, "Chance for output 4");
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
				if (open[i] == 1)
					lights[i].setSmoothBrightness(1.0, 0.1);
				else
					lights[i].setSmoothBrightness(0.0, 0.1);
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
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/chance.svg")));

		addParam(createParamCentered<Rogan1PPink>(mm2px(Vec(7.62, 36.141)), module, Chance::ONE_PARAM));
		addParam(createParamCentered<Rogan1PPink>(mm2px(Vec(7.62, 60.234)), module, Chance::TWO_PARAM));
		addParam(createParamCentered<Rogan1PPink>(mm2px(Vec(7.62, 84.328)), module, Chance::THREE_PARAM));
		addParam(createParamCentered<Rogan1PPink>(mm2px(Vec(7.62, 108.422)), module, Chance::FOUR_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.705, 20.078)), module, Chance::GATE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.86, 36.141)), module, Chance::ONE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.86, 60.234)), module, Chance::TWO_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.86, 84.328)), module, Chance::THREE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.86, 108.422)), module, Chance::FOUR_INPUT));

		addOutput(createOutputCentered<PJ301MOutputPort>(mm2px(Vec(33.02, 36.141)), module, Chance::ONE_OUTPUT));
		addOutput(createOutputCentered<PJ301MOutputPort>(mm2px(Vec(33.02, 60.234)), module, Chance::TWO_OUTPUT));
		addOutput(createOutputCentered<PJ301MOutputPort>(mm2px(Vec(33.02, 84.328)), module, Chance::THREE_OUTPUT));
		addOutput(createOutputCentered<PJ301MOutputPort>(mm2px(Vec(33.02, 108.422)), module, Chance::FOUR_OUTPUT));

		addChild(createLightCentered<MediumLight<PinkLight>>(mm2px(Vec(22.325, 20.078)), module, Chance::ONE_LIGHT));
		addChild(createLightCentered<MediumLight<PinkLight>>(mm2px(Vec(27.405, 20.078)), module, Chance::TWO_LIGHT));
		addChild(createLightCentered<MediumLight<PinkLight>>(mm2px(Vec(32.485, 20.078)), module, Chance::THREE_LIGHT));
		addChild(createLightCentered<MediumLight<PinkLight>>(mm2px(Vec(37.565, 20.078)), module, Chance::FOUR_LIGHT));
	}
};

Model *modelChance = createModel<Chance, ChanceWidget>("chance");