// Fill out your copyright notice in the Description page of Project Settings.


#include "Level_Flocking.h"


// Sets default values
ALevel_Flocking::ALevel_Flocking()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALevel_Flocking::BeginPlay()
{
	Super::BeginPlay();

	if (pAgentToEvade)
	{
		pWanderBehavior = std::make_unique<Wander>();
		pAgentToEvade->SetDebugRenderingEnabled(false);
		pAgentToEvade->SetSteeringBehavior(pWanderBehavior.get());
	}
	
	//TrimWorld->SetTrimWorldSize(1000.f);
	//TrimWorld->bShouldTrimWorld = true;

	pFlock = TUniquePtr<Flock>(
		new Flock(
			GetWorld(),
			SteeringAgentClass,
			FlockSize,
			TrimWorld->GetTrimWorldSize(),
			pAgentToEvade,
			true)
			);
}

void ALevel_Flocking::BeginDestroy()
{
	Super::BeginDestroy();

	if (pAgentToEvade)
		pAgentToEvade->Destroy();
}

// Called every frame
void ALevel_Flocking::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (pAgentToEvade != nullptr)
		pAgentToEvade->Tick(DeltaTime);

	pFlock->ImGuiRender(WindowPos, WindowSize);
	pFlock->Tick(DeltaTime);
	pFlock->RenderDebug();
	if (bUseMouseTarget)
	{
		pFlock->SetTarget_Seek(MouseTarget);
	}
}

