#include "plugin.hpp"
#include "common.hpp"

struct Pete : Module
{
	enum ParamIds
	{
		BPM_PARAM,
		ON_PARAM,
		DIV_PARAM,
		SPEED_PARAM,
		MUL_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
		INPUT_INPUT,
		BPM_INPUT,
		ON_INPUT,
		DIV_INPUT,
		SPEED_INPUT,
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
		NUM_LIGHTS
	};

	int data_size = 0;
	int write_pos = 0;
	float read_pos = 0;
	std::vector<float> record_data, playback_data;
	bool active = false;

	dsp::SchmittTrigger on_trigger;

	Pete()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(BPM_PARAM, 1.f, 120.f * 16.f, 120.f, "BPM of the clock");
		configParam(ON_PARAM, 0.f, 1.f, 0.f, "Whether or not to active the repetition");
		configParam(DIV_PARAM, 0.f, 8.f, 0.f, "Number to divide the previous 4 beats by");
		configParam(SPEED_PARAM, -8.f, 8.f, 1.f, "Modifies the playback speed of the recorded loop");
		configParam(MUL_PARAM, 0.f, 2.f, 1.f, "Multiplies the output volume");
	}

	float getBPS()
	{
		if (inputs[BPM_INPUT].isConnected())
			return 60.f / rescale(abs(inputs[BPM_INPUT].getVoltage()), 0.f, 10.f, 1.f, 120.f * 16.f);
		else
			return 60.f / params[BPM_PARAM].getValue();
	}

	int getDiv()
	{
		return pow(2, static_cast<int>(params[DIV_PARAM].getValue() * abs(inputs[DIV_INPUT].getNormalVoltage(10.f)) / 10.f));
	}

	void process(const ProcessArgs &args) override
	{
		const int data_size = static_cast<int>(getBPS() * args.sampleRate * 4.f);
		if (data_size == 0)
			return;

		if ((int)record_data.size() != data_size)
		{
			while ((int)record_data.size() < data_size)
			{
				record_data.push_back(0.f);
			}
			while ((int)record_data.size() > data_size)
			{
				record_data.pop_back();
			}
		}

		if (inputs[INPUT_INPUT].isConnected())
		{
			record_data[write_pos % data_size] = inputs[INPUT_INPUT].getVoltage();
			write_pos = (write_pos + 1) % data_size;
		}

		bool now_active = params[ON_PARAM].getValue() > 0.5f;
		if (inputs[ON_INPUT].isConnected())
		{
			if (on_trigger.process(rescale(inputs[ON_INPUT].getVoltage(), 0.1f, 2.f, 0.f, 1.f)))
			{
				now_active = !now_active;
				params[ON_PARAM].setValue(now_active ? 1.f : 0.f);
			}
		}

		if (now_active)
		{ // currently active
			if (!active)
			{ // was not previously active
				playback_data.assign(record_data.begin(), record_data.end());
				read_pos = static_cast<int>((1.f - (float)1.f / getDiv()) * playback_data.size());
			}

			if (outputs[OUTPUT_OUTPUT].isConnected() && inputs[INPUT_INPUT].isConnected())
				outputs[OUTPUT_OUTPUT].setVoltage(playback_data[static_cast<int>(read_pos) % playback_data.size()] * params[MUL_PARAM].getValue() * inputs[MUL_INPUT].getNormalVoltage(10.f) / 10.f);

			read_pos += params[SPEED_PARAM].getValue() * abs(inputs[SPEED_INPUT].getNormalVoltage(10.f)) / 10.f;

			if (read_pos > (int)playback_data.size() || read_pos < 0)
				read_pos = static_cast<int>((1.f - (float)1.f / getDiv()) * playback_data.size());
		}
		else
		{
			if (outputs[OUTPUT_OUTPUT].isConnected())
				outputs[OUTPUT_OUTPUT].setVoltage(inputs[INPUT_INPUT].getNormalVoltage(0.f));
		}

		active = now_active;
	}
};

struct PeteWidget : ModuleWidget
{
	PeteWidget(Pete *module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/pete.svg")));

		addParam(createParamCentered<Rogan1PPinkSmall>(mm2px(Vec(5.08, 46.18)), module, Pete::BPM_PARAM));
		addParam(createParamCentered<PatSwitch>(mm2px(Vec(5.08, 62.242)), module, Pete::ON_PARAM));
		addParam(createParamCentered<Rogan1PPinkSmall>(mm2px(Vec(5.08, 78.305)), module, Pete::DIV_PARAM));
		addParam(createParamCentered<Rogan1PPinkSmall>(mm2px(Vec(5.08, 94.367)), module, Pete::SPEED_PARAM));
		addParam(createParamCentered<Rogan1PPinkSmall>(mm2px(Vec(5.08, 110.43)), module, Pete::MUL_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(12.7, 18.07)), module, Pete::INPUT_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 46.18)), module, Pete::BPM_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 62.242)), module, Pete::ON_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 78.305)), module, Pete::DIV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 94.367)), module, Pete::SPEED_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 110.43)), module, Pete::MUL_INPUT));

		addOutput(createOutputCentered<PJ301MOutputPort>(mm2px(Vec(12.7, 28.109)), module, Pete::OUTPUT_OUTPUT));
	}
};

Model *modelPete = createModel<Pete, PeteWidget>("pete");