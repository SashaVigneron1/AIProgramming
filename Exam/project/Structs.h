#pragma once
#include "Exam_HelperStructs.h"

struct SteeringPlugin_Output_Extended : SteeringPlugin_Output
{
	bool IsValid = false;
};

enum class SteeringType {
	Seek = 0,
	Flee = 1,
	FaceFlee = 2,
	Wander = 3,
	Evade = 4,
	SeekFlee = 5
};

