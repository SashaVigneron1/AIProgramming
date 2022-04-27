#pragma once
#include "SteeringBehaviours.h"

#include "Structs.h"

//****************
//BLENDED STEERING
class BlendedSteering final : public ISteeringBehaviour
{
public:
	struct WeightedBehavior
	{
		ISteeringBehaviour* pBehavior = nullptr;
		float weight = 0.f;

		WeightedBehavior(ISteeringBehaviour* pBehavior, float weight) :
			pBehavior(pBehavior),
			weight(weight)
		{};
	};

	BlendedSteering(vector<WeightedBehavior> weightedBehaviors);

	void AddBehaviour(WeightedBehavior weightedBehavior) { m_WeightedBehaviors.push_back(weightedBehavior); }
	virtual SteeringPlugin_Output_Extended CalculateSteering(float deltaT, const AgentInfo& pAgent) override;

	// returns a reference to the weighted behaviors, can be used to adjust weighting. Is not intended to alter the behaviors themselves.
	vector<WeightedBehavior>& GetWeightedBehaviorsRef() { return m_WeightedBehaviors; }

private:
	vector<WeightedBehavior> m_WeightedBehaviors = {};
};

//*****************
//PRIORITY STEERING
class PrioritySteering final : public ISteeringBehaviour
{
public:
	PrioritySteering(vector<ISteeringBehaviour*> priorityBehaviors)
		:m_PriorityBehaviors(priorityBehaviors)
	{}

	void AddBehaviour(ISteeringBehaviour* pBehavior) { m_PriorityBehaviors.push_back(pBehavior); }
	virtual SteeringPlugin_Output_Extended CalculateSteering(float deltaT, const AgentInfo& pAgent) override;

private:
	vector<ISteeringBehaviour*> m_PriorityBehaviors = {};
};
