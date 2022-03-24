// Copyright 2020-2021 Fly Dream Dev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HAL/ThreadSafeBool.h"
#include "Runtime/Core/Public/HAL/Runnable.h"
#include "InsectsSystemActor.generated.h"

USTRUCT(BlueprintType)
struct FInsectsDamageDealActor
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY()
    AActor* actor = nullptr;

    UPROPERTY()
    float damage = 0.f;

    FInsectsDamageDealActor()
    {
    };
};

USTRUCT(BlueprintType)
struct FInsectsInteractActor
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY()
    AActor* interactActor = nullptr;

    UPROPERTY()
    float responseRadiusSquare = 0.f;

    FInsectsInteractActor()
    {
    };
};


USTRUCT(BlueprintType)
struct FInsectData
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY()
    int instanceIndex = 0;

    UPROPERTY()
    FVector velocity = FVector::ZeroVector;

    UPROPERTY()
    float insectSpeed = 0.f;

    UPROPERTY()
    float insectFollowSpeed = 0.f;

    UPROPERTY()
    float insectFleeSpeed = 0.f;

    UPROPERTY()
    FTransform transform;

    UPROPERTY()
    float elapsedTimeSinceLastWander = 0.0f;

    UPROPERTY()
    float timeFindFloor = 0.f;

    float walkTime = 0.f;
    float idleTime = 0.f;
    
    float canIdleTime = 0.f;
    float canWalkTime = 0.f;

    bool bInsectFollowSpeed = false;
    bool bInsectFleeSpeed = false;

    bool bIsWantIdle = false;
    bool bIsIdleNow = false;
    bool bIsIdleRotate = false;

    UPROPERTY()
    bool bIsDead = false;

    UPROPERTY()
    AActor* attackedActor = nullptr;

    FInsectData()
    {
    };
};

USTRUCT(BlueprintType)
struct FInsectsArrays
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY()
    int instanceIndex;

    UPROPERTY()
    TArray<FInsectData> swarmInsectsArr;

    FInsectsArrays()
    {
        instanceIndex = 0;
        swarmInsectsArr.Empty();
    };

    void AddData(int index, TArray<FInsectData> newInsectData)
    {
        instanceIndex = index;
        swarmInsectsArr = newInsectData;
    };
};

USTRUCT(BlueprintType)
struct FInsectsAudio
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY()
    int instanceIndex = 0;

    UPROPERTY()
    float insectDistance = 0.f;

    FInsectsAudio()
    {
    };
};

UENUM(BlueprintType)
enum class EPriorityOfThread: uint8
{
    Normal                  UMETA(DisplayName = "Normal"),
    AboveNormal             UMETA(DisplayName = "AboveNormal"),
    BelowNormal             UMETA(DisplayName = "BelowNormal"),
    Highest                 UMETA(DisplayName = "Highest"),
    Lowest                  UMETA(DisplayName = "Lowest"),
    SlightlyBelowNormal     UMETA(DisplayName = "SlightlyBelowNormal"),
    TimeCritical            UMETA(DisplayName = "TimeCritical")
};

USTRUCT(BlueprintType)
struct FInsectParameters
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Advanced Insects Spawn")
    EPriorityOfThread threadPriority = EPriorityOfThread::Normal;
    // Use for only more insects mates!
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Advanced Insects Spawn")
    bool ExperimentalOptimization = false;
    // If need react on pawn.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Flock Parameters")
    bool bReactOnPawn = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Fly Insects Parameters")
    bool bFollowToPawn = false;

    // Can attack pawn.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Fly Insects Parameters")
    bool bCanAttackPawn = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Fly Insects Parameters")
    float damageValue = 5.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Fly Insects Parameters")
    TSubclassOf<class UDamageType> damageType;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Advanced Insects Parameters")
    float insectWalkSpeed = 150.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Advanced Insects Parameters")
    float insectOffsetWalkSpeed = 20.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Advanced Insects Parameters")
    float insectFollowSpeed = 300.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Advanced Insects Parameters")
    float insectOffsetFollowSpeed = 50.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Advanced Insects Parameters")
    float insectFleeSpeed = 300.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Advanced Insects Parameters")
    float insectOffsetFleeSpeed = 50.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Advanced Insect Spawn")
    float insectsWanderInRandomRadius = 5000.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Advanced Insects Parameters")
    float enemyAwarenessRadius = 200.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Advanced Insects Parameters")
    float targetAwarenessRadius = 300.0f;

    // Walk parameters.
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Advanced Insects Parameters")
    bool bIsCanInsectIdle = true;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Advanced Insects Parameters")
    float minWalkTime = 3.f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Advanced Insects Parameters")
    float maxWalkTime = 15.f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Advanced Insects Parameters")
    float minIdleTime = 1.f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Advanced Insects Parameters")
    float maxIdleTime = 4.f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Advanced Insects Parameters")
    int maxPursuers = 100;

    float insectsWanderInRandomRadiusSquare = 0.f;

    FInsectParameters()
    {
    };
};

UCLASS()
class ADVANCEDINSECTSSYSTEM_API AInsectsSystemActor : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AInsectsSystemActor();

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    //************************************************************************
    // Component                                                                  
    //************************************************************************
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Component")
    class UInstancedStaticMeshComponent* StaticMeshInstanceComponent;
   
    //************************************************************************

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Advanced Insects Spawn")
    UStaticMesh* StaticMesh;
    UPROPERTY(BlueprintReadOnly, Category = "Advanced Insects Parameters")
    TArray<FInsectData> swarmInsectsData;
    // Recommended - 1 Thread = (2000 - 2500) mates. 
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Advanced Insects Spawn", meta=(ClampMin="1", ClampMax="32"))
    int maxUseThreads = 1;
    // Recommended - 1 Thread = (2000 - 2500) mates.
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Advanced Insects Spawn")
    int32 insectInstances = 1000;
    // Random mesh scale.
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Advanced Insects Spawn")
    float minMeshScale = 1.f;
    // Random mesh scale.
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Advanced Insects Spawn")
    float maxMeshScale = 2.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Insects Parameters")
    TArray<AActor*> enemyActorsArr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Insects Parameters")
    TArray<AActor*> targetActorsArr;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Advanced Insects Parameters")
    FInsectParameters insectParameters;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Advanced Insects Spawn", meta=(ClampMin="0", ClampMax="2"))
    int maxSounds = 2;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Advanced Insects Spawn")
    USoundBase* insectSound = nullptr;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Advanced Insects Spawn")
    bool bEnableSound = true;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Advanced Insects Spawn")
    bool bIsDebugSound = false;

    float interpMoveAnimRate = 50000.f;
    float interpRotateAnimRate = 50000.f;

    TArray<class InsectsThread*> swarmInsectSystemPoolThreadArr;

    UFUNCTION(BlueprintCallable, Category = "Insects System")
    void AddEnemyActor_BP(AActor* setActor, float setResponseDistance);

    UFUNCTION(BlueprintCallable, Category = "Insects System")
    void DeleteEnemyActor_BP(AActor* setActor);

    UFUNCTION(BlueprintCallable, Category = "Insects System")
    void AddTargetActor_BP(AActor* setActor, float setResponseDistance);

    UFUNCTION(BlueprintCallable, Category = "Insects System")
    void DeleteTargetActor_BP(AActor* setActor);
    
    UFUNCTION(BlueprintCallable, Category = "Insects System")
    TArray<FTransform> ApplyDamage_BP(FVector setDamagePoint, float setDamageRadius);

    UFUNCTION(BlueprintCallable, Category = "Insects System")
    void SetPause_BP(bool bSetPause, bool bClearInstance);

    UFUNCTION(BlueprintPure, Category = "Insects System")
    bool GetIsInsectsOnPause_BP();

    UFUNCTION(BlueprintCallable, Category = "Insects System")
    void SetActivateInsects(bool setActivate);

    UFUNCTION(BlueprintPure, Category = "Insects System")
    bool GetIsInsectsActivated();
    

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    virtual void BeginDestroy() override;

    // Add an instance to this component. Transform is given in world space. 
    void AddInsectToWorld(const FTransform &WorldTransform, int &setIndex);

    TArray<FInsectsArrays> allInsectsArrays;

    TArray<class UAudioComponent*> allAudioComponentsArr;

    

private:

    void DivideInsectsArrayForThreads();

    void GenerateSwarmInsectsTread();

    void InsectWorkerTimer();

    void DamageDealTimer();

    FHitResult SphereTrace(FVector startV, FVector &endV, float sphereRadius);

    bool bIsInsectsActivated = true;

    FTimerHandle insectsWorkerTimer_Handle;
    float workerRate = 0.01f;

    bool bIsPaused = false;

    TArray<FInsectsInteractActor> enemyInteractActorsArr;
    TArray<FInsectsInteractActor> targetInteractActorsArr;

    TArray<FInsectsDamageDealActor> insectsDamageDealActorArr;

    FTimerHandle insectsDamageDealTimer_Handle;
    float damageDealRate = 0.25f;

    
};

// Thread
class InsectsThread : public FRunnable
{
public:

    //================================= THREAD =====================================
    
    //Constructor
    InsectsThread(AActor *newActor);
    //Destructor
    ~InsectsThread();

    //Use this method to kill the thread!!
    void EnsureCompletion();
    //Pause the thread 
    void PauseThread();
    //Continue/UnPause the thread
    void ContinueThread();
    //FRunnable interface.
    bool Init();
    uint32 Run();
    void Stop();
    bool IsThreadPaused();
    
    void SetSpawnActorLocation(FVector &newPoint);

    //================================= INSECTS =====================================
    class UBoxComponent* BoxComponentRef;

    TArray<FInsectData> GetInsectsData(TArray<FInsectsInteractActor> setEnemyActorsArr, TArray<FInsectsInteractActor> setTargetActorsArr);

    void InitInsectsParameters(TArray<FInsectData> setInsectsArr, FInsectParameters newParameters);

    TArray<FInsectData> swarmInsectInThreadArr;
    FInsectParameters swarmInsectsParameters;
    TArray<FInsectsInteractActor> enemyActorsArrTHR;
    TArray<FInsectsInteractActor> targetActorsArrTHR;
 
    float threadDeltaTime = 0.f;
    //================================= INSECTS =====================================


    void SetPoolThread(TArray<class InsectsThread*> setPoolThreadArr);

    AActor *ownerActor;

    float groundCheckTraceDistance = 10.f;

    FVector spawnActorLocation = FVector::ZeroVector;
    

private:

    //Thread to run the worker FRunnable on
    FRunnableThread* Thread;

    FCriticalSection m_mutex;
    FEvent* m_semaphore;

    int m_chunkCount;
    int m_amount;
    int m_MinInt;
    int m_MaxInt;

    float threadSleepTime = 0.01f;

    //As the name states those members are Thread safe
    FThreadSafeBool m_Kill;
    FThreadSafeBool m_Pause;

    TArray<class InsectsThread*> PoolThreadArr;

    FHitResult SphereTrace(FVector startV, FVector &endV, float sphereRadius);

    FRotator RotationFromNormal(const FVector& insectForward, const FVector& HitNormal);

    void AvoidVolumeActor(FHitResult setHitResult, FInsectData& insectMember, FHitResult floorHitRes);

    bool IsNeedAvoid(FHitResult setHitResult);

    void IsPointInAvoidBoxSetOriginLocation(FInsectData& insectMember, class AInsectsAvoidVolume* avoidVolume);
};

