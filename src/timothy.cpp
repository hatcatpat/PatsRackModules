#include "plugin.hpp"
#include "common.hpp"

struct Timothy : Module
{
	enum ParamIds
	{
		BPM_PARAM,
		RESET_PARAM,
		ON_PARAM,
		MUL_QUARTER_PARAM,
		MUL_HALF_PARAM,
		MUL_2_PARAM,
		MUL_4_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
		BPM_INPUT,
		RESET_INPUT,
		MUL_QUARTER_INPUT,
		MUL_HALF_INPUT,
		MUL_2_INPUT,
		MUL_4_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
		BPM_OUTPUT,
		_1_OUTPUT,
		_2_OUTPUT,
		_4_OUTPUT,
		_8_OUTPUT,
		_16_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds
	{
		_1_LIGHT,
		_2_LIGHT,
		_4_LIGHT,
		_8_LIGHT,
		_16_LIGHT,
		NUM_LIGHTS
	};

	dsp::Timer timer, reset_timer;
	dsp::PulseGenerator pulse;
	dsp::SchmittTrigger mul_triggers[4];
	dsp::SchmittTrigger reset_trigger;
	bool mul_toggles[4] = {false, false, false, false};
	int num_toggles = 0;
	float muls[4] = {0.25f, 0.5f, 2.f, 4.f};
	float mul = 1.f;
	float dur = 1.f;
	bool running = true, reset_timer_active = false;
	int count = 0;

	Timothy()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(BPM_PARAM, 1.f, 120.f * 16.f, 120.f, "BPM of the clock");
		configParam(RESET_PARAM, 0.f, 1.f, 0.f, "Resets clock");
		configParam(ON_PARAM, 0.f, 1.f, 1.f, "Toggles clock");
		configParam(MUL_QUARTER_PARAM, 0.f, 1.f, 0.f, "Multiplies BPM by 1/4");
		configParam(MUL_HALF_PARAM, 0.f, 1.f, 0.f, "Multiplies BPM by 1/2");
		configParam(MUL_2_PARAM, 0.f, 1.f, 0.f, "Multiplies BPM by 2");
		configParam(MUL_4_PARAM, 0.f, 1.f, 0.f, "Multiplies BPM by 4");
	}

	float getBPS()
	{
		if (inputs[BPM_INPUT].isConnected())
			return 60.f / rescale(abs(inputs[BPM_INPUT].getVoltage()), 0.f, 10.f, 1.f, 120.f * 16.f);
		else
			return 60.f / params[BPM_PARAM].getValue();
	}

	void process(const ProcessArgs &args) override
	{

		bool reset = false;
		reset = params[RESET_PARAM].getValue() > 0.5;
		if (inputs[RESET_INPUT].isConnected())
			if (reset_trigger.process(rescale(inputs[RESET_INPUT].getVoltage(), 0.1f, 2.f, 0.f, 1.f)))
				reset = true;

		if (reset)
		{
			timer.reset();
			count = 0;
			for (auto l : lights)
				l.setBrightness(0.f);
			reset_timer.reset();
			reset_timer_active = true;
		}

		if (reset_timer_active)
		{
			if (reset_timer.process(1.f / args.sampleRate) >= 0.001)
			{
				reset_timer_active = false;
			}
			else
			{
				return;
			}
		}

		num_toggles = 0;
		for (int i = 0; i < 4; ++i)
		{
			const bool last_toggle = mul_toggles[i];

			mul_toggles[i] = params[MUL_QUARTER_PARAM + i].getValue() > 0.5;
			if (inputs[MUL_QUARTER_INPUT + i].isConnected())
			{
				if (mul_triggers[i].process(rescale(inputs[MUL_QUARTER_INPUT + i].getVoltage(), 0.1f, 2.f, 0.f, 1.f)))
				{
					mul_toggles[i] = !mul_toggles[i];
					params[MUL_QUARTER_PARAM + i].setValue(mul_toggles[i] ? 1.f : 0.f);
				}
			}

			if (mul_toggles[i])
			{
				++num_toggles;
				if (last_toggle != mul_toggles[i])
				{
					mul = muls[i];
					for (int j = 0; j < 4; ++j)
					{
						if (i == j)
							continue;

						mul_toggles[j] = false;
						params[MUL_QUARTER_PARAM + j].setValue(0.f);
					}
				}
			}
		}

		running = params[ON_PARAM].getValue() > 0.5;
		if (running)
		{
			dur = getBPS();

			if (outputs[BPM_OUTPUT].isConnected())
				outputs[BPM_OUTPUT].setVoltage(rescale(dur, 60.f / 120.f * 16.f, 60.f / 1.f, 0.f, 10.f));

			if (num_toggles != 0)
				dur /= mul;

			const float r = 1.f / args.sampleRate;

			if (timer.process(r) >= dur)
			{
				timer.reset();
				pulse.trigger(1e-3f);

				lights[_1_LIGHT].setBrightness(lights[_1_LIGHT].getBrightness() > 0.5 ? 0.f : 1.f);
				if (count % 2 == 0)
					lights[_2_LIGHT].setBrightness(lights[_2_LIGHT].getBrightness() > 0.5 ? 0.f : 1.f);
				if (count % 4 == 0)
					lights[_4_LIGHT].setBrightness(lights[_4_LIGHT].getBrightness() > 0.5 ? 0.f : 1.f);
				if (count % 8 == 0)
					lights[_8_LIGHT].setBrightness(lights[_8_LIGHT].getBrightness() > 0.5 ? 0.f : 1.f);
				if (count % 16 == 0)
					lights[_16_LIGHT].setBrightness(lights[_16_LIGHT].getBrightness() > 0.5 ? 0.f : 1.f);

				count = (count + 1) % 16;
			}

			const float pulse_v = pulse.process(r) ? 10.0f : 0.0f;

			outputs[_1_OUTPUT].setVoltage(pulse_v);
			if (count % 2 == 0)
				outputs[_2_OUTPUT].setVoltage(pulse_v);
			if (count % 4 == 0)
				outputs[_4_OUTPUT].setVoltage(pulse_v);
			if (count % 8 == 0)
				outputs[_8_OUTPUT].setVoltage(pulse_v);
			if (count % 16 == 0)
				outputs[_16_OUTPUT].setVoltage(pulse_v);
		}
	}
};

struct TimothyWidget : ModuleWidget
{
	TimothyWidget(Timothy *module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/timothy.svg")));

		addParam(createParamCentered<Rogan1PPink>(mm2px(Vec(10.16, 24.094)), module, Timothy::BPM_PARAM));
		addParam(createParamCentered<PatButton>(mm2px(Vec(15.24, 40.156)), module, Timothy::RESET_PARAM));
		addParam(createParamCentered<PatSwitch>(mm2px(Vec(35.56, 40.156)), module, Timothy::ON_PARAM));
		addParam(createParamCentered<PatSwitchLarge>(mm2px(Vec(15.24, 72.281)), module, Timothy::MUL_QUARTER_PARAM));
		addParam(createParamCentered<PatSwitchLarge>(mm2px(Vec(15.24, 84.328)), module, Timothy::MUL_HALF_PARAM));
		addParam(createParamCentered<PatSwitchLarge>(mm2px(Vec(15.24, 96.375)), module, Timothy::MUL_2_PARAM));
		addParam(createParamCentered<PatSwitchLarge>(mm2px(Vec(15.24, 108.422)), module, Timothy::MUL_4_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.86, 24.094)), module, Timothy::BPM_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.4, 40.156)), module, Timothy::RESET_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(27.94, 72.281)), module, Timothy::MUL_QUARTER_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(27.94, 84.328)), module, Timothy::MUL_HALF_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(27.94, 96.375)), module, Timothy::MUL_2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(27.94, 108.422)), module, Timothy::MUL_4_INPUT));

		addOutput(createOutputCentered<PJ301MOutputPort>(mm2px(Vec(33.02, 24.094)), module, Timothy::BPM_OUTPUT));
		addOutput(createOutputCentered<PJ301MOutputPort>(mm2px(Vec(5.08, 40.156)), module, Timothy::_1_OUTPUT));
		addOutput(createOutputCentered<PJ301MOutputPort>(mm2px(Vec(5.08, 56.219)), module, Timothy::_2_OUTPUT));
		addOutput(createOutputCentered<PJ301MOutputPort>(mm2px(Vec(15.24, 56.219)), module, Timothy::_4_OUTPUT));
		addOutput(createOutputCentered<PJ301MOutputPort>(mm2px(Vec(25.4, 56.219)), module, Timothy::_8_OUTPUT));
		addOutput(createOutputCentered<PJ301MOutputPort>(mm2px(Vec(35.56, 56.219)), module, Timothy::_16_OUTPUT));

		addChild(createLightCentered<SmallLight<PinkLight>>(mm2px(Vec(5.08, 46.18)), module, Timothy::_1_LIGHT));
		addChild(createLightCentered<SmallLight<PinkLight>>(mm2px(Vec(5.08, 62.242)), module, Timothy::_2_LIGHT));
		addChild(createLightCentered<SmallLight<PinkLight>>(mm2px(Vec(15.24, 62.242)), module, Timothy::_4_LIGHT));
		addChild(createLightCentered<SmallLight<PinkLight>>(mm2px(Vec(25.4, 62.242)), module, Timothy::_8_LIGHT));
		addChild(createLightCentered<SmallLight<PinkLight>>(mm2px(Vec(35.56, 62.242)), module, Timothy::_16_LIGHT));
	}
};

Model *modelTimothy = createModel<Timothy, TimothyWidget>("timothy");