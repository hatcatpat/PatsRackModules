#include "plugin.hpp"

#include "common.h"

struct Renick : Module
{
	enum ParamIds
	{
		A_PARAM,
		B_PARAM,
		C_PARAM,
		D_PARAM,
		TIME_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
		A_INPUT,
		B_INPUT,
		C_INPUT,
		D_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
		GATE_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds
	{
		NUM_LIGHTS
	};

	dsp::Timer timer;
	dsp::PulseGenerator pulse;
	dsp::SchmittTrigger trigger;

	std::vector<int> rules[4];
	std::vector<int> word;
	int selection = 0;
	int pos = 0;
	float dur = 500.0f;

	const static int WORD_MAX = 16;
	const static int RULE_MAX = 8;

	//==================================================
	Renick()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(A_PARAM, 0.f, 1000.f, 250.f, "Sets the duration of symbol a");
		configParam(B_PARAM, 0.f, 1000.f, 500.f, "Sets the duration of symbol b");
		configParam(C_PARAM, 0.f, 1000.f, 750.f, "Sets the duration of symbol c");
		configParam(D_PARAM, 0.f, 1000.f, 1000.f, "Sets the duration of symbol d");
		configParam(TIME_PARAM, 0.f, 16.f, 1.f, "Divides the durations of all symbols");
	}

	//==================================================
	json_t *dataToJson() override
	{
		json_t *root_json = json_object();

		json_object_set_new(root_json, "selection", json_integer(selection));
		json_object_set_new(root_json, "pos", json_integer(pos));

		json_t *word_json = json_array();
		for (auto s : word)
		{
			json_t *s_json = json_integer((int)s);
			json_array_append_new(word_json, s_json);
		}
		json_object_set_new(root_json, "word", word_json);

		for (int i = 0; i < 4; ++i)
		{
			json_t *rules_json = json_array();
			for (auto s : rules[i])
			{
				json_t *s_json = json_integer((int)s);
				json_array_append_new(rules_json, s_json);
			}
			json_object_set_new(root_json, ("rule_" + std::to_string(i)).c_str(), rules_json);
		}

		return root_json;
	}

	void dataFromJson(json_t *root_json) override
	{
		json_t *temp_json = json_object_get(root_json, "selection");
		if (temp_json)
			selection = json_integer_value(temp_json);

		temp_json = json_object_get(root_json, "pos");
		if (temp_json)
			pos = json_integer_value(temp_json);

		temp_json = json_object_get(root_json, "word");
		if (temp_json)
		{
			for (size_t i = 0; i < json_array_size(temp_json); ++i)
				word.push_back(json_integer_value(json_array_get(temp_json, i)));
		}

		for (int i = 0; i < 4; ++i)
		{
			temp_json = json_object_get(root_json, ("rule_" + std::to_string(i)).c_str());
			if (temp_json)
			{
				for (size_t j = 0; j < json_array_size(temp_json); ++j)
					rules[i].push_back(json_integer_value(json_array_get(temp_json, j)));
			}
		}
	}

	//==================================================
	void process(const ProcessArgs &args) override
	{
		const float t = 1.0f / args.sampleRate;

		if (word.size() == 0)
		{
			pos = 0;
			word.push_back(0);
			dur = params[word[0]].getValue() * abs(inputs[word[0]].getNormalVoltage(10.f)) / 10.f;
		}

		if (timer.process(t) >= dur / (1000.f * fmax(0.01, params[TIME_PARAM].getValue())))
		{
			pulse.trigger(1e-3f);
			timer.reset();

			pos++;
			pos %= word.size();

			if (pos == 0)
			{
				updateWord();
			}
			dur = params[word[pos]].getValue() * abs(inputs[word[pos]].getNormalVoltage(10.f)) / 10.f;
		}

		const float pulse_v = pulse.process(t) ? 10.0f : 0.0f;
		if (outputs[GATE_OUTPUT].isConnected())
			outputs[GATE_OUTPUT].setVoltage(pulse_v);
	}

	//==================================================
	void reset()
	{
		word.clear();
		for (int i = 0; i < 4; ++i)
			rules[i].clear();
		pos = 0;
		timer.reset();
	}

	//==================================================
	void updateWord()
	{
		std::vector<int> new_word;
		for (auto x : word)
		{
			for (auto r : rules[x])
			{
				if (new_word.size() >= WORD_MAX)
					break;
				new_word.push_back(r);
			}
			if (new_word.size() >= WORD_MAX)
				break;
		}
		word = new_word;
	}

	//==================================================
	void moveUp()
	{
		selection--;
		if (selection < 0)
			selection = 3;
	}

	//==================================================
	void moveDown()
	{
		selection++;
		selection %= 4;
	}

	//==================================================
	void clearSelection()
	{
		rules[selection].clear();
	}

	//==================================================
	void addLetter(const int letter_id)
	{
		if (rules[selection].size() < RULE_MAX)
		{
			rules[selection].push_back(letter_id);
		}
	}
};
//==================================================
//==================================================
//==================================================

//==================================================
// Shows the current word and position
struct RenickWordDisplay : Widget
{
	Renick *module;

	int x_unit;

	RenickWordDisplay(const Vec &pos, Renick *module)
	{
		box.pos = pos;
		box.size = mm2px(Vec(50.8, 4.016));
		x_unit = (float)box.size.x / Renick::WORD_MAX;
		this->module = module;
	}

	void draw(const DrawArgs &args) override
	{
		nvgFillColor(args.vg, nvgRGB(230, 230, 230));
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
		nvgFill(args.vg);

		nvgStrokeColor(args.vg, nvgRGB(0, 0, 0));
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
		nvgStroke(args.vg);

		if (!module)
			return;

		nvgFontSize(args.vg, 10);
		nvgFillColor(args.vg, PAT_PINK);
		int i = 0;
		for (auto s : module->word)
		{
			if (i == module->pos)
				nvgFillColor(args.vg, nvgRGB(255, 0, 0));
			else
				nvgFillColor(args.vg, nvgRGB(0, 0, 0));

			std::string S = "";
			switch (s)
			{
			case 0:
				S = "a";
				break;
			case 1:
				S = "b";
				break;
			case 2:
				S = "c";
				break;
			case 3:
				S = "d";
				break;
			}
			nvgText(args.vg, (i + 0.5) * x_unit, box.size.y * 0.85, S.c_str(), NULL);
			++i;
		}
	}
};

//==================================================
struct RenickRuleDisplay : Widget
{
	Renick *module;
	int letter_id;
	float x_unit;

	RenickRuleDisplay(const Vec &pos, Renick *module, const std::string letter)
	{
		box.pos = pos;
		box.size = mm2px(Vec(35.560, 4.016));
		this->module = module;
		x_unit = (float)box.size.x / Renick::RULE_MAX;

		if (letter == "a")
			letter_id = 0;
		if (letter == "b")
			letter_id = 1;
		if (letter == "c")
			letter_id = 2;
		if (letter == "d")
			letter_id = 3;
	}

	void onButton(const event::Button &e) override
	{
		if (e.action == GLFW_PRESS)
			module->selection = letter_id;
	}

	void draw(const DrawArgs &args) override
	{
		if (!module)
			return;

		nvgFillColor(args.vg, nvgRGB(230, 230, 230));
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
		nvgFill(args.vg);

		nvgStrokeColor(args.vg, module->selection == letter_id ? PAT_PINK : nvgRGB(0, 0, 0));
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
		nvgStroke(args.vg);

		if (module->rules[letter_id].size() == 0)
			return;

		nvgFontSize(args.vg, 10);
		nvgFillColor(args.vg, PAT_PINK);
		int i = 0;
		for (auto s : module->rules[letter_id])
		{
			std::string S = "";
			switch (s)
			{
			case 0:
				S = "a";
				break;
			case 1:
				S = "b";
				break;
			case 2:
				S = "c";
				break;
			case 3:
				S = "d";
				break;
			}

			nvgText(args.vg, (i + 0.5) * x_unit, box.size.y * 0.85, S.c_str(), NULL);
			++i;
		}
	}
};

//==================================================
struct RenickButton : SvgWidget
{
	Renick *module;

	std::shared_ptr<Svg> pressed_svg, regular_svg;

	RenickButton(const Vec &pos, Renick *module)
	{
		box.pos = pos;
		this->module = module;
	}

	virtual void onLeftClick() {}

	void onButton(const event::Button &e) override
	{
		if (e.action == GLFW_PRESS)
		{
			onLeftClick();
			setSvg(pressed_svg);
		}
		else
		{
			setSvg(regular_svg);
		}
	}
};

struct RenickUpButton : RenickButton
{
	RenickUpButton(const Vec &pos, Renick *module) : RenickButton(pos, module)
	{
		regular_svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/renick_buttons/up.svg"));
		pressed_svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/renick_buttons/up_pressed.svg"));

		setSvg(regular_svg);
	}

	void onLeftClick() override
	{
		module->moveUp();
	}
};

struct RenickDownButton : RenickButton
{
	RenickDownButton(const Vec &pos, Renick *module) : RenickButton(pos, module)
	{
		regular_svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/renick_buttons/down.svg"));
		pressed_svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/renick_buttons/down_pressed.svg"));

		setSvg(regular_svg);
	}

	void onLeftClick() override
	{
		module->moveDown();
	}
};

struct RenickClearButton : RenickButton
{
	RenickClearButton(const Vec &pos, Renick *module) : RenickButton(pos, module)
	{
		regular_svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/renick_buttons/clear.svg"));
		pressed_svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/renick_buttons/clear_pressed.svg"));

		setSvg(regular_svg);
	}

	void onLeftClick() override
	{
		module->clearSelection();
	}
};

struct RenickResetButton : RenickButton
{
	RenickResetButton(const Vec &pos, Renick *module) : RenickButton(pos, module)
	{
		regular_svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/renick_buttons/reset.svg"));
		pressed_svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/renick_buttons/reset_pressed.svg"));

		setSvg(regular_svg);
	}

	void onLeftClick() override
	{
		module->reset();
	}
};

struct RenickLetterButton : RenickButton
{

	int letter_id;

	RenickLetterButton(const Vec &pos, Renick *module, std::string letter) : RenickButton(pos, module)
	{
		if (letter == "a")
			letter_id = 0;
		if (letter == "b")
			letter_id = 1;
		if (letter == "c")
			letter_id = 2;
		if (letter == "d")
			letter_id = 3;

		const std::string name = "res/renick_buttons/" + letter;
		regular_svg = APP->window->loadSvg(asset::plugin(pluginInstance, name + ".svg"));
		pressed_svg = APP->window->loadSvg(asset::plugin(pluginInstance, name + "_pressed.svg"));

		setSvg(regular_svg);
	}

	void onLeftClick() override
	{
		module->addLetter(letter_id);
	}
};

//==================================================
//==================================================
//==================================================
struct RenickWidget : ModuleWidget
{
	RenickWidget(Renick *module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/renick.svg")));

		addParam(createParamCentered<Rogan1PPink>(mm2px(Vec(22.86, 24.094)), module, Renick::A_PARAM));
		addParam(createParamCentered<Rogan1PPink>(mm2px(Vec(38.1, 24.094)), module, Renick::B_PARAM));
		addParam(createParamCentered<Rogan1PPink>(mm2px(Vec(22.86, 40.156)), module, Renick::C_PARAM));
		addParam(createParamCentered<Rogan1PPink>(mm2px(Vec(38.1, 40.156)), module, Renick::D_PARAM));
		addParam(createParamCentered<Rogan1PPink>(mm2px(Vec(7.62, 58.227)), module, Renick::TIME_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 24.094)), module, Renick::A_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(50.8, 24.094)), module, Renick::B_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 40.156)), module, Renick::C_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(50.8, 40.156)), module, Renick::D_INPUT));

		addOutput(createOutputCentered<PJ301MOutputPort>(mm2px(Vec(53.34, 58.227)), module, Renick::GATE_OUTPUT));

		// mm2px(Vec(5.08, 4.016))
		addChild(new RenickUpButton(mm2px(Vec(17.78, 52.203)), module));
		// mm2px(Vec(5.08, 4.016))
		addChild(new RenickDownButton(mm2px(Vec(25.4, 52.203)), module));
		// mm2px(Vec(5.08, 4.016))
		addChild(new RenickClearButton(mm2px(Vec(40.64, 52.203)), module));

		addChild(new RenickResetButton(mm2px(Vec(33.02, 52.203)), module));
		// mm2px(Vec(5.08, 4.016))
		addChild(new RenickLetterButton(mm2px(Vec(17.78, 60.234)), module, "a"));
		// mm2px(Vec(5.08, 4.016))
		addChild(new RenickLetterButton(mm2px(Vec(25.4, 60.234)), module, "b"));
		// mm2px(Vec(5.08, 4.016))
		addChild(new RenickLetterButton(mm2px(Vec(33.02, 60.234)), module, "c"));
		// mm2px(Vec(5.08, 4.016))
		addChild(new RenickLetterButton(mm2px(Vec(40.64, 60.234)), module, "d"));
		// mm2px(Vec(50.8, 4.016))
		addChild(new RenickWordDisplay(mm2px(Vec(5.08, 70.273)), module));
		// mm2px(Vec(35.56, 4.016))
		addChild(new RenickRuleDisplay(mm2px(Vec(20.32, 80.312)), module, "a"));
		// mm2px(Vec(35.56, 4.016))
		addChild(new RenickRuleDisplay(mm2px(Vec(20.32, 88.344)), module, "b"));
		// mm2px(Vec(35.56, 4.016))
		addChild(new RenickRuleDisplay(mm2px(Vec(20.32, 96.375)), module, "c"));
		// mm2px(Vec(35.56, 4.016))
		addChild(new RenickRuleDisplay(mm2px(Vec(20.32, 104.406)), module, "d"));
	}
};

Model *modelRenick = createModel<Renick, RenickWidget>("renick");