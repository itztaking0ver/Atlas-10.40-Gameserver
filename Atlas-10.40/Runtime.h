#pragma once
#include "framework.h"

namespace Funcs {
	static inline auto StaticFindObject = (SDK::UObject * (*)(SDK::UClass*, SDK::UObject*, const wchar_t*, bool)) (Runtime::Offsets::StaticFindObject);
	static inline auto StaticLoadObject = (SDK::UObject * (*)(SDK::UClass*, SDK::UObject*, const TCHAR*, const TCHAR*, uint32_t, SDK::UObject*, bool)) uint64_t(Runtime::Offsets::StaticLoadObject);
	static auto FGameplayAbilitySpecctor = reinterpret_cast<void(*)(FGameplayAbilitySpec*, UGameplayAbility*, int, int, UObject*)>(__int64(Runtime::Offsets::ConstructAbilitySpec));
	static auto GiveAbilityAndActivateOnce = reinterpret_cast<__int64(__fastcall*)(void*, FGameplayAbilitySpecHandle*, FGameplayAbilitySpec)>(__int64(Runtime::Offsets::GiveAbilityAndActivateOnce));
};

namespace Runtime
{
	static inline void* nullptrForHook = nullptr;
	__forceinline static void _HookVT(void** _Vt, uint32_t _Ind, void* _Detour)
	{
		DWORD _Vo;
		VirtualProtect(_Vt + _Ind, 8, PAGE_EXECUTE_READWRITE, &_Vo);
		_Vt[_Ind] = _Detour;
		VirtualProtect(_Vt + _Ind, 8, _Vo, &_Vo);
	}

	static void Hook(uint64 Address, void* Detour, void** OG = nullptr)
	{
		MH_CreateHook(LPVOID(Address), Detour, OG);
		MH_EnableHook(LPVOID(Address));
	}

	static void Patch(uintptr_t ptr, uint8_t byte)
	{
		DWORD og;
		VirtualProtect(LPVOID(ptr), sizeof(byte), PAGE_EXECUTE_READWRITE, &og);
		*(uint8_t*)ptr = byte;
		VirtualProtect(LPVOID(ptr), sizeof(byte), og, &og);
	}

	template <typename T>
	T* StaticFindObject(std::string ObjectPath, UClass* Class = nullptr) {
		return (T*)Funcs::StaticFindObject(Class, nullptr, std::wstring(ObjectPath.begin(), ObjectPath.end()).c_str(), false);
	}

	inline UObject* StaticFindObjectT(std::string ObjectPath, UClass* Class = nullptr) {
		return Funcs::StaticFindObject(Class, nullptr, std::wstring(ObjectPath.begin(), ObjectPath.end()).c_str(), false);
	}

	template<typename T>
	T* StaticLoadObject(std::string name) {
		T* Object = StaticFindObject<T>(name);
		if (!Object) {
			auto Name = std::wstring(name.begin(), name.end()).c_str();
			UObject* BaseObject = Funcs::StaticLoadObject(T::StaticClass(), nullptr, Name, nullptr, 0, nullptr, false);
			Object = static_cast<T*>(BaseObject);
		}
		return Object;
	}

	template <typename _Ot = void*>
	__forceinline static void Exec(const char* _Name, void* _Detour, _Ot& _Orig = nullptrForHook) {
		auto _Fn = StaticFindObject<UFunction>(_Name);
		if (!_Fn) return;
		if (!std::is_same_v<_Ot, void*>)
			_Orig = (_Ot)_Fn->ExecFunction;
		_Fn->ExecFunction = reinterpret_cast<UFunction::FNativeFuncPtr>(_Detour);
	}

	struct FActorSpawnParameters
	{
	public:
		FName Name;

		AActor* Template;
		AActor* Owner;
		APawn* Instigator;
		ULevel* OverrideLevel;
		ESpawnActorCollisionHandlingMethod SpawnCollisionHandlingOverride;

	private:
		uint8	bRemoteOwned : 1;
	public:
		uint8	bNoFail : 1;
		uint8	bDeferConstruction : 1;
		uint8	bAllowDuringConstructionScript : 1;
		EObjectFlags ObjectFlags;
	};

	template <class T>
	T* SpawnActorV3(FVector Location, FRotator Rotation = FRotator{ 0, 0, 0 }, UClass* Class = T::StaticClass(), AActor* Owner = nullptr, FVector Scale3D = { 1,1,1 })
	{
		FActorSpawnParameters addr{};

		addr.Owner = Owner;
		addr.bDeferConstruction = false;
		addr.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		FQuat Quat{};
		FTransform Transform{};

		auto DEG_TO_RAD = 3.14159 / 180;
		auto DIVIDE_BY_2 = DEG_TO_RAD / 2;

		auto SP = sin(Rotation.Pitch * DIVIDE_BY_2);
		auto CP = cos(Rotation.Pitch * DIVIDE_BY_2);

		auto SY = sin(Rotation.Yaw * DIVIDE_BY_2);
		auto CY = cos(Rotation.Yaw * DIVIDE_BY_2);

		auto SR = sin(Rotation.Roll * DIVIDE_BY_2);
		auto CR = cos(Rotation.Roll * DIVIDE_BY_2);

		Quat.X = CR * SP * SY - SR * CP * CY;
		Quat.Y = -CR * SP * CY - SR * CP * SY;
		Quat.Z = CR * CP * SY - SR * SP * CY;
		Quat.W = CR * CP * CY + SR * SP * SY;

		Transform.Rotation = Quat;
		Transform.Scale3D = Scale3D;
		Transform.Translation = Location;

		auto Actor = ((AActor * (*)(UWorld*, UClass*, FTransform const*, FActorSpawnParameters*))(Runtime::Offsets::ImageBase + 0x315a830))(UWorld::GetWorld(), Class, &Transform, &addr);
		return (T*)Actor;
	}

	template <class T>
	T* SpawnActorV2(FVector Location, FRotator Rotation = FRotator{ 0, 0, 0 }, UClass* Class = T::StaticClass(), AActor* Owner = nullptr, FVector Scale3D = { 1,1,1 })
	{
		FQuat Quat{};
		FTransform Transform{};

		auto DEG_TO_RAD = 3.14159 / 180;
		auto DIVIDE_BY_2 = DEG_TO_RAD / 2;

		auto SP = sin(Rotation.Pitch * DIVIDE_BY_2);
		auto CP = cos(Rotation.Pitch * DIVIDE_BY_2);

		auto SY = sin(Rotation.Yaw * DIVIDE_BY_2);
		auto CY = cos(Rotation.Yaw * DIVIDE_BY_2);

		auto SR = sin(Rotation.Roll * DIVIDE_BY_2);
		auto CR = cos(Rotation.Roll * DIVIDE_BY_2);

		Quat.X = CR * SP * SY - SR * CP * CY;
		Quat.Y = -CR * SP * CY - SR * CP * SY;
		Quat.Z = CR * CP * SY - SR * SP * CY;
		Quat.W = CR * CP * CY + SR * SP * SY;

		Transform.Rotation = Quat;
		Transform.Scale3D = Scale3D;
		Transform.Translation = Location;

		auto Actor = UGameplayStatics::BeginSpawningActorFromClass(UWorld::GetWorld(), Class, Transform, false, Owner);
		if (Actor)
			UGameplayStatics::FinishSpawningActor(Actor, Transform);
		return (T*)Actor;
	}

	__forceinline static void HookVFT(void** _Vt, uint32_t _Ind, void* _Detour)
	{
		DWORD _Vo;
		VirtualProtect(_Vt + _Ind, 8, PAGE_EXECUTE_READWRITE, &_Vo);
		_Vt[_Ind] = _Detour;
		VirtualProtect(_Vt + _Ind, 8, _Vo, &_Vo);
	}

	template <class T>
	T* SpawnActor(FVector Location, FRotator Rotation = FRotator{ 0, 0, 0 }, UClass* Class = T::StaticClass(), FVector Scale3D = { 1,1,1 })
	{
		FQuat Quat{};
		FTransform Transform{};

		auto DEG_TO_RAD = 3.14159 / 180;
		auto DIVIDE_BY_2 = DEG_TO_RAD / 2;

		auto SP = sin(Rotation.Pitch * DIVIDE_BY_2);
		auto CP = cos(Rotation.Pitch * DIVIDE_BY_2);

		auto SY = sin(Rotation.Yaw * DIVIDE_BY_2);
		auto CY = cos(Rotation.Yaw * DIVIDE_BY_2);

		auto SR = sin(Rotation.Roll * DIVIDE_BY_2);
		auto CR = cos(Rotation.Roll * DIVIDE_BY_2);

		Quat.X = CR * SP * SY - SR * CP * CY;
		Quat.Y = -CR * SP * CY - SR * CP * SY;
		Quat.Z = CR * CP * SY - SR * SP * CY;
		Quat.W = CR * CP * CY + SR * SP * SY;

		Transform.Rotation = Quat;
		Transform.Scale3D = Scale3D;
		Transform.Translation = Location;

		auto Actor = UGameplayStatics::BeginSpawningActorFromClass(UWorld::GetWorld(), Class, Transform, false, nullptr);
		if (Actor) UGameplayStatics::FinishSpawningActor(Actor, Transform);
		return (T*)Actor;
	}

	template <class T>
	T* SpawnActor(FTransform Transform = {}, UClass* Class = T::StaticClass(), FVector Scale3D = { 1,1,1 })
	{
		FQuat Quat{};

		auto Actor = UGameplayStatics::BeginSpawningActorFromClass(UWorld::GetWorld(), Class, Transform, false, nullptr);
		if (Actor) UGameplayStatics::FinishSpawningActor(Actor, Transform);
		return (T*)Actor;
	}

	__forceinline float ScalableFloat(FScalableFloat& Float, float LookupValue)
	{
		if (!Float.Curve.CurveTable) return Float.Value;
		float O;
		UDataTableFunctionLibrary::EvaluateCurveTableRow(Float.Curve.CurveTable, Float.Curve.RowName, LookupValue, nullptr, &O, FString());
		return O;
	}

	__forceinline TArray<AActor*> GetAll(UClass* Class)
	{
		TArray<AActor*> ret;
		UGameplayStatics::GetAllActorsOfClass(UWorld::GetWorld(), Class, &ret);
		return ret;
	}

	template <typename _At = AActor>
	__forceinline static TArray<_At*> GetAll(UClass* Class)
	{
		return GetAll(Class);
	}

	template <typename _At = AActor>
	__forceinline static TArray<_At*> GetAll()
	{
		TArray<_At*> Result;
		for (AActor* Actor : GetAll(AActor::StaticClass()))
		{
			if (auto CastedActor = Actor->Cast<_At>())
			{
				Result.Add(CastedActor);
			}
		}
		return Result;
	}

	template <typename _Ct>
	__forceinline static void Every(uint32_t Ind, void* Detour) {
		for (int i = 0; i < UObject::GObjects->Num(); i++) {
			auto Obj = UObject::GObjects->GetByIndex(i);
			if (Obj && Obj->IsA(_Ct::StaticClass())) {
				HookVFT(Obj->VTable, Ind, Detour);
			}
		}
	}

	float EvaluateScalableFloat(FScalableFloat& Float);
}

static std::string ToMultiByte(const UC::xstring& str)
{
	return std::string(str.begin(), str.end());
}

static std::wstring ToWString(const UC::xstring& str)
{
	std::string narrow(str.begin(), str.end());
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), -1, nullptr, 0);
	std::wstring wide(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), -1, wide.data(), size_needed);
	return wide;
}

struct FParseConditionResult
{
	bool bMatch;
	size_t NextStart;
};

static FParseConditionResult ParseCondition(UC::xstring Condition, const FGameplayTagContainer& TargetTags, const FGameplayTagContainer& SourceTags, const FGameplayTagContainer& ContextTags)
{
	size_t CondTypeStart = -1, CondTypeEnd = -1, NextCond = -1;
	for (auto& c : Condition)
	{
		if (c == '>' || c == '<' || c == '=' || c == '&')
		{
			CondTypeStart = static_cast<size_t>(&c - Condition.data());
		}
		else if (CondTypeStart != -1 && (c != '>' && c != '<' && c != '=' && c != '&'))
		{
			CondTypeEnd = static_cast<size_t>(&c - Condition.data());
		}
		else if (CondTypeEnd != -1 && (c == '=' || c == '&'))
		{
			NextCond = static_cast<size_t>(&c - Condition.data());
		}
		if (CondTypeStart != -1 && CondTypeEnd != -1 && NextCond != -1) break;
	}
	if (CondTypeStart == UC::xstring::npos)
	{
		CondTypeStart = Condition.find(" ");
		if (CondTypeStart == UC::xstring::npos)
		{
			return { false, NextCond };
		}
		CondTypeStart++;
		if (CondTypeEnd == UC::xstring::npos)
			CondTypeEnd = Condition.find(" ", CondTypeStart);
		NextCond = Condition.find("&&", CondTypeEnd + 1);
		if (NextCond == UC::xstring::npos)
			NextCond = Condition.find("||", CondTypeEnd + 1);
	}
	else if (CondTypeEnd == UC::xstring::npos)
	{
		CondTypeEnd = CondTypeStart + 1;
	}
	auto Left = Condition.substr(0, CondTypeStart - 1);
	auto CondType = Condition.substr(CondTypeStart, CondTypeEnd - CondTypeStart);
	auto Right = Condition.substr(CondTypeEnd + 1, NextCond == UC::xstring::npos ? NextCond : (Condition.substr(NextCond - 1, 1) == " " ? NextCond - 1 : NextCond) - CondTypeEnd - 1);

	if (ToMultiByte(CondType) == "HasTag" || ToMultiByte(CondType) == "MissingTag")
	{
		FGameplayTagContainer Container;
		std::string LeftStr = ToMultiByte(Left);
		if (LeftStr == "Target.Tags")
		{
			Container = TargetTags;
		}
		else if (LeftStr == "Source.Tags")
		{
			Container = SourceTags;
		}
		else if (LeftStr == "Context.Tags")
		{
			Container = ContextTags;
		}
		else
		{
			return { false, NextCond };
		}

		FGameplayTag Tag(FName(ToWString(Right).c_str()));

		if (ToMultiByte(CondType) == "HasTag")
		{
			bool hasTag = Container.HasTag(Tag);
			return { hasTag, NextCond };
		}
		else
		{
			bool missingTag = !Container.HasTag(Tag);
			return { missingTag, NextCond };
		}
	}

	return { false, NextCond };
}

static bool IsConditionMet(const UC::FString& InCondition, const FGameplayTagContainer& TargetTags, const FGameplayTagContainer& SourceTags, const FGameplayTagContainer& ContextTags)
{
	std::string TempCondition = InCondition.ToString();
	UC::xstring Condition(TempCondition.begin(), TempCondition.end());
	if (Condition.empty())
	{
		return true;
	}

	FParseConditionResult Result = ParseCondition(Condition, TargetTags, SourceTags, ContextTags);

	if (Result.NextStart != UC::xstring::npos)
	{
	loop:
		if (Result.NextStart == UC::xstring::npos)
		{
			return Result.bMatch;
		}
		auto Start = Condition.substr(Result.NextStart, 2);
		if (ToMultiByte(Start) == "&&")
		{
			Condition = Condition.substr(Result.NextStart + 2);
			if (Condition.substr(0, 1) == " ")
				Condition = Condition.substr(1);
			auto LastResult = Result;
			Result = ParseCondition(Condition, TargetTags, SourceTags, ContextTags);
			if (!LastResult.bMatch || !Result.bMatch)
				Result.bMatch = false;
			goto loop;
		}

		else if (ToMultiByte(Start) == "||")
		{
			Condition = Condition.substr(Result.NextStart + 2);
			if (Condition.substr(0, 1) == " ")
				Condition = Condition.substr(1);
			auto LastResult = Result;
			Result = ParseCondition(Condition, TargetTags, SourceTags, ContextTags);
			if (LastResult.bMatch || Result.bMatch)
				Result.bMatch = true;
			goto loop;
		}
		else
		{
			return Result.bMatch;
		}
	}

	return Result.bMatch;
}


class FOutputDevice
{
public:
	bool bSuppressEventTag;
	bool bAutoEmitLineTerminator;
	uint8_t _Padding1[0x6];
};

class FFrame : public FOutputDevice
{
public:
	void** VTable;
	UFunction* Node;
	UObject* Object;
	uint8* Code;
	uint8* Locals;
	void* MostRecentProperty;
	uint8_t* MostRecentPropertyAddress;
	uint8_t _Padding1[0x40];
	UField* PropertyChainForCompiledIn;

public:
	inline void StepCompiledIn(void* const Result, bool ForceExplicitProp = false)
	{
		if (Code && !ForceExplicitProp)
		{
			((void (*)(FFrame*, UObject*, void* const))(Runtime::Offsets::ImageBase + 0x22fcb50))(this, Object, Result);
		}
		else
		{
			UField* _Prop = PropertyChainForCompiledIn;
			PropertyChainForCompiledIn = _Prop->Next;
			((void (*)(FFrame*, void* const, UField*))(Runtime::Offsets::ImageBase + 0x22fcb80))(this, Result, _Prop);
		}
	}

	template <typename T>
	inline T& StepCompiledInRef() {
		T TempVal{};
		MostRecentPropertyAddress = nullptr;

		if (Code)
		{
			((void (*)(FFrame*, UObject*, void* const))(Runtime::Offsets::ImageBase + 0x22fcb50))(this, Object, &TempVal);
		}
		else
		{
			UField* _Prop = PropertyChainForCompiledIn;
			PropertyChainForCompiledIn = _Prop->Next;
			((void (*)(FFrame*, void* const, UField*))(Runtime::Offsets::ImageBase + 0x22fcb80))(this, &TempVal, _Prop);
		}

		return MostRecentPropertyAddress ? *(T*)MostRecentPropertyAddress : TempVal;
	}

	inline void IncrementCode()
	{
		Code = (uint8_t*)(__int64(Code) + (bool)Code);
	}
};

#define DefHookOg(_Rt, _Name, ...) static inline _Rt (*_Name##OG)(##__VA_ARGS__); static _Rt _Name(##__VA_ARGS__); 
#define callOG(_Tr, _Pt, _Th, ...) ([&](){ auto _Fn = Runtime::StaticFindObject<UFunction>(_Pt "." # _Th); _Fn->ExecFunction = (UFunction::FNativeFuncPtr) _Th##OG; _Tr->_Th(##__VA_ARGS__); _Fn->ExecFunction = (UFunction::FNativeFuncPtr) _Th; })()
#define __runOnce(_V) static uint32_t _ROnce_##_V = 0; if (++_ROnce_##_V == 1)
#define _runOnce(_V) __runOnce(_V)
#define HookOG(ret, name, args) using name##OG_t = ret(*)args; inline static name##OG_t name##OG; static ret name args;
#define runOnce _runOnce(__COUNTER__)
#define callOGWithRet(_Tr, _Pt, _Th, ...) ([&](){ auto _Fn = Runtime::StaticFindObject<UFunction>(_Pt "." # _Th); _Fn->ExecFunction = (UFunction::FNativeFuncPtr) _Th##OG; auto _Rt = _Tr->_Th(##__VA_ARGS__); _Fn->ExecFunction = (UFunction::FNativeFuncPtr) _Th; return _Rt; })()