// Copyright 2020-2021 Fly Dream Dev. All Rights Reserved.


#include "InsectsSystemActor.h"


#include "DrawDebugHelpers.h"
#include "InsectsAvoidVolume.h"
#include "Runtime/Core/Public/HAL/RunnableThread.h"
#include "TimerManager.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

#include "Kismet/KismetMathLibrary.h"

// Sets default values
AInsectsSystemActor::AInsectsSystemActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMeshInstanceComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("StaticMeshInstanceComponent"));
	StaticMeshInstanceComponent->SetupAttachment(GetRootComponent());
	StaticMeshInstanceComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StaticMeshInstanceComponent->SetGenerateOverlapEvents(false);
}

void AInsectsSystemActor::AddEnemyActor_BP(AActor* setActor, float setResponseDistance)
{
	if (setActor)
	{
		bool bFoundEqualActor_(false);
		for (int i = 0; i < enemyInteractActorsArr.Num(); ++i)
		{
			if (enemyInteractActorsArr[i].interactActor == setActor)
			{
				bFoundEqualActor_ = true;
			}
		}
		
		if (!bFoundEqualActor_)
		{
			FInsectsInteractActor newInteractActor;
			newInteractActor.interactActor = setActor;
			newInteractActor.responseRadiusSquare = FMath::Square(setResponseDistance);
			enemyInteractActorsArr.Add(newInteractActor);
		}
		
	}
}

void AInsectsSystemActor::DeleteEnemyActor_BP(AActor* setActor)
{
	if (setActor)
	{
		for (int i = 0; i < enemyInteractActorsArr.Num(); ++i)
		{
			if (enemyInteractActorsArr[i].interactActor == setActor)
			{
				enemyInteractActorsArr.RemoveAt(i);
				break;
			}
		}
		if (enemyActorsArr.Contains(setActor))
		{
			enemyActorsArr.Remove(setActor);
		}
	}
}

void AInsectsSystemActor::AddTargetActor_BP(AActor* setActor, float setResponseDistance)
{
	if (setActor)
	{
		bool bFoundEqualActor_(false);
		for (int i = 0; i < targetInteractActorsArr.Num(); ++i)
		{
			if (targetInteractActorsArr[i].interactActor == setActor)
			{
				bFoundEqualActor_ = true;
			}
		}

		if (!bFoundEqualActor_)
		{
			FInsectsInteractActor newInteractActor;
			newInteractActor.interactActor = setActor;
			newInteractActor.responseRadiusSquare = FMath::Square(setResponseDistance);
			targetInteractActorsArr.Add(newInteractActor);
		}
	}
}

void AInsectsSystemActor::DeleteTargetActor_BP(AActor* setActor)
{
	if (setActor)
	{
		for (int i = 0; i < targetInteractActorsArr.Num(); ++i)
		{
			if (targetInteractActorsArr[i].interactActor == setActor)
			{
				targetInteractActorsArr.RemoveAt(i);
				break;
			}
		}
		if (targetActorsArr.Contains(setActor))
		{
			targetActorsArr.Remove(setActor);
		}
	}
}

TArray<FTransform> AInsectsSystemActor::ApplyDamage_BP(FVector setDamagePoint, float setDamageRadius)
{
	TArray<FTransform> deadVectorArr_;

	if (GetIsInsectsOnPause_BP())
	{
		return deadVectorArr_;
	}
	
	for (int i = 0; i < swarmInsectsData.Num(); ++i)
	{
		if (!swarmInsectsData[i].bIsDead)
		{
			if ((swarmInsectsData[i].transform.GetLocation() - setDamagePoint).Size() <= setDamageRadius)
			{
				deadVectorArr_.Add(swarmInsectsData[i].transform);
				swarmInsectsData[i].transform.SetScale3D(FVector::ZeroVector);
				StaticMeshInstanceComponent->UpdateInstanceTransform(swarmInsectsData[i].instanceIndex, swarmInsectsData[i].transform, true, false);
				swarmInsectsData[i].bIsDead = true;
			}
		}
	}
	StaticMeshInstanceComponent->MarkRenderStateDirty();
	return deadVectorArr_;
}

void AInsectsSystemActor::SetPause_BP(bool bSetPause, bool bClearInstance)
{
	bIsPaused = bSetPause;

	for (int i = 0; i < swarmInsectSystemPoolThreadArr.Num(); ++i)
	{
		if (swarmInsectSystemPoolThreadArr[i])
		{
			if (bIsPaused)
			{
				if (!swarmInsectSystemPoolThreadArr[i]->IsThreadPaused())
				{
					swarmInsectSystemPoolThreadArr[i]->PauseThread();
				}
			}
			else
			{
				if (swarmInsectSystemPoolThreadArr[i]->IsThreadPaused())
				{
					swarmInsectSystemPoolThreadArr[i]->ContinueThread();
				}
			}
		}
	}

	if (maxSounds > 0 && insectSound != nullptr && bEnableSound)
	{
		for (int i = 0; i < allAudioComponentsArr.Num(); ++i)
		{
			if (bIsPaused)
			{
				allAudioComponentsArr[i]->Stop();
			}
			else
			{
				allAudioComponentsArr[i]->Play();
			}
		}
	}

	if (bIsPaused)
	{
		if (bClearInstance)
		{
			StaticMeshInstanceComponent->ClearInstances();
		}
	}
	else
	{
		if (StaticMeshInstanceComponent->GetInstanceCount() == 0)
		{
			for (int i = 0; i < swarmInsectsData.Num(); ++i)
			{
				StaticMeshInstanceComponent->AddInstanceWorldSpace(swarmInsectsData[i].transform);
			}
		}
	}
}

bool AInsectsSystemActor::GetIsInsectsOnPause_BP()
{
	return bIsPaused;
}

void AInsectsSystemActor::SetActivateInsects(bool setActivate)
{
	bIsInsectsActivated = setActivate;

	for (int i = 0; i < swarmInsectSystemPoolThreadArr.Num(); ++i)
	{
		if (setActivate && swarmInsectSystemPoolThreadArr[i]->IsThreadPaused())
		{
			swarmInsectSystemPoolThreadArr[i]->ContinueThread();
		}
		else if (!setActivate)
		{
			swarmInsectSystemPoolThreadArr[i]->PauseThread();
		}
	}
}

bool AInsectsSystemActor::GetIsInsectsActivated()
{
	return bIsInsectsActivated;
}

// Called when the game starts or when spawned
void AInsectsSystemActor::BeginPlay()
{
	Super::BeginPlay();

	StaticMeshInstanceComponent->SetStaticMesh(StaticMesh);

	StaticMeshInstanceComponent->SetWorldScale3D(FVector(1.f, 1.f, 1.f));

	for (int i = 0; i < enemyActorsArr.Num(); ++i)
	{
		FInsectsInteractActor newActor_;
		newActor_.interactActor = enemyActorsArr[i];
		newActor_.responseRadiusSquare = FMath::Square(insectParameters.enemyAwarenessRadius);
		enemyInteractActorsArr.Add(newActor_);
	}
	for (int i = 0; i < targetActorsArr.Num(); ++i)
	{
		FInsectsInteractActor newActor_;
		newActor_.interactActor = targetActorsArr[i];
		newActor_.responseRadiusSquare = FMath::Square(insectParameters.targetAwarenessRadius);
		targetInteractActorsArr.Add(newActor_);
		
	}

	insectParameters.insectsWanderInRandomRadiusSquare = FMath::Square(insectParameters.insectsWanderInRandomRadius);

	if (insectParameters.ExperimentalOptimization)
	{
		StaticMeshInstanceComponent->CastShadow = false;
		bEnableSound = false;
		PrimaryActorTick.bCanEverTick = false;
	}

	FVector traceStart_ = GetActorLocation();
	FVector traceEnd_ = traceStart_ + GetRootComponent()->GetUpVector() * -500.f;
	FHitResult hitResult_(SphereTrace(traceStart_, traceEnd_, 2.f));

	for (int i = 0; i < insectInstances; ++i)
	{
		FRotator const rotation_(FRotator(0.f, UKismetMathLibrary::RandomRotator().Yaw, 0.f));
		float const randScale_(FMath::RandRange(minMeshScale, maxMeshScale));

		if (hitResult_.bBlockingHit)
		{
			SetActorLocation(hitResult_.Location + FVector(0.f, 0.f, 0.f));
		}

		AddInsectToWorld(FTransform(rotation_, hitResult_.Location, FVector(randScale_, randScale_, randScale_)), i);
	}

	DivideInsectsArrayForThreads();

	for (int i = 0; i < swarmInsectSystemPoolThreadArr.Num(); i++)
	{
		swarmInsectSystemPoolThreadArr[i] = nullptr;
	}

	GenerateSwarmInsectsTread();

	FVector spawnLoc_ = GetActorLocation();

	for (int i = 0; i < swarmInsectSystemPoolThreadArr.Num(); i++)
	{
		swarmInsectSystemPoolThreadArr[i]->SetSpawnActorLocation(spawnLoc_);
	}

	if (maxSounds > 0 && insectSound != nullptr && bEnableSound)
	{
		for (int i = 0; i < maxSounds; ++i)
		{
			UAudioComponent* newAudioComp_ = NewObject<UAudioComponent>(this, UAudioComponent::StaticClass(), TEXT("DynamicSpawnedUAudioComponent"));
			if (newAudioComp_)
			{
				newAudioComp_->RegisterComponent();
				newAudioComp_->SetSound(insectSound);
				newAudioComp_->Play();
				allAudioComponentsArr.Add(newAudioComp_);
			}
		}
	}

	if (insectParameters.ExperimentalOptimization)
	{
		GetWorldTimerManager().SetTimer(insectsWorkerTimer_Handle, this, &AInsectsSystemActor::InsectWorkerTimer, workerRate, true, workerRate);
	}

	if (insectParameters.bCanAttackPawn)
	{
		GetWorldTimerManager().SetTimer(insectsDamageDealTimer_Handle, this, &AInsectsSystemActor::DamageDealTimer, damageDealRate, true, damageDealRate);
	}
}

void AInsectsSystemActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (int i = 0; i < swarmInsectSystemPoolThreadArr.Num(); i++)
	{
		if (swarmInsectSystemPoolThreadArr[i])
		{
			swarmInsectSystemPoolThreadArr[i]->EnsureCompletion();
			delete swarmInsectSystemPoolThreadArr[i];
			swarmInsectSystemPoolThreadArr[i] = nullptr;
		}
	}
	swarmInsectSystemPoolThreadArr.Empty();

	Super::EndPlay(EndPlayReason);
}

void AInsectsSystemActor::BeginDestroy()
{
	for (int i = 0; i < swarmInsectSystemPoolThreadArr.Num(); i++)
	{
		if (swarmInsectSystemPoolThreadArr[i])
		{
			swarmInsectSystemPoolThreadArr[i]->EnsureCompletion();
			delete swarmInsectSystemPoolThreadArr[i];
			swarmInsectSystemPoolThreadArr[i] = nullptr;
		}
	}
	swarmInsectSystemPoolThreadArr.Empty();

	Super::BeginDestroy();
}

void AInsectsSystemActor::DivideInsectsArrayForThreads()
{
	TArray<FInsectData> tempInsectData_;
	FInsectsArrays newInsectData_;

	if (maxUseThreads > insectInstances)
	{
		newInsectData_.AddData(0, tempInsectData_);
		allInsectsArrays.Add(newInsectData_);
		allInsectsArrays[0].swarmInsectsArr = swarmInsectsData;
		return;
	}

	int insectID_(0);
	int insectPart_ = swarmInsectsData.Num() / maxUseThreads;
	int const insectPartIncrement_ = insectPart_;

	for (int i = 0; i < maxUseThreads; i++)
	{
		for ( ; insectID_ < insectPart_; insectID_++)
		{
			tempInsectData_.Add(swarmInsectsData[insectID_]);
		}

		newInsectData_.AddData(i, tempInsectData_);
		allInsectsArrays.Add(newInsectData_);
		tempInsectData_.Empty();

		insectPart_ += insectPartIncrement_;
	}

	// Adding the remainder of the division.
	if (insectID_ < swarmInsectsData.Num() - 1 && insectID_ == (insectPartIncrement_ * maxUseThreads))
	{
		for ( ; insectID_ < swarmInsectsData.Num(); insectID_++)
		{
			allInsectsArrays[maxUseThreads - 1].swarmInsectsArr.Add(swarmInsectsData[insectID_]);
		}
	}
}

void AInsectsSystemActor::AddInsectToWorld(const FTransform& WorldTransform, int& setIndex)
{
	StaticMeshInstanceComponent->AddInstanceWorldSpace(WorldTransform);

	FInsectData insect_;
	insect_.instanceIndex = setIndex;
	insect_.transform = WorldTransform;
	swarmInsectsData.Add(insect_);
}

void AInsectsSystemActor::GenerateSwarmInsectsTread()
{
	if (maxUseThreads > insectInstances)
	{
		swarmInsectSystemPoolThreadArr.Add(new InsectsThread(this));
		swarmInsectSystemPoolThreadArr[0]->InitInsectsParameters(allInsectsArrays[0].swarmInsectsArr, insectParameters);
		swarmInsectSystemPoolThreadArr[0]->SetPoolThread(swarmInsectSystemPoolThreadArr);
		return;
	}
	for (int i = 0; i < maxUseThreads; i++)
	{
		swarmInsectSystemPoolThreadArr.Add(new InsectsThread(this));
		swarmInsectSystemPoolThreadArr[i]->InitInsectsParameters(allInsectsArrays[i].swarmInsectsArr, insectParameters);
	}

	// Set pool thread for control insect from first thread.
	for (int i = 0; i < swarmInsectSystemPoolThreadArr.Num(); i++)
	{
		swarmInsectSystemPoolThreadArr[i]->SetPoolThread(swarmInsectSystemPoolThreadArr);
	}
}

void AInsectsSystemActor::InsectWorkerTimer()
{
	if (GetIsInsectsOnPause_BP())
	{
		return;
	}
	
	if (!StaticMeshInstanceComponent || !bIsInsectsActivated)
	{
		return;
	}

	TArray<FInsectData> swarmInsectMembersDataArr_;

	for (int i = 0; i < swarmInsectSystemPoolThreadArr.Num(); i++)
	{
		if (swarmInsectSystemPoolThreadArr[i])
		{
			TArray<FInsectData> tempArr_ = swarmInsectSystemPoolThreadArr[i]->GetInsectsData(enemyInteractActorsArr, targetInteractActorsArr);
			for (int id = 0; id < tempArr_.Num(); id++)
			{
				swarmInsectMembersDataArr_.Add(tempArr_[id]);
			}
		}
	}

	bool aliveInstance_(false);

	TArray<float> allDistanceToInsectsArr_;
	TArray<FInsectsAudio> allInsectsAudioArr_;

	// Move insects. 
	for (int32 insectID_ = 0; insectID_ < swarmInsectMembersDataArr_.Num(); ++insectID_)
	{
		if (!swarmInsectsData[insectID_].bIsDead)
		{
			aliveInstance_ = true;

			swarmInsectsData[insectID_].transform.SetLocation(swarmInsectMembersDataArr_[insectID_].transform.GetLocation());
			swarmInsectsData[insectID_].transform.SetRotation(FQuat(swarmInsectMembersDataArr_[insectID_].transform.Rotator()));
			StaticMeshInstanceComponent->UpdateInstanceTransform(swarmInsectsData[insectID_].instanceIndex, swarmInsectsData[insectID_].transform, true, false);

			if (insectParameters.bCanAttackPawn && swarmInsectMembersDataArr_[insectID_].attackedActor)
			{
				UGameplayStatics::ApplyDamage(swarmInsectMembersDataArr_[insectID_].attackedActor, insectParameters.damageValue,
				                              nullptr, this, insectParameters.damageType);
			}
		}
	}

	// If all instance death destroy actor.
	if (!aliveInstance_)
	{
		for (int i = 0; i < swarmInsectSystemPoolThreadArr.Num(); i++)
		{
			swarmInsectSystemPoolThreadArr[i]->Stop();
		}
		Destroy();
	}

	StaticMeshInstanceComponent->MarkRenderStateDirty();
}

void AInsectsSystemActor::DamageDealTimer()
{
	if (GetIsInsectsOnPause_BP() || insectsDamageDealActorArr.Num() == 0)
	{
		return;
	}
	
	for (int i = 0; i < insectsDamageDealActorArr.Num(); ++i)
	{
		if (insectsDamageDealActorArr[i].actor)
		{
			UGameplayStatics::ApplyDamage(insectsDamageDealActorArr[i].actor, insectsDamageDealActorArr[i].damage,
											nullptr, this, insectParameters.damageType);
		}
	}
	insectsDamageDealActorArr.Empty();
}

FHitResult AInsectsSystemActor::SphereTrace(FVector startV, FVector& endV, float sphereRadius)
{
	FCollisionQueryParams traceParams_(TEXT("TraceCheck"), false, this);
	traceParams_.bReturnPhysicalMaterial = false;

	// Not tracing complex uses the rough collision instead making tiny objects easier to select.
	traceParams_.bTraceComplex = false;

	FHitResult hitTrace(ForceInit);
	GetWorld()->SweepSingleByChannel(hitTrace, startV, endV, this->GetActorQuat(), ECollisionChannel::ECC_Visibility, FCollisionShape::MakeSphere(sphereRadius), traceParams_);

	return hitTrace;
}

void InsectsThread::SetPoolThread(TArray<InsectsThread*> setPoolThreadArr)
{
	PoolThreadArr = setPoolThreadArr;
}

FHitResult InsectsThread::SphereTrace(FVector startV, FVector& endV, float sphereRadius)
{
	FCollisionQueryParams traceParams_(TEXT("TraceCheck"), false, ownerActor);
	//	traceParams.bTraceAsyncScene = true;
	traceParams_.bReturnPhysicalMaterial = false;

	// Not tracing complex uses the rough collision instead making tiny objects easier to select.
	traceParams_.bTraceComplex = false;

	FHitResult hitTrace(ForceInit);
	ownerActor->GetWorld()->SweepSingleByChannel(hitTrace, startV, endV, ownerActor->GetActorQuat(), ECollisionChannel::ECC_Visibility, FCollisionShape::MakeSphere(sphereRadius), traceParams_);

	return hitTrace;
}

FRotator InsectsThread::RotationFromNormal(const FVector& insectForward, const FVector& HitNormal)
{
	return FRotationMatrix::MakeFromZY(HitNormal, FVector::CrossProduct(HitNormal, insectForward)).Rotator();
}

void InsectsThread::AvoidVolumeActor(FHitResult setHitResult, FInsectData& insectMember, FHitResult floorHitRes)
{
	AInsectsAvoidVolume* avoidVolume_ = Cast<AInsectsAvoidVolume>(setHitResult.GetActor());
	if (avoidVolume_)
	{
		FVector runVector_ = setHitResult.Location - insectMember.transform.GetLocation();
		runVector_.RotateAngleAxis(FMath::RandRange(20.f, 220.f), insectMember.transform.Rotator().Vector());
		runVector_ *= -1.f;
		runVector_.Normalize();
		FRotator runRotate_; // = RotationFromNormal(runVector_, floorHitRes.Normal);


		runRotate_ = RotationFromNormal(UKismetMathLibrary::GetRightVector(insectMember.transform.Rotator()) +
		                                UKismetMathLibrary::GetForwardVector(insectMember.transform.Rotator()), floorHitRes.Normal);
		if (FMath::RandBool())
		{
			runRotate_ *= -1.f;
		}


		insectMember.transform.SetRotation(runRotate_.Quaternion());

		// Move actor.
		//	insectMember.velocity = insectMember.transform.Rotator().Vector() * (insectMember.insectSpeed * threadDeltaTime);
		//	insectMember.transform.SetLocation(insectMember.transform.GetLocation() + insectMember.velocity);
	}
}

bool InsectsThread::IsNeedAvoid(FHitResult setHitResult)
{
	bool bIsAvoid_(false);

	AInsectsAvoidVolume* avoidVolume_ = Cast<AInsectsAvoidVolume>(setHitResult.GetActor());
	if (avoidVolume_)
	{
		bIsAvoid_ = true;
	}
	return bIsAvoid_;
}

void InsectsThread::IsPointInAvoidBoxSetOriginLocation(FInsectData& insectMember, AInsectsAvoidVolume* avoidVolume)
{
	if (avoidVolume)
	{
		FTransform boxTransform_ = avoidVolume->BoxComponent->GetComponentTransform();
		if (UKismetMathLibrary::IsPointInBoxWithTransform(insectMember.transform.GetLocation(), boxTransform_, avoidVolume->BoxComponent->GetScaledBoxExtent()))
		{
			if (!insectMember.bIsIdleNow)
			{
				insectMember.transform.SetLocation(ownerActor->GetActorLocation());
			}
		}
	}
}

InsectsThread::InsectsThread(AActor* newActor)
{
	m_Kill = false;
	m_Pause = false;

	//Initialize FEvent (as a cross platform (Confirmed Mac/Windows))
	m_semaphore = FGenericPlatformProcess::GetSynchEventFromPool(false);

	EThreadPriority threadPriority_;

	switch (swarmInsectsParameters.threadPriority)
	{
	case EPriorityOfThread::Normal:
		threadPriority_ = TPri_Normal;
		break;
	case EPriorityOfThread::Highest:
		threadPriority_ = TPri_Highest;
		break;
	case EPriorityOfThread::Lowest:
		threadPriority_ = TPri_Lowest;
		break;
	case EPriorityOfThread::AboveNormal:
		threadPriority_ = TPri_AboveNormal;
		break;
	case EPriorityOfThread::BelowNormal:
		threadPriority_ = TPri_BelowNormal;
		break;
	case EPriorityOfThread::SlightlyBelowNormal:
		threadPriority_ = TPri_SlightlyBelowNormal;
		break;
	case EPriorityOfThread::TimeCritical:
		threadPriority_ = TPri_TimeCritical;
		break;

	default:
		threadPriority_ = TPri_AboveNormal;
		break;
	}

	ownerActor = newActor;

	Thread = FRunnableThread::Create(this, (TEXT("%s_FSomeRunnable"), *newActor->GetName()), 0, threadPriority_);
}

InsectsThread::~InsectsThread()
{
	if (m_semaphore)
	{
		//Cleanup the FEvent
		FGenericPlatformProcess::ReturnSynchEventToPool(m_semaphore);
		m_semaphore = nullptr;
	}

	if (Thread)
	{
		//Cleanup the worker thread
		delete Thread;
		Thread = nullptr;
	}
}

void InsectsThread::EnsureCompletion()
{
	Stop();

	if (Thread)
	{
		Thread->WaitForCompletion();
	}
}

void InsectsThread::PauseThread()
{
	m_Pause = true;
}

void InsectsThread::ContinueThread()
{
	m_Pause = false;

	if (m_semaphore)
	{
		//Here is a FEvent signal "Trigger()" -> it will wake up the thread.
		m_semaphore->Trigger();
	}
}

bool InsectsThread::Init()
{
	return true;
}

uint32 InsectsThread::Run()
{
	//Initial wait before starting
	FPlatformProcess::Sleep(FMath::RandRange(0.05f, 0.5f));

	threadDeltaTime = 0.f;

	while (!m_Kill)
	{
		if (m_Pause)
		{
			//FEvent->Wait(); will "sleep" the thread until it will get a signal "Trigger()"
			m_semaphore->Wait();

			if (m_Kill)
			{
				return 0;
			}
		}
		else
		{
			uint64 const timePlatform = FPlatformTime::Cycles64();

			//Critical section:
			m_mutex.Lock();

			TArray<FInsectsInteractActor> enemyActorsArr_ = enemyActorsArrTHR;
			TArray<FInsectsInteractActor> targetActorsArr_ = targetActorsArrTHR;

			m_mutex.Unlock();

			for (int32 insectID_ = 0; insectID_ < swarmInsectInThreadArr.Num(); ++insectID_)
			{
				FInsectData& insectMember_ = swarmInsectInThreadArr[insectID_];

				float *insectSpeed_;
				if (insectMember_.bInsectFleeSpeed)
				{
					insectSpeed_ = &insectMember_.insectFleeSpeed;
				}
				else if (insectMember_.bInsectFollowSpeed)
				{
					insectSpeed_ = &insectMember_.insectFollowSpeed;
				}
				else
				{
					insectSpeed_ = &insectMember_.insectSpeed;
				}
				

				// Idle calculation for insects.
				if (swarmInsectsParameters.bIsCanInsectIdle)
				{
					if (!insectMember_.bIsIdleNow)
					{
						insectMember_.bIsIdleRotate = false;
						insectMember_.walkTime += threadDeltaTime;
						insectMember_.idleTime = 0.f;

						if (insectMember_.walkTime >= insectMember_.canWalkTime)
						{
							insectMember_.bIsIdleNow = true;
							insectMember_.canIdleTime = FMath::RandRange(swarmInsectsParameters.minIdleTime, swarmInsectsParameters.maxIdleTime);
						}
					}
					else
					{
						insectMember_.idleTime += threadDeltaTime;
						insectMember_.walkTime = 0.f;

						if (insectMember_.idleTime >= insectMember_.canIdleTime)
						{
							insectMember_.bIsIdleNow = false;
							insectMember_.canWalkTime = FMath::RandRange(swarmInsectsParameters.minWalkTime, swarmInsectsParameters.maxWalkTime);
						}
					}
				}

				FRotator const insectRotation_(insectMember_.transform.Rotator());
				FVector const insectUpVector_(UKismetMathLibrary::GetUpVector(insectRotation_));
				FVector const insectForwardVector_(UKismetMathLibrary::GetForwardVector(insectRotation_));
				FVector const insectLocation_(insectMember_.transform.GetLocation());


				insectMember_.elapsedTimeSinceLastWander += threadDeltaTime;

				bool bIsTryRotateToFloor_(false);

				// Trace check Floor.
				FVector findFloorStartV_ = insectLocation_ + insectUpVector_ * 2.f;
				FVector findFloorEndV_ = insectUpVector_ * -15.f + insectLocation_;
				FHitResult const floorHitResult_ = SphereTrace(findFloorStartV_, findFloorEndV_, 2.f);

				// Trace check forward obstacle.
				FVector findWallStartV_ = insectUpVector_ * 2.f + insectLocation_;
				FVector findWallForwardV_ = insectForwardVector_ * 10.f + findWallStartV_;
				findWallForwardV_ += insectRotation_.Vector() * (*insectSpeed_ * threadDeltaTime);
				FHitResult const wallHitResult_ = SphereTrace(findWallStartV_, findWallForwardV_, 0.3f);

				FRotator targetRot_;
				FRotator newRot_;
				if (wallHitResult_.bBlockingHit)
				{
					AInsectsAvoidVolume* avoidVolume_ = Cast<AInsectsAvoidVolume>(wallHitResult_.GetActor());
					if (!avoidVolume_)
					{
						// Rotate actor to wall.
						targetRot_ = RotationFromNormal(insectUpVector_, wallHitResult_.Normal);
						newRot_ = FMath::RInterpTo(insectRotation_, targetRot_, threadDeltaTime, 20.f);

						insectMember_.transform.SetRotation(targetRot_.Quaternion());
					}
				}
				else
				{
					FVector endLocCheckFloor_ = (insectUpVector_ * -1.f + insectForwardVector_) * groundCheckTraceDistance + insectLocation_;

					// Trace check forward Ground.
					FHitResult forwardGroundHit_ = SphereTrace(insectLocation_ + insectUpVector_ * 2.f, endLocCheckFloor_, 2.f);

					// Try find ground.
					if (!forwardGroundHit_.bBlockingHit || IsNeedAvoid(forwardGroundHit_) || IsNeedAvoid(floorHitResult_))
					{
						bIsTryRotateToFloor_ = true;

						insectMember_.bIsIdleNow = false;

						insectMember_.timeFindFloor += threadDeltaTime;


						bool findPoint_(false);
						for (int i = 0; i < 30; i++)
						{
							if (!findPoint_)
							{
								FVector startGroundFindV_ = insectLocation_ + UKismetMathLibrary::GetForwardVector(insectRotation_ * (10.f + (*insectSpeed_ * threadDeltaTime)));

								FVector endGroundFindV_ = UKismetMathLibrary::RandomRotator(true).Vector() * 20.f + (*insectSpeed_ * threadDeltaTime) + startGroundFindV_;

								FHitResult findGroundHit_ = SphereTrace(startGroundFindV_, endGroundFindV_, 1.f);

								if (findGroundHit_.bBlockingHit && findGroundHit_.Location != FVector::ZeroVector && !IsNeedAvoid(findGroundHit_) && !insectMember_.bIsIdleNow)
								{
									FRotator groundFindRot_ = RotationFromNormal(insectUpVector_ * -1.f, findGroundHit_.Normal);
									insectMember_.transform.SetRotation(groundFindRot_.Quaternion());
									insectMember_.transform.SetLocation(findGroundHit_.Location);

									findPoint_ = true;
								}
							}
							if (findPoint_)
							{
								break;
							}
						}

						if (!findPoint_)
						{
							FVector findDownFloorStart_ = insectLocation_;
							findDownFloorStart_.Z -= 5.f;
							FVector findDownFloorEnd_ = findDownFloorStart_;
							findDownFloorEnd_.Z -= 10000.f;
							FHitResult findDownFloorHit_ = SphereTrace(findDownFloorStart_, findDownFloorEnd_, 1.f);
							if (findDownFloorHit_.bBlockingHit && findDownFloorHit_.Location != FVector::ZeroVector && !insectMember_.bIsIdleNow)
							{
								insectMember_.transform.SetLocation(findDownFloorHit_.Location);
							}
						}

						// If insect cant find floor with trace.
						if (insectMember_.timeFindFloor > 0.2f)
						{
							insectMember_.transform.SetRotation(UKismetMathLibrary::RandomRotator(true).Quaternion());
						}
					}
					else if (IsNeedAvoid(forwardGroundHit_))
					{
						AvoidVolumeActor(forwardGroundHit_, insectMember_, floorHitResult_);
					}
					else if (floorHitResult_.Location != FVector::ZeroVector && floorHitResult_.bBlockingHit)
					{
						if (IsNeedAvoid(floorHitResult_))
						{
							AvoidVolumeActor(floorHitResult_, insectMember_, floorHitResult_);
						}
						else
						{
							// Reset timer.
							insectMember_.timeFindFloor = 0.f;

							// Rotate to floor normal.
							targetRot_ = RotationFromNormal(insectForwardVector_, floorHitResult_.Normal);
							if (targetRot_ != FRotator::ZeroRotator)
							{
								newRot_ = FMath::RInterpTo(insectRotation_, targetRot_, threadDeltaTime, 20.f);
								insectMember_.transform.SetRotation(newRot_.Quaternion());
							}

							// Run around spawn location
							if ((insectLocation_ - spawnActorLocation).SizeSquared() > swarmInsectsParameters.insectsWanderInRandomRadiusSquare)
							{
								FVector runVector_ = spawnActorLocation - insectLocation_;
								runVector_.Normalize();
								FRotator runRotate_ = RotationFromNormal(runVector_, floorHitResult_.Normal);
								insectMember_.transform.SetRotation(runRotate_.Quaternion());
							}

							// Random change course. 
							if (insectMember_.elapsedTimeSinceLastWander >= FMath::RandRange(3.f, 10.f) && !bIsTryRotateToFloor_ &&
								floorHitResult_.bBlockingHit && floorHitResult_.Location != FVector::ZeroVector || insectMember_.bIsIdleNow && !insectMember_.bIsIdleRotate)
							{
								// One rotate insect if idle.
								if (insectMember_.bIsIdleNow)
								{
									insectMember_.bIsIdleRotate = true;
								}

								// Rotate to floor normal.
								targetRot_ = RotationFromNormal(UKismetMathLibrary::GetRightVector(insectRotation_) + insectForwardVector_, floorHitResult_.Normal);
								if (FMath::RandBool())
								{
									targetRot_ *= -1.f;
								}
								newRot_ = FMath::RInterpTo(insectRotation_, targetRot_, threadDeltaTime, 20.f);
								insectMember_.transform.SetRotation(newRot_.Quaternion());

								insectMember_.elapsedTimeSinceLastWander = 0.f;
							}

							// Pursue target actors.
							if (targetActorsArr_.Num() > 0 && swarmInsectsParameters.bReactOnPawn)
							{
								TArray<float> distanceToTargetSquareArr_;
								for (int enemyID_ = 0; enemyID_ < targetActorsArr_.Num(); enemyID_++)
								{
									if (targetActorsArr_[enemyID_].interactActor)
									{
										distanceToTargetSquareArr_.Add((targetActorsArr_[enemyID_].interactActor->GetActorLocation() - insectLocation_).SizeSquared());
									}
								}
								
								int idTargetActor_;
								float distanceToTargetSquare_;
								UKismetMathLibrary::MinOfFloatArray(distanceToTargetSquareArr_, idTargetActor_, distanceToTargetSquare_);
							
								AActor* targetActor_ = nullptr;
								if (targetActorsArr_.IsValidIndex(idTargetActor_))
								{
									targetActor_ = targetActorsArr_[idTargetActor_].interactActor;
								}

								if (distanceToTargetSquare_ < targetActorsArr_[idTargetActor_].responseRadiusSquare && distanceToTargetSquare_ > 10000.f && floorHitResult_.Location != FVector::ZeroVector)
								{
									insectMember_.bIsIdleNow = false;
									insectMember_.bInsectFollowSpeed = true;
							
									FVector runVector_(FVector::ZeroVector);
									if (targetActor_)
									{
										runVector_ = targetActor_->GetActorLocation() - insectLocation_;
									}
							
									runVector_.Normalize();
									FRotator runRotate_ = RotationFromNormal(runVector_, floorHitResult_.Normal);
							
									insectMember_.transform.SetRotation(runRotate_.Quaternion());
								}
								else
								{
									insectMember_.bInsectFollowSpeed = false;
								}
							
								// Add actors for attacking. 
								if (swarmInsectsParameters.bCanAttackPawn)
								{
									if (floorHitResult_.GetActor())
									{
										if (floorHitResult_.GetActor() == targetActor_)
										{
											insectMember_.attackedActor = targetActor_;
										}
									}
									else if (floorHitResult_.GetComponent())
									{
										if (floorHitResult_.GetComponent()->GetOwner() == targetActor_)
										{
											insectMember_.attackedActor = targetActor_;
										}
									}
								}
							}

							// Flee from enemy actors.
							if (enemyActorsArr_.Num() > 0 && swarmInsectsParameters.bReactOnPawn)
							{
								TArray<float> distanceToEnemySquareArr_;
								for (int enemyID_ = 0; enemyID_ < enemyActorsArr_.Num(); enemyID_++)
								{
									if (enemyActorsArr_[enemyID_].interactActor)
									{
										distanceToEnemySquareArr_.Add((enemyActorsArr_[enemyID_].interactActor->GetActorLocation() - insectLocation_).SizeSquared());
									}
								}
								
								int idEnemyActor_;
								float distanceToEnemySquare_;
								UKismetMathLibrary::MinOfFloatArray(distanceToEnemySquareArr_, idEnemyActor_, distanceToEnemySquare_);

								AActor* enemyActor_ = nullptr;
								if (enemyActorsArr_.IsValidIndex(idEnemyActor_))
								{
									enemyActor_ = enemyActorsArr_[idEnemyActor_].interactActor;
								}

								if (distanceToEnemySquare_ < enemyActorsArr_[idEnemyActor_].responseRadiusSquare && floorHitResult_.Location != FVector::ZeroVector)
								{
									insectMember_.bIsIdleNow = false;
									insectMember_.bInsectFleeSpeed = true;
									insectMember_.bInsectFollowSpeed = false;
									
									FVector runVector_(FVector::ZeroVector);
									if (enemyActor_)
									{
										runVector_ = enemyActor_->GetActorLocation() - insectLocation_;
										runVector_ *= -1.f;
									}

									runVector_.Normalize();
									FRotator runRotate_ = RotationFromNormal(runVector_, floorHitResult_.Normal);

									insectMember_.transform.SetRotation(runRotate_.Quaternion());
								}
								else
								{
									insectMember_.bInsectFleeSpeed = false;
								}

								// Add actors for attacking. 
								if (swarmInsectsParameters.bCanAttackPawn)
								{
									if (floorHitResult_.GetActor())
									{
										if (floorHitResult_.GetActor() == enemyActor_)
										{
											insectMember_.attackedActor = enemyActor_;
										}
									}
									else if (floorHitResult_.GetComponent())
									{
										if (floorHitResult_.GetComponent()->GetOwner() == enemyActor_)
										{
											insectMember_.attackedActor = enemyActor_;
										}
									}
								}
							}
						}
					}
				}

				if (wallHitResult_.bBlockingHit && !bIsTryRotateToFloor_)
				{
					// Avoid volume.
					if (IsNeedAvoid(wallHitResult_))
					{
						AvoidVolumeActor(wallHitResult_, insectMember_, floorHitResult_);
					}
					else if (!insectMember_.bIsIdleNow)
					{
						// Move to wall.
						insectMember_.transform.SetLocation(wallHitResult_.Location);
					}
				}
				else if (!bIsTryRotateToFloor_ && !insectMember_.bIsIdleNow)
				{
					// Set location near floor.
					if (floorHitResult_.bBlockingHit && floorHitResult_.Location != FVector::ZeroVector)
					{
						insectMember_.transform.SetLocation(floorHitResult_.Location);
					}
					// Move actor.
					insectMember_.velocity = insectRotation_.Vector() * (*insectSpeed_ * threadDeltaTime);

					insectMember_.transform.SetLocation(insectLocation_ + insectMember_.velocity);
				}

				if (wallHitResult_.bBlockingHit)
				{
					AInsectsAvoidVolume* avoidVolume_ = Cast<AInsectsAvoidVolume>(wallHitResult_.GetActor());
					if (avoidVolume_)
					{
						IsPointInAvoidBoxSetOriginLocation(insectMember_, avoidVolume_);
					}
				}
				else if (floorHitResult_.bBlockingHit)
				{
					AInsectsAvoidVolume* avoidVolume_ = Cast<AInsectsAvoidVolume>(floorHitResult_.GetActor());
					if (avoidVolume_)
					{
						IsPointInAvoidBoxSetOriginLocation(insectMember_, avoidVolume_);
					}
				}

				swarmInsectInThreadArr[insectID_] = insectMember_;
			}

			threadDeltaTime = FTimespan(FPlatformTime::Cycles64() - timePlatform).GetTotalSeconds();

			if (swarmInsectsParameters.ExperimentalOptimization)
			{
				threadDeltaTime += threadSleepTime;
			}

			//Critical section:
			//	m_mutex.Lock();
			//We are locking our FCriticalSection so no other thread will access it
			//And thus it is a thread-safe access now

			//	swarmInsectInThreadArr = insectsArr_;

			//Unlock FCriticalSection so other threads may use it.
			//	m_mutex.Unlock();

			//Pause Condition - if we RandomVectors contains more vectors than m_amount we shall pause the thread to release system resources.
			m_Pause = true;

			if (swarmInsectsParameters.ExperimentalOptimization)
			{
				//A little sleep between the chunks (So CPU will rest a bit -- (may be omitted))
				FPlatformProcess::Sleep(threadSleepTime);
			}
		}
	}
	return 0;
}

void InsectsThread::Stop()
{
	m_Kill = true; //Thread kill condition "while (!m_Kill){...}"
	m_Pause = false;

	if (m_semaphore)
	{
		//We shall signal "Trigger" the FEvent (in case the Thread is sleeping it shall wake up!!)
		m_semaphore->Trigger();
	}
}

bool InsectsThread::IsThreadPaused()
{
	return (bool)m_Pause;
}

TArray<FInsectData> InsectsThread::GetInsectsData(TArray<FInsectsInteractActor> setEnemyActorsArr, TArray<FInsectsInteractActor> setTargetActorsArr)
{
	m_mutex.Lock();

	TArray<FInsectData> actualArray_ = swarmInsectInThreadArr;

	enemyActorsArrTHR = setEnemyActorsArr;
	targetActorsArrTHR = setTargetActorsArr;

	// Clear attacked actors in thread
	if (swarmInsectsParameters.bCanAttackPawn)
	{
		for (int i = 0; i < swarmInsectInThreadArr.Num(); ++i)
		{
			swarmInsectInThreadArr[i].attackedActor = nullptr;
		}
	}

	this->ContinueThread();

	m_mutex.Unlock();

	return actualArray_;
}

void InsectsThread::InitInsectsParameters(TArray<FInsectData> setInsectsArr, FInsectParameters newParameters)
{
	swarmInsectsParameters = newParameters;
	swarmInsectInThreadArr = setInsectsArr;

	for (int i = 0; i < swarmInsectInThreadArr.Num(); i++)
	{
		float randWalkSpeed_ = swarmInsectsParameters.insectWalkSpeed + FMath::RandRange(-swarmInsectsParameters.insectOffsetWalkSpeed, swarmInsectsParameters.insectOffsetWalkSpeed);
		swarmInsectInThreadArr[i].insectSpeed = randWalkSpeed_;

		float randFollowSpeed_ =  swarmInsectsParameters.insectFollowSpeed + FMath::RandRange(-swarmInsectsParameters.insectOffsetFollowSpeed, swarmInsectsParameters.insectOffsetFollowSpeed);
		swarmInsectInThreadArr[i].insectFollowSpeed = randFollowSpeed_;

		float randFleeSpeed_ =  swarmInsectsParameters.insectFleeSpeed + FMath::RandRange(-swarmInsectsParameters.insectOffsetFleeSpeed, swarmInsectsParameters.insectOffsetFleeSpeed);
		swarmInsectInThreadArr[i].insectFleeSpeed = randFleeSpeed_;
		
		swarmInsectInThreadArr[i].canIdleTime = FMath::RandRange(swarmInsectsParameters.minIdleTime, swarmInsectsParameters.maxIdleTime);
		swarmInsectInThreadArr[i].canWalkTime = FMath::RandRange(swarmInsectsParameters.minWalkTime, swarmInsectsParameters.maxWalkTime);
	}
}

void InsectsThread::SetSpawnActorLocation(FVector& newPoint)
{
	spawnActorLocation = newPoint;
}

// Called every frame
void AInsectsSystemActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetIsInsectsOnPause_BP())
	{
		return;
	}

	if (!StaticMeshInstanceComponent || !bIsInsectsActivated || insectParameters.ExperimentalOptimization)
	{
		return;
	}

	TArray<FInsectData> swarmInsectMembersDataArr_;

	for (int i = 0; i < swarmInsectSystemPoolThreadArr.Num(); i++)
	{
		if (swarmInsectSystemPoolThreadArr[i])
		{
			TArray<FInsectData> tempArr_ = swarmInsectSystemPoolThreadArr[i]->GetInsectsData(enemyInteractActorsArr, targetInteractActorsArr);
			for (int id = 0; id < tempArr_.Num(); id++)
			{
				swarmInsectMembersDataArr_.Add(tempArr_[id]);
			}
		}
	}

	bool aliveInstance_(false);

	TArray<FInsectsAudio> allInsectsAudioArr_;

	// Move insects. 
	for (int32 insectID_ = 0; insectID_ < swarmInsectMembersDataArr_.Num(); ++insectID_)
	{
		if (!swarmInsectsData[insectID_].bIsDead)
		{
			aliveInstance_ = true;


			//Interpolate Vector and Rotate
			FVector interpVector = FMath::VInterpConstantTo(swarmInsectsData[insectID_].transform.GetLocation(),
			                                                swarmInsectMembersDataArr_[insectID_].transform.GetLocation(), DeltaTime, interpMoveAnimRate);

			swarmInsectsData[insectID_].transform.SetLocation(interpVector);
			swarmInsectsData[insectID_].transform.SetRotation(FQuat(swarmInsectMembersDataArr_[insectID_].transform.Rotator()));
			StaticMeshInstanceComponent->UpdateInstanceTransform(swarmInsectsData[insectID_].instanceIndex, swarmInsectsData[insectID_].transform, true, false);

			if (insectParameters.bCanAttackPawn && swarmInsectMembersDataArr_[insectID_].attackedActor)
			{				
				bool bFoundActorForDealDamage_(false);
				for (int i = 0; i < insectsDamageDealActorArr.Num(); ++i)
				{
					if (insectsDamageDealActorArr[i].actor == swarmInsectMembersDataArr_[insectID_].attackedActor)
					{
						insectsDamageDealActorArr[i].damage += insectParameters.damageValue;
						bFoundActorForDealDamage_ = true;
					}
				}
				if (!bFoundActorForDealDamage_)
				{
					FInsectsDamageDealActor newActor_;
					newActor_.actor = swarmInsectMembersDataArr_[insectID_].attackedActor;
					newActor_.damage = insectParameters.damageValue;
					insectsDamageDealActorArr.Add(newActor_);
				}
			}

			if (maxSounds > 0 && insectSound != nullptr && bEnableSound)
			{
				APlayerCameraManager* playerCam_ = UGameplayStatics::GetPlayerCameraManager(this, 0);
				if (playerCam_)
				{
					FVector camLoc_ = playerCam_->GetCameraLocation();

					FInsectsAudio insectsAudio_;
					insectsAudio_.instanceIndex = insectID_;
					insectsAudio_.insectDistance = (camLoc_ - swarmInsectsData[insectID_].transform.GetLocation()).SizeSquared();
					allInsectsAudioArr_.Add(insectsAudio_);
				}
			}
		}
	}

	if (maxSounds > allInsectsAudioArr_.Num() && bEnableSound)
	{
		maxSounds = allInsectsAudioArr_.Num();
	}

	if (maxSounds > 0 && insectSound != nullptr && bEnableSound)
	{
		TArray<float> allDistanceToInsectsArr_;
		for (int i = 0; i < allInsectsAudioArr_.Num(); ++i)
		{
			allDistanceToInsectsArr_.Add(allInsectsAudioArr_[i].insectDistance);
		}
		for (int i = 0; i < maxSounds - 1; ++i)
		{
			int index_;
			float minVal_;
			UKismetMathLibrary::MinOfFloatArray(allDistanceToInsectsArr_, index_, minVal_);

			allAudioComponentsArr[i]->SetWorldLocation(swarmInsectsData[allInsectsAudioArr_[index_].instanceIndex].transform.GetLocation());

			if (bIsDebugSound)
			{
				DrawDebugSphere(GetWorld(), swarmInsectsData[allInsectsAudioArr_[index_].instanceIndex].transform.GetLocation(),
				                30.f, 8, FColor::Green, false, 0.f, 0, 1.f);

				DrawDebugString(GetWorld(), swarmInsectsData[allInsectsAudioArr_[index_].instanceIndex].transform.GetLocation(), TEXT("Sound is here!"),
				                0, FColor::Orange, 0, true, 1);
			}

			allDistanceToInsectsArr_.RemoveAt(index_);
			allInsectsAudioArr_.RemoveAt(index_);
		}
	}

	// If all instance death destroy actor.
	if (!aliveInstance_)
	{
		for (int i = 0; i < swarmInsectSystemPoolThreadArr.Num(); i++)
		{
			swarmInsectSystemPoolThreadArr[i]->Stop();
		}
		Destroy();
	}

	StaticMeshInstanceComponent->MarkRenderStateDirty();
}
