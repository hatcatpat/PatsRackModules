#include "plugin.hpp"

#include "common.h"

struct Polyamory : Module
{
	enum ParamIds
	{
		WIDTH_PARAM,
		CENTER_PARAM,
		MUL_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
		A_INPUT,
		B_INPUT,
		C_INPUT,
		D_INPUT,
		WIDTH_INPUT,
		CENTER_INPUT,
		MUL_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
		OUTPUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds
	{
		A_LIGHT,
		B_LIGHT,
		C_LIGHT,
		D_LIGHT,
		NUM_LIGHTS
	};

	Polyamory()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(WIDTH_PARAM, 0.f, 1.f, 0.1f, "The width of the region");
		configParam(CENTER_PARAM, 0.f, 1.f, 0.5f, "The center of the region, [0,1] -> [a,d]");
		configParam(MUL_PARAM, 0.f, 2.f, 1.f, "Multiplies the output");
	}

	void process(const ProcessArgs &args) override
	{

		const float w = params[WIDTH_PARAM].getValue() * abs(inputs[WIDTH_INPUT].getNormalVoltage(10.f)) / 10.f;
		const float c = params[CENTER_PARAM].getValue() * abs(inputs[CENTER_INPUT].getNormalVoltage(10.f)) / 10.f;
		const float m = params[MUL_PARAM].getValue() * abs(inputs[MUL_INPUT].getNormalVoltage(10.f)) / 10.f;

		if (inputs[A_INPUT].isConnected())
		{
			if (inputs[A_INPUT].isPolyphonic())
			{
				// use A's inputs instead of b,c,d
				std::vector<float> r;
				r.assign(inputs[A_INPUT].getChannels(), 0.f);

				float sum = 0.f;
				for (size_t i = 0; i < r.size(); ++i)
				{

					const float x = (float)(i + 0.5f) / r.size();
					const float d = abs(c - x);

					if (d >= w)
					{
						r[i] = 0.f;
					}
					else
					{
						r[i] = w - d;
					}

					sum += r[i] * inputs[A_INPUT].getVoltage(i);
				}

				sum /= r.size();
				sum *= m;
				outputs[OUTPUT_OUTPUT].setVoltage(sum);

				for (int i = 0; i < 4; ++i)
				{
					const float x = (float)(i + 0.5f) / 4.f;
					const float d = abs(c - x);

					if (d >= w)
					{
						r[i] = 0.f;
					}
					else
					{
						r[i] = w - d;
					}

					lights[i].setSmoothBrightness(r[i] * m, 0.01);
				}

				return;
			}
		}

		// otherwise, use a,b,c,d's inputs
		float r[4] = {0.f};

		int num_connections = 0;
		float sum = 0.f;
		for (int i = 0; i < 4; ++i)
		{
			if (inputs[i].isConnected())
				++num_connections;

			const float x = (float)(i + 0.5f) / 4.f;
			const float d = abs(c - x);

			if (d >= w)
			{
				r[i] = 0.f;
			}
			else
			{
				r[i] = w - d;
			}

			sum += r[i] * inputs[i].getNormalVoltage(0.f);

			lights[i].setSmoothBrightness(r[i] * m, 0.01);
		}

		if (num_connections == 0)
		{
			outputs[OUTPUT_OUTPUT].setVoltage(0.f);
			return;
		}

		sum /= num_connections;
		sum *= m;
		outputs[OUTPUT_OUTPUT].setVoltage(sum);
	}
};

struct PolyamoryWidget : ModuleWidget
{
	PolyamoryWidget(Polyamory *module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/polyamory.svg")));

		addParam(createParamCentered<Rogan1PPink>(mm2px(Vec(17.78, 60.234)), module, Polyamory::WIDTH_PARAM));
		addParam(createParamCentered<Rogan1PPink>(mm2px(Vec(17.78, 76.297)), module, Polyamory::CENTER_PARAM));
		addParam(createParamCentered<Rogan1PPink>(mm2px(Vec(17.78, 92.359)), module, Polyamory::MUL_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(12.7, 24.094)), module, Polyamory::A_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(27.94, 24.094)), module, Polyamory::B_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(12.7, 38.148)), module, Polyamory::C_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(27.94, 38.148)), module, Polyamory::D_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.48, 60.234)), module, Polyamory::WIDTH_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.48, 76.297)), module, Polyamory::CENTER_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.48, 92.359)), module, Polyamory::MUL_INPUT));

		addOutput(createOutputCentered<PJ301MOutputPort>(mm2px(Vec(20.32, 108.422)), module, Polyamory::OUTPUT_OUTPUT));

		addChild(createLightCentered<MediumLight<PinkLight>>(mm2px(Vec(5.08, 51.159)), module, Polyamory::A_LIGHT));
		addChild(createLightCentered<MediumLight<PinkLight>>(mm2px(Vec(15.24, 51.159)), module, Polyamory::B_LIGHT));
		addChild(createLightCentered<MediumLight<PinkLight>>(mm2px(Vec(25.4, 51.159)), module, Polyamory::C_LIGHT));
		addChild(createLightCentered<MediumLight<PinkLight>>(mm2px(Vec(35.56, 51.159)), module, Polyamory::D_LIGHT));
	}
};

Model *modelPolyamory = createModel<Polyamory, PolyamoryWidget>("polyamory");