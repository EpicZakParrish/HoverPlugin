// Fill out your copyright notice in the Description page of Project Settings.

#include "HoverPluginPrivatePCH.h"
#include "SimpleCodeHoverComponent.h"
#include "DrawDebugHelpers.h"


USimpleCodeHoverComponent::USimpleCodeHoverComponent()
	: MaxHoverForce(10000.0f)
	, MaxHoverForceDistance(200.0f)
	, PrimitiveComponent(nullptr)
{
	// This constructor function is called when a hover component is instantiated.
	// Other than the auto-generated code below that enables the BeginPlay() and
	// Tick() functions to be called, it simply initializes the component's private
	// properties, such as MaxHoverForce with reasonable default values.

	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;
}


void USimpleCodeHoverComponent::BeginPlay()
{
	// This function is called when the game starts. It is equivalent to the
	// 'Event Begin Play' event in the Blueprint version of this component.

	Super::BeginPlay();

	// Get and store the primitive component that owns this hover component.
	// This line is equivalent to the blue sub-graph in the Blueprint component
	// that calls the GetOwner, GetRootComponent and CastToPrimitiveComponent
	// Blueprint functions respectively.

	PrimitiveComponent = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());

	// If we have a valid primitive component then we initialize its physics.
	// This code is equivalent to the red sub-graph in the Blueprint component.

	if (PrimitiveComponent != nullptr)
	{
		PrimitiveComponent->SetSimulatePhysics(true);
		PrimitiveComponent->SetLinearDamping(2.0f);
		PrimitiveComponent->SetAngularDamping(2.0f);
	}

	// Note that, unlike the Blueprint version of this component, which stored a
	// "HasPrimitive" flag in a Blueprint variable, we simply kept a pointer
	// to the primitive component itself. We therefore do not need to convert
	// the red sub-graph of the Blueprint component. When we wish to check
	// whether a valid primitive component exists, we simply check the pointer.
}


void USimpleCodeHoverComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	// This function is called every tick. It is equivalent to the "Event Tick"
	// event in the Blueprint version of this component.

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// If we don't have a valid primitive component then nothing needs to be done.
	// This is equivalent to the gray 'Primitive Check' block in the Blueprint.

	if (PrimitiveComponent == nullptr)
	{
		return;
	}

	// Let's replicate the BP code that performs the line trace each tick. First, we
	// have to calculate the start and end position for the trace. The SceneComponent
	// super class has a field called 'ComponentToWorld' which is a transformation
	// matrix that contains the component's location and orientation. This is equivalent
	// to the 'GetWorldLocation' call in the Blueprint. The trace end point is simply
	// underneath the component's location.

	const FVector Start = ComponentToWorld.GetLocation();
	const FVector End = FVector(Start.X, Start.Y, (Start.Z-MaxHoverForceDistance));

	// Now we are ready to perform the actual trace. The result of a trace operation
	// will be returned in a so called HitResult that contains information about if
	// and where the trace hit anything.

	FHitResult HitResult;

	// There are many ways to do traces in the Engine, such as using a line or a volume,
	// whether the trace should stop as soon as it hits something or return all hits,
	// and you can also specify what types of objects should be considered for collisions.
	// The following line is equivalent to the LineTraceByChannel call in the Blueprint.

	if (World->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldDynamic, FCollisionQueryParams(TEXT("HoverComponentTrace"))))
	{
		// We can now calculate the hover force that should be applied to the parent
		// component. The following lines are equivalent to the yellow BP sub-graph.

		const float Distance = (HitResult.ImpactPoint - Start).Size();
		const float Ratio = FMath::Clamp(Distance / MaxHoverForceDistance, 0.0f, 1.0f);
		const FVector SpringForce = (1.0f - Ratio) * MaxHoverForce * HitResult.Normal;

		// Finally, we apply the calculated force to the component.

		PrimitiveComponent->AddForce(SpringForce, NAME_None, false);

		// The BP node for the line trace has an option to draw a red debug line for the,
		// trace and a red point for the hit point. We can implement this in C++ using the
		// helper functions in DrawDebugHelpers.h

		::DrawDebugLine(World, Start, HitResult.Location, FColor::Red, false, 0.0f);
		::DrawDebugPoint(World, HitResult.ImpactPoint, 16.0f, FColor::Red, false, 0.0f);
	}
}
