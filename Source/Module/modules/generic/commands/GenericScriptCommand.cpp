/*
  ==============================================================================

    GenericScriptCommand.cpp
    Created: 6 Apr 2019 1:01:44pm
    Author:  bkupe

  ==============================================================================
*/

#include "GenericScriptCommand.h"
#include "UI/ChataigneAssetManager.h"

String GenericScriptCommand::commandScriptTemplate = "";

GenericScriptCommand::GenericScriptCommand(ChataigneGenericModule * _module, CommandContext context, var params, IteratorProcessor* iterator) :
	BaseCommand(_module, context, params, iterator),
	script(this, false)
{

	addChildControllableContainer(&script);

	if(commandScriptTemplate.isEmpty()) commandScriptTemplate = ChataigneAssetManager::getInstance()->getScriptTemplateBundle(StringArray("generic", "command"));
	script.scriptTemplate = &commandScriptTemplate;
}

GenericScriptCommand::~GenericScriptCommand()
{
}

void GenericScriptCommand::setValueInternal(var value, int iterationIndex)
{
	Array<var> values;
	values.add(value);
	script.callFunction(setValueId, values);
}

void GenericScriptCommand::triggerInternal(int iterationIndex)
{
	if(context != MAPPING) script.callFunction(triggerId, Array<var>());
}

var GenericScriptCommand::getJSONData()
{
	var data = BaseCommand::getJSONData();
	data.getDynamicObject()->setProperty("script", script.getJSONData());
	return data;
}

void GenericScriptCommand::loadJSONDataInternal(var data)
{
	BaseCommand::loadJSONDataInternal(data);
	script.loadJSONData(data.getProperty("script", var()));
}

BaseCommand * GenericScriptCommand::create(ControllableContainer * module, CommandContext context, var params, IteratorProcessor * iterator)
{
	return new GenericScriptCommand((ChataigneGenericModule *)module, context, params, iterator);

}

