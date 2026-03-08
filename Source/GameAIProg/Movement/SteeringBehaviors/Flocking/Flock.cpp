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

#ifdef GAMEAI_USE_SPACE_PARTITIONING
	OldPositions.SetNum(FlockSize);

	pPartitionedSpace = std::make_unique<CellSpace>(pWorld, WorldSize * 2, WorldSize * 2, NrOfCellsX, NrOfCellsX, FlockSize);
#else
	Neighbors.SetNum(10);
#endif

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
			{pSeparationBehaviors[i].get(), 0.2f},
			{ pCohesionBehaviors[i].get(), 0.2f},
			{ pVelMatchBehaviors[i].get(), 0.2f},
			{ pSeekBehaviors[i].get(), 0.2f},
			{ pWanderBehaviors[i].get(), 0.2f}
		});


		Agents[i]->SetSteeringBehavior(pBlendedSteerings[i].get());
		Agents[i]->SetDebugRenderingEnabled(false);
#ifdef GAMEAI_USE_SPACE_PARTITIONING
		pPartitionedSpace->AddAgent(*Agents[i]);
		OldPositions[i] = Agents[i]->GetPosition();
#endif
	}

}

Flock::~Flock()
{
 // TODO: Cleanup any additional data
	pPartitionedSpace->EmptyCells();
	for (ASteeringAgent* const pAgent : Agents)
	{
		pAgent->Destroy();
	}
}

void Flock::Tick(float DeltaTime)
{
	for (int i = 0; i < Agents.Num(); ++i)
	{
#ifdef GAMEAI_USE_SPACE_PARTITIONING
		pPartitionedSpace->UpdateAgentCell(*Agents[i], OldPositions[i]);
		OldPositions[i] = Agents[i]->GetPosition();
#else
		NrOfNeighbors = 0;
		RegisterNeighbors(pAgent);
#endif
		Agents[i]->Tick(DeltaTime);
	}
 // TODO: update the flock
 // TODO: for every agent:
  // TODO: register the neighbors for this agent (-> fill the memory pool with the neighbors for the currently evaluated agent)
  // TODO: update the agent (-> the steeringbehaviors use the neighbors in the memory pool)
  // TODO: trim the agent to the world
}

void Flock::RenderDebug()
{
#ifdef GAMEAI_USE_SPACE_PARTITIONING
	pPartitionedSpace->RenderCells(bUsePartitioning);
#endif
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

		// Button to change between partitioning or not
		ImGui::Checkbox("Use Spacial Partitioning", &bUsePartitioning);

  // TODO: implement ImGUI checkboxes for debug rendering here


		ImGui::Text("Behavior Weights");
		ImGui::Spacing();
  		
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
	TArray<ASteeringAgent*> Neighbors = GetNeighbors();

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
	TArray<ASteeringAgent*> Neighbors = GetNeighbors();

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