#include "Runtime.h"

float FRotator::ClampAxis(float Angle)
{
    UKismetMathLibrary::FMod(Angle, 360.f, &Angle);

    if (Angle < 0.f) Angle += 360.f;
    return Angle;
}

FName::FName(FString str) {
    *this = UKismetStringLibrary::Conv_StringToName(str);
}

FRotator FQuat::Rotator()
{
    const float SingularityTest = Z * X - W * Y;
    const float YawY = 2.f * (W * Z + X * Y);
    const float YawX = (1.f - 2.f * (UKismetMathLibrary::Square(Y) + UKismetMathLibrary::Square(Z)));

    const float SINGULARITY_THRESHOLD = 0.4999995f;
    const float RAD_TO_DEG = 57.29577951308232f;
    FRotator RotatorFromQuat{};

    if (SingularityTest < -SINGULARITY_THRESHOLD)
    {
        RotatorFromQuat.Pitch = -90.f;
        RotatorFromQuat.Yaw = UKismetMathLibrary::Atan2(YawY, YawX) * RAD_TO_DEG;
        RotatorFromQuat.Roll = FRotator::NormalizeAxis(-RotatorFromQuat.Yaw - (2.f * UKismetMathLibrary::Atan2(X, W) * RAD_TO_DEG));
    }
    else if (SingularityTest > SINGULARITY_THRESHOLD)
    {
        RotatorFromQuat.Pitch = 90.f;
        RotatorFromQuat.Yaw = UKismetMathLibrary::Atan2(YawY, YawX) * RAD_TO_DEG;
        RotatorFromQuat.Roll = FRotator::NormalizeAxis(RotatorFromQuat.Yaw - (2.f * UKismetMathLibrary::Atan2(X, W) * RAD_TO_DEG));
    }
    else
    {
        RotatorFromQuat.Pitch = UKismetMathLibrary::Asin(2.f * (SingularityTest)) * RAD_TO_DEG;
        RotatorFromQuat.Yaw = UKismetMathLibrary::Atan2(YawY, YawX) * RAD_TO_DEG;
        RotatorFromQuat.Roll = UKismetMathLibrary::Atan2(-2.f * (W * X + Y * Z), (1.f - 2.f * (UKismetMathLibrary::Square(X) + UKismetMathLibrary::Square(Y)))) * RAD_TO_DEG;
    }

    return RotatorFromQuat;
}

float Runtime::EvaluateScalableFloat(FScalableFloat& Float)
{
    if (!Float.Curve.CurveTable) return Float.Value;

    float Out;
    UDataTableFunctionLibrary::EvaluateCurveTableRow(Float.Curve.CurveTable, Float.Curve.RowName, (float)0, nullptr, &Out, FString());
    return Out;
}
