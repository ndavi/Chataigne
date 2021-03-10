/*
  ==============================================================================

	SimpleRemapFilter.cpp
	Created: 4 Feb 2017 5:39:07pm
	Author:  Ben

  ==============================================================================
*/

SimpleRemapFilter::SimpleRemapFilter(const String& name, var params, Multiplex* multiplex) :
	MappingFilter(name, params, multiplex),
	targetIn(nullptr),
	targetOut(nullptr)
{
	useCustomInputRange = filterParams.addBoolParameter("Use Custom Input Range", "If enabled, the input range will be set from the source input range", false);

	targetIn = filterParams.addPoint2DParameter("Input Min/Max", "Custom input range", false);
	targetIn->setPoint(0, 1);

	targetOut = filterParams.addPoint2DParameter("Target Min/Max", "New maximum for output");
	targetOut->isCustomizableByUser = false;
	targetOut->setPoint(0, 1);

	forceFloatOutput = filterParams.addBoolParameter("Force Float", "If checked, this will force transforming integer in floats", false);

	filterTypeFilters.add(Controllable::FLOAT, Controllable::INT, Controllable::BOOL, Controllable::POINT2D, Controllable::POINT3D);
	autoSetRange = false;
}

SimpleRemapFilter::SimpleRemapFilter(var params, Multiplex* multiplex) :
	SimpleRemapFilter("Remap", params, multiplex) //using getTypeString makes mac crash...
{

}

SimpleRemapFilter::~SimpleRemapFilter()
{
}

Parameter* SimpleRemapFilter::setupSingleParameterInternal(Parameter* source, int multiplexIndex)
{
	Parameter* p = nullptr;
	if (!source->isComplex() && forceFloatOutput->boolValue())
	{
		p = new FloatParameter(source->niceName, source->description, source->value, source->minimumValue, source->maximumValue);
		p->isSavable = false;
		p->setControllableFeedbackOnly(true);
	}
	else
	{
		p = MappingFilter::setupSingleParameterInternal(source, multiplexIndex);
	}

	if (!useCustomInputRange->isOverriden || !useCustomInputRange->boolValue()) useCustomInputRange->setValue(!source->hasRange());
	if (p->isComplex())
	{
		var minVal;
		var maxVal;
		for (int i = 0; i < p->value.size(); i++)
		{
			minVal.append(jmin<float>(targetOut->x, targetOut->y));
			maxVal.append(jmax<float>(targetOut->x, targetOut->y));
		}
		p->setRange(minVal, maxVal);
	}
	else
	{
		p->setRange(jmin<float>(targetOut->x, targetOut->y), jmax<float>(targetOut->x, targetOut->y));
	}
	return p;
}

MappingFilter::ProcessResult  SimpleRemapFilter::processSingleParameterInternal(Parameter* source, Parameter* out, int multiplexIndex)
{
	if (targetIn == nullptr || targetOut == nullptr || out == nullptr) return STOP_HERE;

	var remappedValue = getRemappedValueFor(source, multiplexIndex);
	out->setValue(remappedValue);

	return CHANGED;
}

var SimpleRemapFilter::getRemappedValueFor(Parameter* source, int multiplexIndex)
{
	var sourceVal = source->getValue();
	var targetVal = sourceVal;

	var linkOut = filterParams.getLinkedValue(targetOut, multiplexIndex);

	if (linkOut[0] == linkOut[1])
	{
		if (source->isComplex())
		{
			var tVal;
			for (int i = 0; i < source->value.size(); i++) tVal.append(linkOut[0]);
			return tVal;
		}
		else
		{
			return linkOut[0];
		}
	}

	var tOut;
	if (source->isComplex())
	{
		for (int i = 0; i < source->value.size(); i++)
		{
			var ttOut;
			ttOut.append(linkOut[0]);
			ttOut.append(linkOut[1]);
			tOut.append(ttOut);
		}
	}
	else
	{
		tOut = linkOut;

	}


	var linkIn = filterParams.getLinkedValue(targetIn, multiplexIndex);

	var tIn;
	if (!source->hasRange() || useCustomInputRange->boolValue())
	{
		if (source->isComplex())
		{
			for (int i = 0; i < source->value.size(); i++)
			{
				var ttIn;
				ttIn.append(linkIn[0]);
				ttIn.append(linkIn[1]);
				tIn.append(ttIn);
			}
		}
		else
		{
			tIn = linkIn;
		}
	}
	else
	{
		tIn.append(source->minimumValue);
		tIn.append(source->maximumValue);
	}


	if (sourceVal.isArray())
	{
		var targetVal;
		for (int i = 0; i < sourceVal.size(); i++)
		{
			var ttIn = tIn[i];
			var ttOut = tOut[i];

			if (ttIn[0] != ttIn[1] && ttOut[0] != ttOut[1])
			{
				float targetValue = jmap((float)sourceVal[i], (float)ttIn[0], (float)ttIn[1], (float)ttOut[0], (float)ttOut[1]);
				targetVal.append(targetValue);
			}
			else
			{
				targetVal.append(sourceVal[i]);
			}
		}

		return targetVal;
	}
	else
	{
		if (tIn[0] != tIn[1] && tOut[0] != tOut[1])
		{
			return jmap(source->floatValue(), (float)tIn[0], (float)tIn[1], (float)tOut[0], (float)tOut[1]);
		}
	}

	return source->getValue();
}



void SimpleRemapFilter::filterParamChanged(Parameter* p)
{
	if (p == useCustomInputRange)
	{
		targetIn->setEnabled(useCustomInputRange->boolValue());

	}
	else if (p == targetOut)
	{
		for (int i = 0; i < filteredParameters.size(); i++)
		{
			auto mFilteredParams = filteredParameters[i];

			for (auto& f : *mFilteredParams)
			{
				var mRange = filterParams.getLinkedValue(targetOut, i);

				if (f->type == Controllable::FLOAT || f->type == Controllable::INT)
				{
					f->setRange(jmin<float>(mRange[0], mRange[1]), jmax<float>(mRange[0], mRange[1]));
				}
				else if(f->type == Controllable::POINT2D || f->type == Controllable::POINT3D)
				{
					var minVal;
					var maxVal;
					for (int i = 0; i < f->value.size(); i++)
					{
						minVal.append(jmin<float>(mRange[0], mRange[1]));
						maxVal.append(jmax<float>(mRange[0], mRange[1]));
					}

					f->setRange(minVal, maxVal);

				}
			}
		}

		mappingFilterListeners.call(&FilterListener::filteredParamRangeChanged, this);
	}
	else if (p == forceFloatOutput)
	{
		if (!isCurrentlyLoadingData) setupParametersInternal(-1);

		mappingFilterListeners.call(&FilterListener::filteredParamsChanged, this);
		filterAsyncNotifier.addMessage(new FilterEvent(FilterEvent::FILTER_REBUILT, this));
	}
}
