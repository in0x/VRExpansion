// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "IMotionController.h"

#include "HeadMountedDisplay.h" 
#include "HeadMountedDisplayFunctionLibrary.h"

#include "VRBPDatatypes.h"

#include "VRExpansionFunctionLibrary.generated.h"

//General Advanced Sessions Log
DECLARE_LOG_CATEGORY_EXTERN(VRExpansionFunctionLibraryLog, Log, All);

/**
* Redefining this for blueprint as it wasn't declared as BlueprintType
* Stores if the user is wearing the HMD or not. For HMDs without a sensor to detect the user wearing it, the state defaults to Unknown.
*/
UENUM(BlueprintType)
enum class EBPHMDWornState : uint8
{
	Unknown UMETA(DisplayName = "Unknown"),
	Worn UMETA(DisplayName = "Worn"),
	NotWorn UMETA(DisplayName = "Not Worn"),
};


UCLASS()//, meta = (BlueprintSpawnableComponent))
class VREXPANSIONPLUGIN_API UVRExpansionFunctionLibrary : public UBlueprintFunctionLibrary
{
	//GENERATED_BODY()
	GENERATED_BODY()
	//~UVRExpansionFunctionLibrary();
public:

	// Gets the unwound yaw of the HMD
	UFUNCTION(BlueprintPure, Category = "VRExpansionFunctions", meta = (bIgnoreSelf = "true", DisplayName = "GetHMDPureYaw"))
	static FRotator GetHMDPureYaw(FRotator HMDRotation);

	FORCEINLINE static FRotator GetHMDPureYaw_I(FRotator HMDRotation)
	{
		// Took this from UnityVRToolkit, no shame, I liked it
		FRotationMatrix HeadMat(HMDRotation);
		FVector forward = HeadMat.GetScaledAxis(EAxis::X);
		FVector forwardLeveled = forward;
		forwardLeveled.Z = 0;
		forwardLeveled.Normalize();
		FVector mixedInLocalForward = HeadMat.GetScaledAxis(EAxis::Z);

		if (forward.Z > 0)
		{
			mixedInLocalForward = -mixedInLocalForward;
		}

		mixedInLocalForward.Z = 0;
		mixedInLocalForward.Normalize();
		float dot = FMath::Clamp(FVector::DotProduct(forwardLeveled, forward), 0.0f, 1.0f);
		FVector finalForward = FMath::Lerp(mixedInLocalForward, forwardLeveled, dot * dot);

		return FRotationMatrix::MakeFromXZ(finalForward, FVector::UpVector).Rotator();
	}

	// Applies a delta rotation around a pivot point, if bUseOriginalYawOnly is true then it only takes the original Yaw into account (characters)
	UFUNCTION(BlueprintPure, Category = "VRExpansionFunctions", meta = (bIgnoreSelf = "true", DisplayName = "RotateAroundPivot"))
	static void RotateAroundPivot(FRotator RotationDelta, FVector OriginalLocation, FRotator OriginalRotation, FVector PivotPoint, FVector & NewLocation, FRotator & NewRotation,bool bUseOriginalYawOnly = true)
	{		
		if (bUseOriginalYawOnly)
		{
			// Keep original pitch/roll
			NewRotation.Pitch = OriginalRotation.Pitch;
			NewRotation.Roll = OriginalRotation.Roll;

			// Throw out pitch/roll before calculating offset
			OriginalRotation.Roll = 0;
			OriginalRotation.Pitch = 0;

			// Offset to pivot point
			NewLocation = OriginalLocation + OriginalRotation.RotateVector(PivotPoint);

			// Combine rotations
			OriginalRotation.Yaw = (OriginalRotation.Quaternion() * RotationDelta.Quaternion()).Rotator().Yaw;
			NewRotation.Yaw = OriginalRotation.Yaw;

			// Remove pivot point offset
			NewLocation -= OriginalRotation.RotateVector(PivotPoint);

		}
		else
		{
			NewLocation = OriginalLocation + OriginalRotation.RotateVector(PivotPoint);
			NewRotation = (OriginalRotation.Quaternion() * RotationDelta.Quaternion()).Rotator();
			NewLocation -= NewRotation.RotateVector(PivotPoint);
		}
	}

	// Gets whether an HMD device is connected
	UFUNCTION(BlueprintPure, Category = "VRExpansionFunctions", meta = (bIgnoreSelf = "true", DisplayName = "GetIsHMDConnected"))
	static bool GetIsHMDConnected();

	// Gets whether an HMD device is connected
	UFUNCTION(BlueprintPure, Category = "VRExpansionFunctions", meta = (bIgnoreSelf = "true", DisplayName = "GetIsHMDWorn"))
	static EBPHMDWornState GetIsHMDWorn();

	// Gets whether an HMD device is connected
	UFUNCTION(BlueprintPure, Category = "VRExpansionFunctions", meta = (bIgnoreSelf = "true", DisplayName = "GetHMDType"))
	static EBPHMDDeviceType GetHMDType();

	// Gets whether the game is running in VRPreview or is a non editor build game (returns true for either).
	UFUNCTION(BlueprintPure, Category = "VRExpansionFunctions", meta = (bIgnoreSelf = "true", DisplayName = "IsInVREditorPreviewOrGame"))
	static bool IsInVREditorPreviewOrGame();

	/**
	* Finds the minimum area rectangle that encloses all of the points in InVerts
	* Engine default version is server only for some reason
	* Uses algorithm found in http://www.geometrictools.com/Documentation/MinimumAreaRectangle.pdf
	*
	* @param		InVerts	- Points to enclose in the rectangle
	* @outparam	OutRectCenter - Center of the enclosing rectangle
	* @outparam	OutRectSideA - Vector oriented and sized to represent one edge of the enclosing rectangle, orthogonal to OutRectSideB
	* @outparam	OutRectSideB - Vector oriented and sized to represent one edge of the enclosing rectangle, orthogonal to OutRectSideA
	*/
	UFUNCTION(BlueprintCallable, Category = "VRExpansionFunctions", meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext))
	static void NonAuthorityMinimumAreaRectangle(UObject* WorldContextObject, const TArray<FVector>& InVerts, const FVector& SampleSurfaceNormal, FVector& OutRectCenter, FRotator& OutRectRotation, float& OutSideLengthX, float& OutSideLengthY, bool bDebugDraw = false);

	// A Rolling average low pass filter
	UFUNCTION(BlueprintPure, Category = "VRExpansionFunctions", meta = (bIgnoreSelf = "true", DisplayName = "LowPassFilter_RollingAverage"))
	static void LowPassFilter_RollingAverage(FVector lastAverage, FVector newSample, FVector & newAverage, int32 numSamples = 10);

	// A exponential low pass filter
	UFUNCTION(BlueprintPure, Category = "VRExpansionFunctions", meta = (bIgnoreSelf = "true", DisplayName = "LowPassFilter_Exponential"))
	static void LowPassFilter_Exponential(FVector lastAverage, FVector newSample, FVector & newAverage, float sampleFactor = 0.25f);

	// Gets whether an HMD device is connected
	UFUNCTION(BlueprintPure, Category = "VRExpansionFunctions", meta = (bIgnoreSelf = "true", DisplayName = "GetIsActorMovable"))
	static bool GetIsActorMovable(AActor * ActorToCheck);

	// Gets if an actors root component contains a grip slot within range
	UFUNCTION(BlueprintPure, Category = "VRGrip", meta = (bIgnoreSelf = "true", DisplayName = "GetGripSlotInRangeByTypeName"))
	static void GetGripSlotInRangeByTypeName(FName SlotType, AActor * Actor, FVector WorldLocation, float MaxRange, bool & bHadSlotInRange, FTransform & SlotWorldTransform);

	// Gets if an actors root component contains a grip slot within range
	UFUNCTION(BlueprintPure, Category = "VRGrip", meta = (bIgnoreSelf = "true", DisplayName = "GetGripSlotInRangeByTypeName_Component"))
	static void GetGripSlotInRangeByTypeName_Component(FName SlotType, UPrimitiveComponent * Component, FVector WorldLocation, float MaxRange, bool & bHadSlotInRange, FTransform & SlotWorldTransform);

	/* Returns true if the values are equal (A == B) */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Equal VR Grip", CompactNodeTitle = "==", Keywords = "== equal"), Category = "VRExpansionFunctions")
	static bool EqualEqual_FBPActorGripInformation(const FBPActorGripInformation &A, const FBPActorGripInformation &B);


	/** Make a transform net quantize from location, rotation and scale */
	UFUNCTION(BlueprintPure, meta = (Scale = "1,1,1", Keywords = "construct build", NativeMakeFunc), Category = "VRExpansionLibrary|TransformNetQuantize")
		static FTransform_NetQuantize MakeTransform_NetQuantize(FVector Location, FRotator Rotation, FVector Scale);

	/** Breaks apart a transform net quantize into location, rotation and scale */
	UFUNCTION(BlueprintPure, Category = "VRExpansionLibrary|TransformNetQuantize", meta = (NativeBreakFunc))
		static void BreakTransform_NetQuantize(const FTransform_NetQuantize& InTransform, FVector& Location, FRotator& Rotation, FVector& Scale);

	/** Converts a FTransform into a FTransform_NetQuantize */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToTransform_NetQuantize (Transform)", CompactNodeTitle = "->", BlueprintAutocast), Category = "VRExpansionLibrary|TransformNetQuantize")
		static FTransform_NetQuantize Conv_TransformToTransformNetQuantize(const FTransform &InTransform);

	// Adds a USceneComponent Subclass, that is based on the passed in Class, and added to the Outer(Actor) object
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Add Scene Component By Class"), Category = "VRExpansionLibrary")
		static USceneComponent* AddSceneComponentByClass(UObject* Outer, TSubclassOf<USceneComponent> Class, const FTransform & ComponentRelativeTransform);
};	
