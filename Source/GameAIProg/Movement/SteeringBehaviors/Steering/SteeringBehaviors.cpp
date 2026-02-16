#include "SteeringBehaviors.h"
#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"

//SEEK
//*******
// TODO: Do the Week01 assignment :^)
SteeringOutput Seek::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{};

	Steering.LinearVelocity = Target.Position - Agent.GetPosition();
	
	// Add debug Rendering
	if (Agent.GetDebugRenderingEnabled())
	{
		DrawDebugLine(Agent.GetWorld(), FVector(Agent.GetPosition(), 0), FVector(Target.Position, 0), FColor::Green, false, -1.f, 0, 5.f);
		DrawDebugLine(Agent.GetWorld(), FVector(Agent.GetPosition(), 0), FVector(Agent.GetPosition() + Steering.LinearVelocity, 0), FColor::Red, false, -1.f, 0, 5.f);
		DrawDebugCircle(Agent.GetWorld(), FVector(Target.Position, 0), 5.f, 12, FColor::Green, false, -1.f, 0, 5.f, FVector(1, 0, 0), FVector(0, 1, 0), false);
	}

	return Steering;
}

//FLEE
SteeringOutput Flee::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{};
	Steering.LinearVelocity = Agent.GetPosition() - Target.Position;
	// Add debug Rendering

	return Steering;
}

//ARRIVE
SteeringOutput Arrive::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{};
	OriginalMaxSpeed = OriginalMaxSpeed < 0.f ? Agent.GetMaxLinearSpeed() : OriginalMaxSpeed;
	float SlowRadius = 500.f;
	float TargetRadius = 100.f;

	FVector2D ToTarget = Target.Position - Agent.GetPosition();
	float Distance = ToTarget.Size();

	if (Distance < TargetRadius)
	{
		Agent.SetMaxLinearSpeed(OriginalMaxSpeed);
		Steering.LinearVelocity = FVector2D::ZeroVector;
		return Steering;
	}

	float TargetSpeed = OriginalMaxSpeed;
	if (Distance < SlowRadius)
	{
		TargetSpeed = OriginalMaxSpeed * Distance / SlowRadius;
	}
	Agent.SetMaxLinearSpeed(TargetSpeed);
	Steering.LinearVelocity = Target.Position - Agent.GetPosition();

	if (Agent.GetDebugRenderingEnabled())
	{
		DrawDebugCircle(Agent.GetWorld(), FVector(Agent.GetPosition(), 0), SlowRadius, 32, FColor::Blue, false, -1.f, 0, 5.f, FVector(1, 0, 0), FVector(0, 1, 0), false);
		DrawDebugCircle(Agent.GetWorld(), FVector(Agent.GetPosition(), 0), TargetRadius, 32, FColor::Red, false, -1.f, 0, 5.f, FVector(1, 0, 0), FVector(0, 1, 0), false);
	}

	return Steering;
}

//FACE
SteeringOutput Face::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{};

	FVector ToTarget = FVector(Target.Position - Agent.GetPosition(), 0);
	Steering.AngularVelocity = FMath::FindDeltaAngleDegrees(Agent.GetRotation(), ToTarget.Rotation().Yaw);
	return Steering;
}

//PURSUIT
SteeringOutput Pursuit::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{};
	FVector2D ToTarget = Target.Position - Agent.GetPosition();
	FVector2D predictedTargetPos = Target.Position + Target.LinearVelocity * ToTarget.Size() / Agent.GetMaxLinearSpeed();
	Steering.LinearVelocity = predictedTargetPos - Agent.GetPosition();
	return Steering;
}

//EVADE
SteeringOutput Evade::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{};
	FVector2D ToTarget = Target.Position - Agent.GetPosition();
	FVector2D predictedTargetPos = Target.Position + Target.LinearVelocity * ToTarget.Size() / Agent.GetMaxLinearSpeed();
	Steering.LinearVelocity = Agent.GetPosition() - predictedTargetPos;
	return Steering;
}

//WANDER
SteeringOutput Wander::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{};
	m_WanderAngle += FMath::RandRange(-m_MaxAngleChange / 2.f, m_MaxAngleChange / 2.f);
	FVector2D CircleCenter = Agent.GetPosition() + FVector2D(Agent.GetActorForwardVector().X, Agent.GetActorForwardVector().Y) * m_OffsetDistance;
	FVector2D Displacement = FVector2D(FMath::Cos(m_WanderAngle), FMath::Sin(m_WanderAngle)) * m_Radius;

	Target.Position = CircleCenter + Displacement;

	if (Agent.GetDebugRenderingEnabled())
	{
		DrawDebugCircle(Agent.GetWorld(), FVector(CircleCenter, 0), m_Radius, 32, FColor::Blue, false, -1.f, 0, 5.f, FVector(1, 0, 0), FVector(0, 1, 0), false);
		//DrawDebugCircle(Agent.GetWorld(), FVector(Target.Position, 0), 10.f, 12, FColor::Green, false, -1.f, 0, 5.f, FVector(1, 0, 0), FVector(0, 1, 0), false);
		DrawDebugLine(Agent.GetWorld(), FVector(Agent.GetPosition(), 0), FVector(CircleCenter, 0.f), FColor::Red, false, -1.f, 0, 5.f);
	}

	Steering = Seek::CalculateSteering(DeltaT, Agent);

	return Steering;
}
