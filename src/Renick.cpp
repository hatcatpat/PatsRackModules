#include "plugin.hpp"

struct Renick : Module
{
	enum ParamIds
	{
		A_PARAM,
		B_PARAM,
		C_PARAM,
		D_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
		GATE_INPUT,
		A_CV_INPUT,
		B_CV_INPUT,
		C_CV_INPUT,
		D_CV_INPUT,
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

	dsp::SchmittTrigger trigger;

	std::vector<int> rules[4];
	std::vector<int> word;
	int selection = 0;
	int pos = 0;
	int count = 0;
	int mod = 1;
	int depth = 0;
	int open = 0;

	const static int DEPTH_MAX = 4;
	const static int WORD_MAX = 8;

	Renick()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(A_PARAM, 1.0f, 16.0f, 1.0f, "Integer value of symbol a");
		configParam(B_PARAM, 1.0f, 16.0f, 1.0f, "Integer value of symbol b");
		configParam(C_PARAM, 1.0f, 16.0f, 1.0f, "Integer value of symbol c");
		configParam(D_PARAM, 1.0f, 16.0f, 1.0f, "Integer value of symbol d");
	}

	void process(const ProcessArgs &args) override
	{
		if (!inputs[GATE_INPUT].isConnected())
			return;

		// if (rules[0].size() + rules[1].size() + rules[2].size() + rules[3].size() == 0)
			// if(word.size() == 0)
				// return;

		if (word.size() == 0)
		{
			word.push_back(0);
			mod = letterIdToValue(word[pos]);
		}

		const float v = inputs[GATE_INPUT].getVoltage();

		if (trigger.process(rescale(v, 0.1f, 2.0f, 0.0f, 1.0f)))
		{
			if (count == 0)
			{
				pos++;
				pos %= word.size();

				if (pos == 0)
				{
					if (depth < DEPTH_MAX)
					{
						updateWord();
						depth++;
					}
					else
					{
						depth = 0;
						word.clear();
						word.push_back(0);
					}
				}
				mod = letterIdToValue(word[pos]);

				open = 1;
			}
			else
			{
				open = 0;
			}
			count++;
			count %= mod;
		}

		if (outputs[OUTPUT_OUTPUT].isConnected())
			outputs[OUTPUT_OUTPUT].setVoltage(v * open);
	}

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

	int letterIdToValue(const int id)
	{
		float v = 1;
		switch (id)
		{
		case 0:
			v = params[A_PARAM].getValue();
			break;
		case 1:
			v = params[B_PARAM].getValue();
			break;
		case 2:
			v = params[C_PARAM].getValue();
			break;
		case 3:
			v = params[D_PARAM].getValue();
			break;
		}
		return (int)floor(v);
	}

	void moveUp()
	{
		selection--;
		if (selection < 0)
			selection = 3;
	}

	void moveDown()
	{
		selection++;
		selection %= 4;
	}

	void clearSelection()
	{
		rules[selection].clear();
	}

	void addLetter(const int letter_id)
	{
		if (rules[selection].size() < 4)
		{
			rules[selection].push_back(letter_id);
		}
	}
};

struct RenickVec : Widget
{
	Renick *module;
	int vec_id;

	RenickVec(const Vec &pos, Renick *module, const int vec_id)
	{
		box.pos = pos;
		box.size = mm2px(Vec(25.4, 5.354));
		this->module = module;
		this->vec_id = vec_id;
	}

	void onButton(const event::Button &e) override
	{
		if (e.action == GLFW_PRESS)
			module->selection = vec_id;
	}

	void draw(const DrawArgs &args) override
	{
		if (!module)
			return;

		nvgFillColor(args.vg, module->selection == vec_id ? nvgRGB(153, 153, 153) : nvgRGB(230, 230, 230));
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
		nvgFill(args.vg);

		nvgStrokeColor(args.vg, nvgRGB(0, 0, 0));
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
		nvgStroke(args.vg);

		if (module->rules[vec_id].size() == 0)
			return;

		std::string S = "";
		for (auto s : module->rules[vec_id])
		{
			S += " ";
			switch (s)
			{
			case 0:
				S += "a";
				break;
			case 1:
				S += "b";
				break;
			case 2:
				S += "c";
				break;
			case 3:
				S += "d";
				break;
			}
		}

		nvgFontSize(args.vg, 10);
		nvgFillColor(args.vg, nvgRGB(0, 0, 0));
		nvgText(args.vg, 0, box.size.y * 0.85, S.c_str(), NULL);
	}
};

struct RenickDisplay : Widget
{
	Renick *module;

	int x_unit;

	RenickDisplay(const Vec &pos, Renick *module)
	{
		box.pos = pos;
		box.size = mm2px(Vec(30.48, 5.354));
		x_unit = (float)box.size.x / Renick::WORD_MAX;
		this->module = module;
	}

	void draw(const DrawArgs &args) override
	{
		if (!module)
			return;

		nvgFillColor(args.vg, nvgRGB(230, 230, 230));
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
		nvgFill(args.vg);

		nvgStrokeColor(args.vg, nvgRGB(0, 0, 0));
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
		nvgStroke(args.vg);

		nvgFontSize(args.vg, 10);
		nvgFillColor(args.vg, nvgRGB(0, 0, 0));
		int i = 0;
		for (auto s : module->word)
		{
			if(i == module->pos)
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
			nvgText(args.vg, (i+0.5)*x_unit, box.size.y * 0.85, S.c_str(), NULL);
			++i;
		}

	}
};

struct RenickButton : SvgWidget
{
	Renick *module;

	RenickButton(const Vec &pos, Renick *module)
	{
		box.pos = pos;
		this->module = module;
	}
};

struct RenickUpButton : RenickButton
{
	std::shared_ptr<Svg> pressed_svg, regular_svg;

	RenickUpButton(const Vec &pos, Renick *module) : RenickButton(pos, module)
	{
		regular_svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/RenickButtons/Up.svg"));
		pressed_svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/RenickButtons/Up_Pressed.svg"));

		setSvg(regular_svg);
	}

	void onButton(const event::Button &e) override
	{
		if (e.action == GLFW_PRESS)
		{
			module->moveUp();
			setSvg(pressed_svg);
		}
		else
		{
			setSvg(regular_svg);
		}
	}
};

struct RenickDownButton : RenickButton
{
	RenickDownButton(const Vec &pos, Renick *module) : RenickButton(pos, module)
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/RenickButtons/Down.svg")));
	}

	void onButton(const event::Button &e) override
	{
		if (e.action == GLFW_PRESS)
			module->moveDown();
	}
};

struct RenickClearButton : RenickButton
{
	RenickClearButton(const Vec &pos, Renick *module) : RenickButton(pos, module)
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/RenickButtons/Clear.svg")));
	}

	void onButton(const event::Button &e) override
	{
		if (e.action == GLFW_PRESS)
			module->clearSelection();
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

		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/RenickButtons/" + letter + ".svg")));
	}

	void onButton(const event::Button &e) override
	{
		if (e.action == GLFW_PRESS)
		{
			module->addLetter(letter_id);
		}
	}
};

struct RenickWidget : ModuleWidget
{
	RenickWidget(Renick *module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Renick.svg")));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(12.7, 21.417 - 5.08 / 2.0f)), module, Renick::A_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(22.86, 21.417 - 5.08 / 2.0f)), module, Renick::B_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(12.7, 30.706 + 5.08 / 2.0f)), module, Renick::C_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(22.86, 30.706 + 5.08 / 2.0f)), module, Renick::D_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24 + 5.08 / 2.0f, 8.168 + 5.08 / 2.0f)), module, Renick::GATE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.08, 21.417 - 5.08 / 2.0f)), module, Renick::A_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.48, 21.417 - 5.08 / 2.0f)), module, Renick::B_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.08, 30.706 + 5.08 / 2.0f)), module, Renick::C_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.48, 30.706 + 5.08 / 2.0f)), module, Renick::D_CV_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.86, 115.252 + 5.08 / 2.0f)), module, Renick::OUTPUT_OUTPUT));

		// mm2px(Vec(5.08, 5.354))
		// addChild(createWidget<Widget>(mm2px(Vec(2.54, 50.865))));
		addChild(new RenickUpButton(mm2px(Vec(2.54, 50.865)), module));
		// mm2px(Vec(5.08, 5.354))
		addChild(new RenickDownButton(mm2px(Vec(11.06, 50.865)), module));
		// mm2px(Vec(13.6, 5.354))
		// addChild(createWidget<Widget>(mm2px(Vec(19.5, 50.865))));
		addChild(new RenickClearButton(mm2px(Vec(19.5, 50.865)), module));
		// mm2px(Vec(5.08, 5.354))
		// addChild(createWidget<Widget>(mm2px(Vec(2.46, 61.573))));
		addChild(new RenickLetterButton(mm2px(Vec(2.46, 61.573)), module, "a"));
		// mm2px(Vec(5.08, 5.354))
		// addChild(createWidget<Widget>(mm2px(Vec(10.98, 61.573))));
		addChild(new RenickLetterButton(mm2px(Vec(10.98, 61.573)), module, "b"));
		// mm2px(Vec(5.08, 5.354))
		// addChild(createWidget<Widget>(mm2px(Vec(19.5, 61.573))));
		addChild(new RenickLetterButton(mm2px(Vec(19.5, 61.573)), module, "c"));
		// mm2px(Vec(5.08, 5.354))
		// addChild(createWidget<Widget>(mm2px(Vec(28.02, 61.573))));
		addChild(new RenickLetterButton(mm2px(Vec(28.02, 61.573)), module, "d"));
		// mm2px(Vec(25.4, 5.354))
		// addChild(createWidget<Widget>(mm2px(Vec(7.62, 72.281))));
		addChild(new RenickVec(mm2px(Vec(7.62, 72.281)), module, 0));
		// mm2px(Vec(25.4, 5.354))
		// addChild(createWidget<Widget>(mm2px(Vec(7.62, 82.99))));
		addChild(new RenickVec(mm2px(Vec(7.62, 82.99)), module, 1));
		// mm2px(Vec(25.4, 5.354))
		// addChild(createWidget<Widget>(mm2px(Vec(7.62, 93.698))));
		addChild(new RenickVec(mm2px(Vec(7.62, 93.698)), module, 2));
		// mm2px(Vec(25.4, 5.354))
		// addChild(createWidget<Widget>(mm2px(Vec(7.62, 104.406))));
		addChild(new RenickVec(mm2px(Vec(7.62, 104.406)), module, 3));

		addChild(new RenickDisplay(mm2px(Vec(2.54, 42.833)), module));
	}
};

Model *modelRenick = createModel<Renick, RenickWidget>("Renick");