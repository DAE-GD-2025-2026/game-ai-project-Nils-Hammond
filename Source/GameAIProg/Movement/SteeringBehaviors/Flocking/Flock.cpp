#include "Flock.h"
#include "FlockingSteeringBehaviors.h"
#include "Shared/ImGuiHelpers.h"


Flock::Flock(
	UWorld* pWorld,
	TSubclassOf<ASteeringAgent> AgentClass,
	int FlockSize,
	float WorldSize,
	ASteeringAgent* const pAgentToEvade,
	bool bTrimWorld)
	: pWorld{pWorld}
	, FlockSize{ FlockSize }
	, pAgentToEvade{pAgentToEvade}
{
	Agents.SetNum(FlockSize);
	pSeparationBehaviors.SetNum(FlockSize);
	pCohesionBehaviors.SetNum(FlockSize);
	pVelMatchBehaviors.SetNum(FlockSize);
	pSeekBehaviors.SetNum(FlockSize);
	pWanderBehaviors.SetNum(FlockSize);
	pBlendedSteerings.SetNum(FlockSize);

	for (int i = 0; i < FlockSize; ++i)
	{
		while (Agents[i] == nullptr)
		{
			FVector SpawnLocation = FVector(FMath::FRandRange(0, WorldSize), FMath::FRandRange(0, WorldSize), 90.f);
			Agents[i] = pWorld->SpawnActor<ASteeringAgent>(AgentClass, SpawnLocation, FRotator::ZeroRotator);
		}
		pSeparationBehaviors[i] = std::make_unique<Separation>(this);
		pCohesionBehaviors[i] = std::make_unique<Cohesion>(this);
		pVelMatchBehaviors[i] = std::make_unique<VelocityMatch>(this);
		pSeekBehaviors[i] = std::make_unique<Seek>();
		pWanderBehaviors[i] = std::make_unique<Wander>();

		pBlendedSteerings[i] = std::make_unique<BlendedSteering>(std::vector<BlendedSteering::WeightedBehavior>
		{
			{pSeparationBehaviors[i].get(), 0.5f},
			{ pCohesionBehaviors[i].get(), 0.5f},
			{ pVelMatchBehaviors[i].get(), 0.5f},
			{ pSeekBehaviors[i].get(), 0.5f},
			{ pWanderBehaviors[i].get(), 0.5f}
		});


		Agents[i]->SetSteeringBehavior(pBlendedSteerings[i].get());
		Agents[i]->SetDebugRenderingEnabled(false);
	}

	Neighbors.SetNum(10);
 // TODO: initialize the flock and the memory pool
}

Flock::~Flock()
{
 // TODO: Cleanup any additional data
	for (ASteeringAgent* const pAgent : Agents)
	{
		pAgent->Destroy();
	}
}

void Flock::Tick(float DeltaTime)
{
	for (ASteeringAgent* const pAgent : Agents)
	{
		NrOfNeighbors = 0;
		RegisterNeighbors(pAgent);
		pAgent->Tick(DeltaTime);
	}
 // TODO: update the flock
 // TODO: for every agent:
  // TODO: register the neighbors for this agent (-> fill the memory pool with the neighbors for the currently evaluated agent)
  // TODO: update the agent (-> the steeringbehaviors use the neighbors in the memory pool)
  // TODO: trim the agent to the world
}

void Flock::RenderDebug()
{
 // TODO: Render all the agents in the flock
}

void Flock::ImGuiRender(ImVec2 const& WindowPos, ImVec2 const& WindowSize)
{
#ifdef PLATFORM_WINDOWS
#pragma region UI
	//UI
	{
		//Setup
		bool bWindowActive = true;
		ImGui::SetNextWindowPos(WindowPos);
		ImGui::SetNextWindowSize(WindowSize);
		ImGui::Begin("Gameplay Programming", &bWindowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Text("LMB: place target");
		ImGui::Text("RMB: move cam.");
		ImGui::Text("Scrollwheel: zoom cam.");
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Text("Flocking");
		ImGui::Spacing();

  // TODO: implement ImGUI checkboxes for debug rendering here

		ImGui::Text("Behavior Weights");
		ImGui::Spacing();

  // TODO: implement ImGUI sliders for steering behavior weights here
  		
		ImGuiHelpers::ImGuiSliderFloatWithSetter("Separation",
			pBlendedSteerings[0]->GetWeightedBehaviorsRef()[0].Weight, 0.f, 1.f,
			[this](float InVal)
			{
				for (int i = 0; i < pBlendedSteerings.Num(); ++i)
				{
					pBlendedSteerings[i]->GetWeightedBehaviorsRef()[0].Weight = InVal;
				}
			},
			"%.2f");

		ImGuiHelpers::ImGuiSliderFloatWithSetter("Cohesion",
			pBlendedSteerings[0]->GetWeightedBehaviorsRef()[1].Weight, 0.f, 1.f,
			[this](float InVal)
			{
				for (int i = 0; i < pBlendedSteerings.Num(); ++i)
				{
					pBlendedSteerings[i]->GetWeightedBehaviorsRef()[1].Weight = InVal;
				}
			},
			"%.2f");

		ImGuiHelpers::ImGuiSliderFloatWithSetter("Velocity Match",
			pBlendedSteerings[0]->GetWeightedBehaviorsRef()[2].Weight, 0.f, 1.f,
			[this](float InVal)
			{
				for (int i = 0; i < pBlendedSteerings.Num(); ++i)
				{
					pBlendedSteerings[i]->GetWeightedBehaviorsRef()[2].Weight = InVal;
				}
			},
			"%.2f");

		ImGuiHelpers::ImGuiSliderFloatWithSetter("Seek",
			pBlendedSteerings[0]->GetWeightedBehaviorsRef()[3].Weight, 0.f, 1.f,
			[this](float InVal)
			{
				for (int i = 0; i < pBlendedSteerings.Num(); ++i)
				{
					pBlendedSteerings[i]->GetWeightedBehaviorsRef()[3].Weight = InVal;
				}
			},
			"%.2f");

		ImGuiHelpers::ImGuiSliderFloatWithSetter("Wander",
			pBlendedSteerings[0]->GetWeightedBehaviorsRef()[4].Weight, 0.f, 1.f,
			[this](float InVal)
			{
				for (int i = 0; i < pBlendedSteerings.Num(); ++i)
				{
					pBlendedSteerings[i]->GetWeightedBehaviorsRef()[4].Weight = InVal;
				}
			},
			"%.2f");
		
		//ImGuiHelpers::ImGuiSliderFloatWithSetter("Seek",
		//	pBlendedSteering->GetWeightedBehaviorsRef()[0].Weight, 0.f, 1.f,
		//	[this](float InVal) { pBlendedSteering->GetWeightedBehaviorsRef()[0].Weight = InVal; }, "%.2f");
		//
		//ImGuiHelpers::ImGuiSliderFloatWithSetter("Wander",
		//	pBlendedSteering->GetWeightedBehaviorsRef()[1].Weight, 0.f, 1.f,
		//	[this](float InVal) { pBlendedSteering->GetWeightedBehaviorsRef()[1].Weight = InVal; }, "%.2f");
		//
		//ImGuiHelpers::ImGuiSliderFloatWithSetter("Separation",
		//	pBlendedSteering->GetWeightedBehaviorsRef()[2].Weight, 0.f, 1.f,
		//	[this](float InVal) { pBlendedSteering->GetWeightedBehaviorsRef()[2].Weight = InVal; }, "%.2f");
		//
		//ImGuiHelpers::ImGuiSliderFloatWithSetter("Cohesion",
		//	pBlendedSteering->GetWeightedBehaviorsRef()[3].Weight, 0.f, 1.f,
		//	[this](float InVal) { pBlendedSteering->GetWeightedBehaviorsRef()[3].Weight = InVal; }, "%.2f");
		//
		//ImGuiHelpers::ImGuiSliderFloatWithSetter("Velocity Match",
		//	pBlendedSteering->GetWeightedBehaviorsRef()[4].Weight, 0.f, 1.f,
		//	[this](float InVal) { pBlendedSteering->GetWeightedBehaviorsRef()[4].Weight = InVal; }, "%.2f");

		//End
		ImGui::End();
	}
#pragma endregion
#endif
}

void Flock::RenderNeighborhood()
{
 // TODO: Debugrender the neighbors for the first agent in the flock
}

#ifndef GAMEAI_USE_SPACE_PARTITIONING
void Flock::RegisterNeighbors(ASteeringAgent* const pAgent)
{
 // TODO: Implement
	for (ASteeringAgent* const pOtherAgent : Agents)
	{
		if (pOtherAgent != pAgent)
		{
			const float SqrDist = FVector2D::DistSquared(pAgent->GetPosition(), pOtherAgent->GetPosition());
			if (SqrDist < NeighborhoodRadius * NeighborhoodRadius && NrOfNeighbors < Neighbors.Num())
			{
				Neighbors[NrOfNeighbors] = pOtherAgent;
				++NrOfNeighbors;
			}
		}
	}
}
#endif

FVector2D Flock::GetAverageNeighborPos() const
{
	FVector2D avgPosition = FVector2D::ZeroVector;

 // TODO: Implement
	for (int neighborIdx = 0; neighborIdx < NrOfNeighbors; ++neighborIdx)
	{
		avgPosition += Neighbors[neighborIdx]->GetPosition();
	}
	avgPosition /= NrOfNeighbors;
	
	return avgPosition;
}

FVector2D Flock::GetAverageNeighborVelocity() const
{
	FVector2D avgVelocity = FVector2D::ZeroVector;

 // TODO: Implement
	for (int neighborIdx = 0; neighborIdx < NrOfNeighbors; ++neighborIdx)
	{
		avgVelocity += Neighbors[neighborIdx]->GetLinearVelocity();
	}
	avgVelocity /= NrOfNeighbors;

	return avgVelocity;
}

void Flock::SetTarget_Seek(FSteeringParams const& Target)
{
	for (int i = 0; i < pBlendedSteerings.Num(); ++i)
	{
		pBlendedSteerings[i]->SetTarget(Target);
	}
}