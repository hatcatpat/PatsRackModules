#include "plugin.hpp"

Plugin *pluginInstance;

void init(Plugin *p)
{
	pluginInstance = p;

	// Add modules here
	// p->addModel(modelMyModule);
	p->addModel(modelChance);
	p->addModel(modelRenick);
	p->addModel(modelPolyamory);
	p->addModel(modelSnap);
	p->addModel(modelTimothy);
	p->addModel(modelHoldme);
	p->addModel(modelPete);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
