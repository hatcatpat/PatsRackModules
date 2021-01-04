#include "plugin.hpp"

#include "common.hpp"

struct Holdme : Module
{
	enum ParamIds
	{
		_MIN_PARAM,
		_MAX_PARAM,
		START_PARAM,
		END_PARAM,
		TOGGLE_PARAM,
		GATE_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
		_MIN_INPUT,
		_MAX_INPUT,
		START_INPUT,
		END_INPUT,
		TOGGLE_INPUT,
		GATE_INPUT,
		INPUT_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
		OUTPUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds
	{
		NUM_LIGHTS
	};

	dsp::SchmittTrigger gate_trigger, toggle_trigger;
	bool gating = false;
	float last_output = 0.f;

	Holdme()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(_MIN_PARAM, -5.f, 10.f, 0.f, "Minimum value of the input");
		configParam(_MAX_PARAM, -5.f, 10.f, 10.f, "Maximum value of the input");
		configParam(START_PARAM, -5.f, 10.f, 0.f, "Minimum value of the output");
		configParam(END_PARAM, -5.f, 10.f, 10.f, "Maximum value of the output");
		configParam(TOGGLE_PARAM, 0.f, 1.f, 0.f, "Toggles whether or not to sample+hold");
		configParam(GATE_PARAM, 0.f, 1.f, 0.f, "Triggers a sampling");
	}

	void process(const ProcessArgs &args) override
	{

		gating = params[TOGGLE_PARAM].getValue() > 0.5f;
		if (inputs[TOGGLE_INPUT].isConnected())
		{
			if (toggle_trigger.process(rescale(inputs[TOGGLE_INPUT].getVoltage(), 0.1f, 2.f, 0.f, 1.f)))
			{
				gating = !gating;
				params[TOGGLE_PARAM].setValue(gating ? 1.f : 0.f);
			}
		}

		if (inputs[INPUT_INPUT].isConnected())
		{
			if (gating)
			{
				bool should_sample = params[GATE_PARAM].getValue() > 0.5f;
				if (inputs[GATE_INPUT].isConnected())
					should_sample = should_sample || gate_trigger.process(rescale(inputs[GATE_INPUT].getVoltage(), 0.1f, 2.f, 0.f, 1.f));

				if (should_sample)
				{
					outputs[OUTPUT_OUTPUT].setVoltage(mapRange());
					last_output = outputs[OUTPUT_OUTPUT].getVoltage();
				}
			}
			else
			{
				outputs[OUTPUT_OUTPUT].setVoltage(mapRange());
				last_output = outputs[OUTPUT_OUTPUT].getVoltage();
			}
		}
	}

	float mapRange()
	{
		const float min_value = params[_MIN_PARAM].getValue() * abs(inputs[_MIN_INPUT].getNormalVoltage(10.f)) / 10.f;
		const float max_value = params[_MAX_PARAM].getValue() * abs(inputs[_MAX_INPUT].getNormalVoltage(10.f)) / 10.f;
		const float start_value = params[START_PARAM].getValue() * abs(inputs[START_INPUT].getNormalVoltage(10.f)) / 10.f;
		const float end_value = params[END_PARAM].getValue() * abs(inputs[END_INPUT].getNormalVoltage(10.f)) / 10.f;

		if (max_value == min_value)
			return 0.f;
		else
			return rescale(inputs[INPUT_INPUT].getVoltage(), min_value, max_value, start_value, end_value);
		// return (inputs[INPUT_INPUT].getVoltage() - min_value) * (end_value - start_value) / (max_value - min_value) + start_value;
	}
};

//==================================================
struct HoldmeDisplay : Widget
{

	Holdme *module;
	Rect bounds;

	HoldmeDisplay(const Vec &pos, Holdme *module)
	{
		float w = mm2px(40.64);
		float h = mm2px(8.031);
		float scx = 0.9;
		float scy = 0.75;

		box.pos.x = pos.x + w * (1.f - scx) / 2.f;
		box.pos.y = pos.y + h * (1.f - scy) / 2.f;
		box.size = Vec(w * scx, h * scy);

		this->module = module;
	}

	void draw(const DrawArgs &args) override
	{

		nvgStrokeColor(args.vg, PAT_PINK);
		nvgStrokeWidth(args.vg, mm2px(0.5));
		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 0.5);
		nvgStroke(args.vg);

		if (module)
		{
			float w = module->last_output / 10.f * (box.size.x * 2.f / 3.f);
			float m = 0.6;

			nvgFillColor(args.vg, PAT_PINK);
			nvgBeginPath(args.vg);
			nvgRect(args.vg, box.size.x / 3.f, box.size.y * (1.f - m) / 2.f, w, box.size.y * m);
			nvgFill(args.vg);
		}

		nvgFillColor(args.vg, PAT_PINK);
		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, box.size.x / 3.f, 0, mm2px(0.5), box.size.y, 1.f);
		nvgFill(args.vg);

		nvgFillColor(args.vg, PAT_PINK);
		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, 2.f * box.size.x / 3.f, 0, mm2px(0.5), box.size.y, 1.f);
		nvgFill(args.vg);
	}
};

//==================================================
struct HoldmeWidget : ModuleWidget
{
	HoldmeWidget(Holdme *module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/holdme.svg")));

		addParam(createParamCentered<Rogan1PPinkSmall>(mm2px(Vec(20.32, 24.094)), module, Holdme::_MIN_PARAM));
		addParam(createParamCentered<Rogan1PPinkSmall>(mm2px(Vec(20.32, 34.133)), module, Holdme::_MAX_PARAM));
		addParam(createParamCentered<Rogan1PPinkSmall>(mm2px(Vec(20.32, 44.172)), module, Holdme::START_PARAM));
		addParam(createParamCentered<Rogan1PPinkSmall>(mm2px(Vec(20.32, 54.211)), module, Holdme::END_PARAM));

		addParam(createParamCentered<PatSwitch>(mm2px(Vec(20.32, 70.273)), module, Holdme::TOGGLE_PARAM));
		addParam(createParamCentered<PatButton>(mm2px(Vec(20.32, 80.312)), module, Holdme::GATE_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.48, 24.094)), module, Holdme::_MIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.48, 34.133)), module, Holdme::_MAX_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.48, 44.172)), module, Holdme::START_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.48, 54.211)), module, Holdme::END_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.48, 70.273)), module, Holdme::TOGGLE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.48, 80.312)), module, Holdme::GATE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 104.406)), module, Holdme::INPUT_INPUT));

		addOutput(createOutputCentered<PJ301MOutputPort>(mm2px(Vec(25.4, 104.406)), module, Holdme::OUTPUT_OUTPUT));

		// mm2px(Vec(40.64, 8.031))
		addChild(new HoldmeDisplay(mm2px(Vec(-0.0, 88.344)), module));
	}
};

Model *modelHoldme = createModel<Holdme, HoldmeWidget>("holdme");