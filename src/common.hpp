#include "plugin.hpp"

extern Plugin* pluginInstance;

struct PJ301MOutputPort : app::SvgPort {
	PJ301MOutputPort() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/common/PJ301M_OUT.svg")));
	}
};

struct Rogan1PPink : Rogan {
	Rogan1PPink() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/common/Rogan1PPink.svg")));
	}
};

struct Rogan1PPinkSmall : Rogan {
	Rogan1PPinkSmall() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/common/Rogan1PPinkSmall.svg")));
	}
};

struct PatSwitch : SVGSwitch
{
	PatSwitch(){
		momentary = false;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/common/toggle/off.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/common/toggle/on.svg")));
	}
};

struct PatSwitchLarge : SVGSwitch
{
	PatSwitchLarge(){
		momentary = false;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/common/toggle_large/off.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/common/toggle_large/on.svg")));
	}
};

struct PatButton : SVGSwitch
{
	PatButton(){
		momentary = true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/common/button/off.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/common/button/on.svg")));
	}
};

static const NVGcolor PAT_PINK = nvgRGB(232,97,122);

template <typename TBase = GrayModuleLightWidget>
struct TPinkLight : TBase {
	TPinkLight() {
		this->addBaseColor(PAT_PINK);
	}
};
typedef TPinkLight<> PinkLight;