#include "Level_CombinedSteering.h"

#include "imgui.h"


// Sets default values
ALevel_CombinedSteering::ALevel_CombinedSteering()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALevel_CombinedSteering::BeginPlay()
{
	Super::BeginPlay();

	m_pWander = new Wander();
	m_pEvade = new Evade();
	m_pSeek = new Seek();
	m_pPrioritySteering = new PrioritySteering({ m_pEvade, m_pWander });
	m_pBlendedSteering = new BlendedSteering({ {m_pSeek, 0.5f}, {m_pWander, 0.5f} });

	m_pDrunkAgent = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass, FVector(200, 200, 0), FRotator::ZeroRotator);
	m_pPriorityAgent = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass, FVector(400, 200, 0), FRotator::ZeroRotator);
	m_pDrunkAgent->SetSteeringBehavior(m_pBlendedSteering);
	m_pPriorityAgent->SetSteeringBehavior(m_pPrioritySteering);
}

void ALevel_CombinedSteering::BeginDestroy()
{
	Super::BeginDestroy();

	delete m_pWander;
	delete m_pEvade;
	delete m_pSeek;
	delete m_pPrioritySteering;
	delete m_pBlendedSteering;
}

void ALevel_CombinedSteering::UpdateTargets()
{
	FTargetData DrunkTarget;
	DrunkTarget.Position = m_pDrunkAgent->GetPosition();
	DrunkTarget.Orientation = m_pDrunkAgent->GetRotation();
	DrunkTarget.LinearVelocity = m_pDrunkAgent->GetLinearVelocity();
	DrunkTarget.AngularVelocity = m_pDrunkAgent->GetAngularVelocity();

	m_pPrioritySteering->SetTarget(DrunkTarget);

	FTargetData PriorityTarget;
	PriorityTarget.Position = m_pPriorityAgent->GetPosition();
	PriorityTarget.Orientation = m_pPriorityAgent->GetRotation();
	PriorityTarget.LinearVelocity = m_pPriorityAgent->GetLinearVelocity();
	PriorityTarget.AngularVelocity = m_pPriorityAgent->GetAngularVelocity();

	m_pBlendedSteering->SetTarget(PriorityTarget);
}

// Called every frame
void ALevel_CombinedSteering::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateTargets();
	
#pragma region UI
	//UI
	{
		//Setup
		bool windowActive = true;
		ImGui::SetNextWindowPos(WindowPos);
		ImGui::SetNextWindowSize(WindowSize);
		ImGui::Begin("Game AI", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	
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
		ImGui::Spacing();
	
		ImGui::Text("Flocking");
		ImGui::Spacing();
		ImGui::Spacing();
	
		if (ImGui::Checkbox("Debug Rendering", &CanDebugRender))
		{
   // TODO: Handle the debug rendering of your agents here :)
		}
		ImGui::Checkbox("Trim World", &TrimWorld->bShouldTrimWorld);
		if (TrimWorld->bShouldTrimWorld)
		{
			ImGuiHelpers::ImGuiSliderFloatWithSetter("Trim Size",
				TrimWorld->GetTrimWorldSize(), 1000.f, 3000.f,
				[this](float InVal) { TrimWorld->SetTrimWorldSize(InVal); });
		}
		
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
	
		ImGui::Text("Behavior Weights");
		ImGui::Spacing();


		ImGuiHelpers::ImGuiSliderFloatWithSetter("Seek",
			m_pBlendedSteering->GetWeightedBehaviorsRef()[0].Weight, 0.f, 1.f,
			[this](float InVal) { m_pBlendedSteering->GetWeightedBehaviorsRef()[0].Weight = InVal; }, "%.2f");
		
		ImGuiHelpers::ImGuiSliderFloatWithSetter("Wander",
		m_pBlendedSteering->GetWeightedBehaviorsRef()[1].Weight, 0.f, 1.f,
		[this](float InVal) { m_pBlendedSteering->GetWeightedBehaviorsRef()[1].Weight = InVal; }, "%.2f");
	
		//End
		ImGui::End();
	}
#pragma endregion
	

	// Combined Steering Update
 // TODO: implement handling mouse click input for seek
 // TODO: implement Make sure to also evade the wanderer
}
