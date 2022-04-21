// Developed by Charalampos Hadjiantoniou - www.chadjiantoniou.com - 2019

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MagneticPickupComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(MagneticPickupComponent, Log, All);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMagnetized, const class AActor*, LockedActor);

UENUM(BlueprintType)
enum class ETriggerMode : uint8 {
	TM_ActorClasses UMETA(DisplayName = "Actor Classes"),
	TM_ActorReferences UMETA(DisplayName = "Actor References"),
	TM_ActorClassesReferences UMETA(DisplayName = "Actor Classes & References"),
	TM_ActorTags UMETA(DisplayName = "Actor Tags"),
};

UCLASS( ClassGroup=(CHadjiantoniou), meta=(BlueprintSpawnableComponent) )
class MAGNETICPICKUP_API UMagneticPickupComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMagneticPickupComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(Category = "Magnetic Pickup Component")
	void OnEnter(class UPrimitiveComponent* ThisComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION(Category = "Magnetic Pickup Component")
	void OnLeave(class UPrimitiveComponent* ThisComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/* Used to dynamically set trigger actors in case the actors are created at run-time*/
	UFUNCTION(Category = "Magnetic Pickup Component")
	void SetTriggerActors(TArray<AActor*> TriggerActorsIn);

	/* Used to dynamically add trigger actors in case the actors are created at run-time*/
	UFUNCTION(Category = "Magnetic Pickup Component")
	void AddTriggerActor(AActor* TriggerActorIn);

	/* Used to dynamically remove trigger actors, in case the actors are created at run-time */
	UFUNCTION(Category = "Magnetic Pickup Component")
	void RemoveTriggerActor(AActor* TriggerActorIn);

	/* Used to dynamically add trigger classes*/
	UFUNCTION(Category = "Magnetic Pickup Component")
	void AddTriggerClass(TSubclassOf<AActor> TriggerClassIn);

	/*Sets owner, creates collision sphere and hooks up the callbacks for the magnetic component*/
	UFUNCTION(BlueprintCallable, Category = "Magnetic Pickup Component")
	void PreInitialize();

	/* Attaches component to root, sets magnetic sphere's radius and relative location*/
	UFUNCTION(BlueprintCallable, Category = "Magnetic Pickup Component")
	void Initialize();

	/* Whether the magnetic functionality is enabled or not*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magnetic Pickup Configuration")
	bool bIsEnabled = true;

	UPROPERTY(BlueprintAssignable, Category = "Magnetic Pickup Component")
	FOnMagnetized OnMagnetized;

	UPROPERTY(BlueprintAssignable, Category = "Magnetic Pickup Component")
	FOnMagnetized OnDeMagnetized;

	UPROPERTY()
	class USphereComponent* MagneticSphere = nullptr;

	/* Whether the component should check for specific actors to magnetize to, or actor classes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magnetic Pickup Configuration", meta = (EditCondition = "bIsEnabled"))
	ETriggerMode TriggerMode = ETriggerMode::TM_ActorClasses;
	
	/* Array of actor references that trigger magnetization. Used in the case of TriggerMode = ActorReferences.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magnetic Pickup Configuration", meta = (EditCondition = "bIsEnabled"))
	TArray<AActor*> TriggerActors;

	/* Array of actor classes that trigger magnetization. Used in the case of TriggerMode = ActorClasses.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magnetic Pickup Configuration", meta = (EditCondition = "bIsEnabled"))
	TArray<TSubclassOf<AActor>> TriggerClasses;

	/* Array of actor tags that trigger magnetization. Used in the case of TriggerMode = ActorTags.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magnetic Pickup Configuration", meta = (EditCondition = "bIsEnabled"))
	TArray<FName> TriggerTags;

	/* In the case of TriggerMode = TM_ActorClassesReferences, if set to true checks inside classes array for existence of collided actor first. By default it checks for actors references first*/
	UPROPERTY(EditAnywhere, Category = "Magnetic Pickup Configuration", meta = (EditCondition = "bIsEnabled"))
	bool bPrioritizeTriggerClass = false;

	/*If it is already magnetized, should it demagnetize if the LockedActor leaves the MagnetSphere*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magnetic Pickup Configuration", meta = (EditCondition = "bIsEnabled"))
	bool bDemagnetizeOnLeave = true;

	/*Interpolation speed in which the item will travel to the target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magnetic Pickup Configuration", meta = (EditCondition = "bIsEnabled"))
	float Speed = 2;

	/*Interpolation speed multiplier s*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magnetic Pickup Configuration", meta = (EditCondition = "bIsEnabled"))
	float SpeedMultiplier = 1;

	/* The radius of the magnetic sphere. Once configured actors/references step into the radius of the sphere, the magnetic functionality will be activated.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magnetic Pickup Configuration", meta = (EditCondition = "bIsEnabled"))
	float MagneticSphereRadius = 0.f;

protected:
	virtual void BeginPlay() override;

private:
	AActor* LockedActor = nullptr;
	bool bIsMagnetized = false;
	AActor* Owner = nullptr;
	bool ContainsActorReference(AActor* ActorIn) const;
	bool ContainsActorClass(TSubclassOf<AActor> ActorClassIn) const;
	bool ContainsActorTag(AActor* ActorClassIn) const;
	void TriggerMagnetization(AActor* OtherActor);
	void TriggerActorReferencesMagnetization(AActor* OtherActor);
	void TriggerActorClassesMagnetization(AActor* OtherActor);
	void TriggerActorTagsMagnetization(AActor* OtherActor);
};