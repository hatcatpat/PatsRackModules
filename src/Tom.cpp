#include "plugin.hpp"

struct Tom : Module
{
	enum ParamIds
	{
		TEMPO_PARAM,
		RESET_PARAM,
		RANDOMISE_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
		TEMPO_CV_INPUT,
		RESET_CV_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
		OUTPUT1_OUTPUT,
		OUTPUT2_OUTPUT,
		OUTPUT3_OUTPUT,
		OUTPUT4_OUTPUT,
		OUTPUT5_OUTPUT,
		OUTPUT6_OUTPUT,
		OUTPUT7_OUTPUT,
		OUTPUT8_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds
	{
		NUM_LIGHTS
	};

	float pos[8] = {0.0f};
	float speed[8] = {1.0f};
	dsp::PulseGenerator pulseGenerator[8];
	dsp::SchmittTrigger resetTrigger;
	const float rate = 1 / APP->engine->getSampleRate();
	bool randomise = false;
	dsp::SchmittTrigger randomiseTrigger;

	Tom()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(TEMPO_PARAM, 0.f, 16.f, 1.f, "Tempo");
		configParam(RESET_PARAM, 0.f, 1.f, 0.f, "Reset Positions");
		configParam(RANDOMISE_PARAM, 0.f, 1.f, 0.f, "Toggle Random Offsets");

		for (int i = 0; i < 8; ++i)
		{
			speed[i] = 9 - i;
		}
	}

	void process(const ProcessArgs &args) override
	{
		for (int o = 0; o < 8; ++o)
		{
			pos[o] += (rate / speed[o]) * params[TEMPO_PARAM].getValue();
			if (pos[o] >= 1)
			{
				pos[o] = fmod(pos[o], 1.0f);
				pulseGenerator[o].trigger(1e-3f);
			}

			if (outputs[o].isConnected())
				outputs[o].setVoltage(pulseGenerator[o].process(rate) ? 10.0f : 0.0f);
		}

		if (resetTrigger.process(
				params[RESET_PARAM].getValue() + inputs[RESET_CV_INPUT].getNormalVoltage(0)))
		{
			if (randomise)
				randomiseOffsets();
			else
				memset(pos, 0.0f, sizeof(pos));
		}

		randomise = params[RANDOMISE_PARAM].getValue() > 0.5f;
	}

	void randomiseOffsets()
	{
		pos[0] = 0.0f;
		for (int o = 1; o < 8; ++o)
		{
			pos[o] = (random::uniform() * 6.0f + 1.0f) / 8.0f;
		}
	}
};

struct TomSlider : SvgWidget
{
	Tom *module;
	int id;

	TomSlider(const Vec &pos, Tom *module, int id)
	{
		this->module = module;
		this->id = id;
		box.pos = pos;
		box.size = Vec(0, 0);
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TomSlider.svg")));
	}

	void draw(const DrawArgs &args) override
	{
		if (!module)
			return;

		const auto c = nvgRGB(100,100,100);

		nvgStrokeColor(args.vg, nvgRGB(230,230,230) );
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
		nvgFill(args.vg);

		nvgFillColor(args.vg, c);
		nvgBeginPath(args.vg);
		nvgRect(args.vg, box.size.x * (1 - 0.05) * module->pos[id], 0, box.size.x * 0.05, box.size.y);
		nvgFill(args.vg);

		nvgStrokeColor(args.vg, c);
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
		nvgStroke(args.vg);
	}
};

struct TomButton : SVGSwitch
{
	TomButton()
	{
		momentary = true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Button/TomButtonOff.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Button/TomButtonOn.svg")));
	}
};

struct TomToggle : SVGSwitch
{
	TomToggle()
	{
		momentary = false;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Button/TomButtonOff.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Button/TomButtonOn.svg")));
	}
};

struct TomWidget : ModuleWidget
{
	TomWidget(Tom *module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Tom.svg")));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(15.24, 16.062)), module, Tom::TEMPO_PARAM));
		addParam(createParamCentered<TomButton>(mm2px(Vec(25.4, 16.062)), module, Tom::RESET_PARAM));
		addParam(createParamCentered<TomToggle>(mm2px(Vec(22.3925, 11.111)), module, Tom::RANDOMISE_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.08, 16.062)), module, Tom::TEMPO_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(35.56, 16.062)), module, Tom::RESET_CV_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(35.56, 28.109)), module, Tom::OUTPUT1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(35.56, 40.156)), module, Tom::OUTPUT2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(35.56, 52.203)), module, Tom::OUTPUT3_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(35.56, 64.25)), module, Tom::OUTPUT4_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(35.56, 76.297)), module, Tom::OUTPUT5_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(35.56, 88.344)), module, Tom::OUTPUT6_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(35.56, 100.391)), module, Tom::OUTPUT7_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(35.56, 112.437)), module, Tom::OUTPUT8_OUTPUT));

		addChild(new TomSlider(mm2px(Vec(5.08/2, 24.094)), module, 0));
		addChild(new TomSlider(mm2px(Vec(5.08/2, 36.141)), module, 1));
		addChild(new TomSlider(mm2px(Vec(5.08/2, 48.187)), module, 2));
		addChild(new TomSlider(mm2px(Vec(5.08/2, 60.234)), module, 3));
		addChild(new TomSlider(mm2px(Vec(5.08/2, 72.281)), module, 4));
		addChild(new TomSlider(mm2px(Vec(5.08/2, 84.328)), module, 5));
		addChild(new TomSlider(mm2px(Vec(5.08/2, 96.375)), module, 6));
		addChild(new TomSlider(mm2px(Vec(5.08/2, 108.422)), module, 7));
	}
};

Model *modelTom = createModel<Tom, TomWidget>("Tom");