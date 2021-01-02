#include "plugin.hpp"
#include "common.hpp"

struct Snap : Module
{
	enum ParamIds
	{
		BPM_PARAM,
		DUR_PARAM,
		DIV_PARAM,
		GATE_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
		BPM_INPUT,
		DUR_INPUT,
		DIV_INPUT,
		GATE_INPUT,
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

	dsp::Timer timer;
	dsp::PulseGenerator pulse;
	dsp::SchmittTrigger gate_trigger;
	bool active = false;
	float div = 1.f;
	int count = 0;

	Snap()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(BPM_PARAM, 1.f, 120.f * 16.f, 120.f, "BPM of the main clock");
		configParam(DUR_PARAM, 0.f, 128.f, 1.f, "Duration in beats");
		configParam(DIV_PARAM, 1.f, 16.f, 3.f, "Number of beats to subdivide");
		configParam(GATE_PARAM, 0.f, 1.f, 0.f, "Triggers a 'snap'");
	}

	void process(const ProcessArgs &args) override
	{
		bool gate = false;
		gate = params[GATE_PARAM].getValue() > 0.5;
		if (inputs[GATE_INPUT].isConnected())
			if (gate_trigger.process(rescale(inputs[GATE_INPUT].getVoltage(), 0.1f, 2.f, 0.f, 1.f)))
				gate = true;

		if (gate)
		{
			div = floor(params[DIV_PARAM].getValue() * abs(inputs[DIV_INPUT].getNormalVoltage(10.f)) / 10.f);
			timer.reset();
			pulse.trigger(1e-3f);
			count = 0;
			active = true;
		}

		const float r = 1.f / args.sampleRate;

		if (active)
		{
			float dur = 60.f / (params[BPM_PARAM].getValue() * abs(inputs[BPM_INPUT].getNormalVoltage(10.f)) / 10.f);
			dur *= params[DUR_PARAM].getValue() * abs(inputs[DUR_INPUT].getNormalVoltage(10.f)) / 10.f;
			dur /= div;

			if (count >= div-1)
			{
				count = 0;
				active = false;
			}
			else
			{
				if (timer.process(r) >= dur)
				{
					timer.reset();
					pulse.trigger(1e-3f);
					++count;
				}
			}
		}

		const float pulse_v = pulse.process(r) ? 10.0f : 0.0f;
		outputs[OUTPUT_OUTPUT].setVoltage(pulse_v);
	}
};

struct SnapWidget : ModuleWidget
{
	SnapWidget(Snap *module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/snap.svg")));

		addParam(createParamCentered<Rogan1PPink>(mm2px(Vec(10.16, 26.102)), module, Snap::BPM_PARAM));
		addParam(createParamCentered<Rogan1PPink>(mm2px(Vec(10.16, 44.172)), module, Snap::DUR_PARAM));
		addParam(createParamCentered<Rogan1PPink>(mm2px(Vec(10.16, 70.273)), module, Snap::DIV_PARAM));
		addParam(createParamCentered<PatButton>(mm2px(Vec(10.16, 88.344)), module, Snap::GATE_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.86, 26.102)), module, Snap::BPM_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.86, 44.172)), module, Snap::DUR_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.86, 70.273)), module, Snap::DIV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.86, 88.344)), module, Snap::GATE_INPUT));

		addOutput(createOutputCentered<PJ301MOutputPort>(mm2px(Vec(15.24 + 2.54 / 2.0, 106.414)), module, Snap::OUTPUT_OUTPUT));
	}
};

Model *modelSnap = createModel<Snap, SnapWidget>("snap");