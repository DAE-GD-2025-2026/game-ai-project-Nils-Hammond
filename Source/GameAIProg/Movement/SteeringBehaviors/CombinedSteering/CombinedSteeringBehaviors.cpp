
#include "CombinedSteeringBehaviors.h"
#include <algorithm>
#include "../SteeringAgent.h"

BlendedSteering::BlendedSteering(const std::vector<WeightedBehavior>& WeightedBehaviors)
	:WeightedBehaviors(WeightedBehaviors)
{};

//****************
//BLENDED STEERING
SteeringOutput BlendedSteering::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput BlendedSteering = {};
	// TODO: Calculate the weighted average steeringbehavior
	for (const WeightedBehavior& weightedBehavior : WeightedBehaviors)
	{
		weightedBehavior.pBehavior->SetTarget(Target);
		SteeringOutput steering = weightedBehavior.pBehavior->CalculateSteering(DeltaT, Agent);
		steering *= weightedBehavior.Weight;
		BlendedSteering = BlendedSteering + steering;
	}
	BlendedSteering.LinearVelocity.Normalize();
	
	// TODO: Add debug drawing
	DrawDebugLine(Agent.GetWorld(), FVector(Agent.GetPosition(), 0), FVector(Target.Position + BlendedSteering.LinearVelocity * Agent.GetMaxLinearSpeed(), 0), FColor::Green, false, -1.f, 0, 5.f);

	return BlendedSteering;
}

float* BlendedSteering::GetWeight(ISteeringBehavior* const SteeringBehavior)
{
	auto it = find_if(WeightedBehaviors.begin(),
		WeightedBehaviors.end(),
		[SteeringBehavior](const WeightedBehavior& Elem)
		{
			return Elem.pBehavior == SteeringBehavior;
		}
	);

	if(it!= WeightedBehaviors.end())
		return &it->Weight;
	
	return nullptr;
}

//*****************
//PRIORITY STEERING
SteeringOutput PrioritySteering::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering = {};

	for (ISteeringBehavior* const pBehavior : m_PriorityBehaviors)
	{
		pBehavior->SetTarget(Target);
		Steering = pBehavior->CalculateSteering(DeltaT, Agent);
		if (Steering.IsValid)
			break;
	}

	//If non of the behavior return a valid output, last behavior is returned
	return Steering;
}