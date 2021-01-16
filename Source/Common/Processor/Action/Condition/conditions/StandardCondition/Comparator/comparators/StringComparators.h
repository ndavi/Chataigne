/*
  ==============================================================================

    StringComparators.h
    Created: 2 Nov 2016 8:58:35pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "../BaseComparator.h"

class StringComparator :
	public BaseComparator
{
public:
	StringComparator(Parameter * sourceParam);
	virtual ~StringComparator();

	const Identifier equalsId = "=";
	const Identifier differentId = "!=";
	const Identifier containsId = "contains";
	const Identifier startsWith = "startsWith";
	const Identifier endsWidth = "endsWidth";

	virtual bool compare(Parameter *sourceParam) override;
};