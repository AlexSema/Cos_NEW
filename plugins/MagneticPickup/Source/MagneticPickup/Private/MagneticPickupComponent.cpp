// Developed by Charalampos Hadjiantoniou - www.chadjiantoniou.com - 2019

#include "MagneticPickupComponent.h"
#include "Components/SphereComponent.h"

DEFINE_LOG_CATEGORY(MagneticPickupComponent);

// Sets default values for this component's properties
UMagneticPickupComponent::UMagneticPickupComponent() {
	PreInitialize();
}

// Called when the game starts
void UMagneticPickupComponent::BeginPlay() {
	Super::BeginPlay();
	Initialize();
}

void UMagneticPickupComponent::PreInitialize() {
	PrimaryComponentTick.bCanEverTick = true;
	MagneticSphere = CreateDefaultSubobject<USphereComponent>(TEXT("MagnetSphere"));
	MagneticSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	//Get reference to owner
	Owner = GetOwner();

	MagneticSphere->OnComponentBeginOverlap.AddDynamic(this, &UMagneticPickupComponent::OnEnter);
	MagneticSphere->OnComponentEndOverlap.AddDynamic(this, &UMagneticPickupComponent::OnLeave);
}

void UMagneticPickupComponent::Initialize() {
	MagneticSphere->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	MagneticSphere->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	MagneticSphere->SetSphereRadius(MagneticSphereRadius, true);
}

// Called every frame
void UMagneticPickupComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bIsEnabled && bIsMagnetized && LockedActor && Owner) {
		Owner->GetRootComponent()->SetWorldLocation(FMath::VInterpTo(Owner->GetActorLocation(), LockedActor->GetActorLocation(), DeltaTime, Speed));
	}
}

void UMagneticPickupComponent::OnEnter(class UPrimitiveComponent* ThisComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) {
	if ((OtherActor != nullptr) && (OtherComp != nullptr) && !LockedActor && OtherActor != LockedActor) {
		auto OtherActorClass = OtherActor->GetClass();
		switch (TriggerMode) {
		case ETriggerMode::TM_ActorClasses: {
			TriggerActorClassesMagnetization(OtherActor);
		} break;
		case ETriggerMode::TM_ActorReferences: {
			TriggerActorReferencesMagnetization(OtherActor);
		}break;
		case ETriggerMode::TM_ActorClassesReferences: {
			if (bPrioritizeTriggerClass) {
				if (ContainsActorClass(OtherActorClass) || ContainsActorReference(OtherActor)) {
					TriggerMagnetization(OtherActor);
					UE_LOG(MagneticPickupComponent, Warning, TEXT("Locked Actor set to %s by ActorClassesReferences (bPrioritizeTriggerClass = true)"), *LockedActor->GetName());
				}
			}
			else {
				if (ContainsActorReference(OtherActor) || ContainsActorClass(OtherActorClass)) {
					TriggerMagnetization(OtherActor);
					UE_LOG(MagneticPickupComponent, Warning, TEXT("Locked Actor set to %s by ActorClassesReferences (bPrioritizeTriggerClass = false)"), *LockedActor->GetName());
				}
			}
		}break;
		case ETriggerMode::TM_ActorTags: {
			TriggerActorTagsMagnetization(OtherActor);
		}break;
		}
	}
}

void UMagneticPickupComponent::TriggerMagnetization(AActor* OtherActor) {
	LockedActor = OtherActor;
	bIsMagnetized = true;
	OnMagnetized.Broadcast(LockedActor);
}

void UMagneticPickupComponent::TriggerActorReferencesMagnetization(AActor* OtherActor) {
	//If the collided actor exists in the TriggerClasses array
	if (ContainsActorReference(OtherActor)) {
		TriggerMagnetization(OtherActor);
		UE_LOG(MagneticPickupComponent, Warning, TEXT("Locked Actor set to %s by ActorReferences"), *LockedActor->GetName());
	}
}

void UMagneticPickupComponent::TriggerActorClassesMagnetization(AActor* OtherActor) {
	//If the class of the collided actor exists in the TriggerClasses array
	auto OtherActorClass = OtherActor->GetClass();
	if (ContainsActorClass(OtherActorClass)) {
		TriggerMagnetization(OtherActor);
		UE_LOG(MagneticPickupComponent, Warning, TEXT("Locked Actor set to %s by ActorClasses"), *LockedActor->GetName());
	}
}

void UMagneticPickupComponent::TriggerActorTagsMagnetization(AActor* OtherActor) {
	//If the collided actor exists in the TriggerClasses array
	if (ContainsActorTag(OtherActor)) {
		TriggerMagnetization(OtherActor);
		UE_LOG(MagneticPickupComponent, Warning, TEXT("Locked Actor set to %s by ActorTags"), *LockedActor->GetName());
	}
}

void UMagneticPickupComponent::OnLeave(class UPrimitiveComponent* ThisComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	if (LockedActor != nullptr && LockedActor == OtherActor && bDemagnetizeOnLeave) {
		bIsMagnetized = false;
		UE_LOG(MagneticPickupComponent, Warning, TEXT("Locked Actor unset from %s"), *LockedActor->GetName());
		LockedActor = nullptr;
		OnDeMagnetized.Broadcast(OtherActor);
	}
}

void UMagneticPickupComponent::SetTriggerActors(TArray<AActor*> TriggerActorsIn) {
	TriggerActors = TriggerActorsIn;
}

void UMagneticPickupComponent::AddTriggerActor(AActor* TriggerActorIn) {
	TriggerActors.AddUnique(TriggerActorIn);
}

void UMagneticPickupComponent::RemoveTriggerActor(AActor* TriggerActorIn) {
	if (TriggerActors.Contains(TriggerActorIn)) {
		TriggerActors.Remove(TriggerActorIn);
	}
}

void UMagneticPickupComponent::AddTriggerClass(TSubclassOf<AActor> TriggerClassIn) {
	TriggerClasses.AddUnique(TriggerClassIn);
}

bool UMagneticPickupComponent::ContainsActorReference(AActor* ActorIn) const {
	return TriggerActors.Contains(ActorIn);
}

bool UMagneticPickupComponent::ContainsActorClass(TSubclassOf<AActor> ActorClassIn) const {
	return TriggerClasses.Contains(ActorClassIn);
}

bool UMagneticPickupComponent::ContainsActorTag(AActor* ActorClassIn) const {
	for (auto Tag : TriggerTags) {
		if (ActorClassIn->ActorHasTag(Tag))
			return true;
	}
	return false;
}