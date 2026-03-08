#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput Steering{};
	Steering = Seek::CalculateSteering(deltaT, pAgent);
	return Steering;
}

//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput Steering{};

	TArray<ASteeringAgent*> const pNeighbors = pFlock->GetNeighbors();
	for (int neighborIdx = 0; neighborIdx < pFlock->GetNrOfNeighbors(); ++neighborIdx)
	{
		FVector2D ToAgent = pAgent.GetPosition() - pNeighbors[neighborIdx]->GetPosition();
		float Distance = ToAgent.Size();
		if (Distance > 0)
			Steering.LinearVelocity += ToAgent / (Distance * Distance);
	}
	Steering.LinearVelocity.Normalize();

	return Steering;
}

//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput VelocityMatch::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput Steering{};

	Steering.LinearVelocity = pFlock->GetAverageNeighborVelocity();
	Steering.LinearVelocity.Normalize();

	return Steering;
}
