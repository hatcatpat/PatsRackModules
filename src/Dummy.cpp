#include "plugin.hpp"


struct Dummy : Module {
	enum ParamIds {
		INPUT1_PARAM,
		INPUT2_PARAM,
		INPUT3_PARAM,
		INPUT4_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		GATE_INPUT,
		CV_1_INPUT,
		CV_2_INPUT,
		CV_3_INPUT,
		CV_4_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
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

	Dummy() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(INPUT1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(INPUT2_PARAM, 0.f, 1.f, 0.f, "");
		configParam(INPUT3_PARAM, 0.f, 1.f, 0.f, "");
		configParam(INPUT4_PARAM, 0.f, 1.f, 0.f, "");
	}

	void process(const ProcessArgs& args) override {
	}
};


struct DummyWidget : ModuleWidget {
	DummyWidget(Dummy* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Dummy.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(5.08, 24.094)), module, Dummy::INPUT1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(5.08, 48.187)), module, Dummy::INPUT2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(5.08, 72.281)), module, Dummy::INPUT3_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(5.08, 96.375)), module, Dummy::INPUT4_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.966, 12.047)), module, Dummy::GATE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.966, 24.094)), module, Dummy::CV_1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.966, 48.187)), module, Dummy::CV_2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.966, 72.281)), module, Dummy::CV_3_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.966, 96.375)), module, Dummy::CV_4_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.966, 36.141)), module, Dummy::OUTPUT1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.966, 60.234)), module, Dummy::OUTPUT2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.966, 84.328)), module, Dummy::OUTPUT3_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.966, 108.422)), module, Dummy::OUTPUT4_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(5.644, 117.511)), module, Dummy::LIGHT3_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(12.041, 117.511)), module, Dummy::LIGHT2_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(18.439, 117.511)), module, Dummy::LIGHT4_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(24.836, 117.511)), module, Dummy::LIGHT1_LIGHT));
	}
};


Model* modelDummy = createModel<Dummy, DummyWidget>("Dummy");