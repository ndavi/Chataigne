/*
  ==============================================================================

    EnumComparator.h
    Created: 19 Feb 2017 5:48:15pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "../BaseComparator.h"

class EnumComparator :
	public BaseComparator
{
public:
	EnumComparator(Parameter * sourceParam);
	virtual ~EnumComparator();

	const Identifier equalsId = "=";
	const Identifier differentId = "!=";

	EnumParameter * enumRef;

	virtual bool compare(Parameter* sourceParam) override;
};