#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput Steering{};

	if (pFlock->GetNrOfNeighbors() == 0)
		return Steering;

	Target.Position = pFlock->GetAverageNeighborPos();
	//Steering.LinearVelocity = pFlock->GetAverageNeighborPos() - pAgent.GetPosition();
	//Steering.LinearVelocity.Normalize();
	Steering = Seek::CalculateSteering(deltaT, pAgent);

	return Steering;
}

//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput Steering{};

	if (pFlock->GetNrOfNeighbors() == 0)
		return Steering;

	TArray<ASteeringAgent*> const pNeighbors = pFlock->GetNeighbors();
	for (int neighborIdx = 0; neighborIdx < pFlock->GetNrOfNeighbors(); ++neighborIdx)
	{
		FVector2D ToAgent = pAgent.GetPosition() - pNeighbors[neighborIdx]->GetPosition();
		float Distance = ToAgent.Size();
		if (Distance > 0)
			Steering.LinearVelocity += ToAgent.GetSafeNormal() / Distance;
	}
	Steering.LinearVelocity.Normalize();

	return Steering;
}

//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput VelocityMatch::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput Steering{};

	if (pFlock->GetNrOfNeighbors() == 0)
		return Steering;

	Steering.LinearVelocity = pFlock->GetAverageNeighborVelocity();
	Steering.LinearVelocity.Normalize();

	return Steering;
}
