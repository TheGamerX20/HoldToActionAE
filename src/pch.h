#pragma once

#include <Detours.h>
#include <F4SE/F4SE.h>
#include <RE/Fallout.h>
#include <REX/REX.h>

using namespace std::literals;

namespace RELEX
{
	class ScopeLock
	{
		bool _unlocked;
		std::uint32_t _old;
		std::uintptr_t _target, _size;

		ScopeLock(const ScopeLock&) = delete;
		ScopeLock operator=(const ScopeLock&) = delete;
	public:
		ScopeLock(std::uintptr_t a_target, std::uintptr_t a_size) noexcept : _unlocked(false), _old(0), _target(a_target), _size(a_size)
		{
			_unlocked = REX::W32::VirtualProtect(reinterpret_cast<void*>(a_target), (std::size_t)a_size, REX::W32::PAGE_EXECUTE_READWRITE, (std::uint32_t*)&_old);
		}

		ScopeLock(void* a_target, std::uintptr_t a_size) noexcept : _unlocked(false), _old(0), _target((std::uintptr_t)a_target), _size(a_size)
		{
			_unlocked = REX::W32::VirtualProtect(a_target, (std::size_t)a_size, REX::W32::PAGE_EXECUTE_READWRITE, (std::uint32_t*)&_old);
		}

		~ScopeLock() noexcept
		{
			if (_unlocked)
			{
				REX::W32::VirtualProtect(reinterpret_cast<void*>(_target), (std::size_t)_size, _old, (std::uint32_t*)&_old);
				REX::W32::FlushInstructionCache(REX::W32::GetCurrentProcess(), (void*)_target, _size);
				_unlocked = false;
			}
		}

		[[nodiscard]] inline virtual bool HasUnlocked() const noexcept(true) { return _unlocked; }
	};

	inline void Write(std::uintptr_t a_target, const std::initializer_list<std::uint8_t>& a_data) noexcept
	{
		if (!a_target || !a_data.size()) return;
		REL::Write(a_target, (const void*)a_data.begin(), a_data.size());
	}

	inline void WriteNop(std::uintptr_t a_target, std::size_t a_size) noexcept
	{
		if (!a_target || !a_size) return;
		std::fill_n(reinterpret_cast<std::uint8_t*>(a_target), a_size, REL::NOP);
	}

	inline void WriteSafe(std::uintptr_t a_target, const std::initializer_list<std::uint8_t>& a_data) noexcept
	{
		if (!a_target || !a_data.size()) return;
		ScopeLock Lock(a_target, a_data.size());
		if (Lock.HasUnlocked())
			REL::Write(a_target, (const void*)a_data.begin(), a_data.size());
	}

	inline void WriteSafeNop(std::uintptr_t a_target, std::size_t a_size) noexcept
	{
		if (!a_target || !a_size) return;
		ScopeLock Lock(a_target, a_size);
		if (Lock.HasUnlocked())
			std::fill_n(reinterpret_cast<std::uint8_t*>(a_target), a_size, REL::NOP);
	}

	inline bool IsRuntimeOG() noexcept
	{
		return REX::FModule::IsRuntimeOG();
	}

	inline bool IsRuntimeNG() noexcept
	{
		return REX::FModule::IsRuntimeNG();
	}

	inline bool IsRuntimeAE() noexcept
	{
		return REX::FModule::IsRuntimeAE();
	}

	static std::uintptr_t DetourCheckRel32AndReturnDestAddress(std::uintptr_t a_target) noexcept
	{
		auto target = reinterpret_cast<const uint8_t*>(a_target);
		if ((target[0] == 0xE8) || (target[0] == 0xE9))
		{
			auto rel32 = reinterpret_cast<const int32_t*>(a_target + 1);
			return a_target + *rel32 + 5;
		}
		else if ((target[0] == 0xFF) && ((target[1] == 0x15) || (target[1] == 0x25)))
		{
			auto rel32 = reinterpret_cast<const int32_t*>(a_target + 2);
			return *reinterpret_cast<std::uintptr_t*>(a_target + *rel32 + 6);
		}

		return 0;
	}

	inline std::uintptr_t DetourJump(std::uintptr_t a_target, std::uintptr_t a_function) noexcept
	{
		auto destAddress = DetourCheckRel32AndReturnDestAddress(a_target);
		auto detourAddress = Detours::X64::DetourFunction(a_target, a_function, Detours::X64Option::USE_REL32_JUMP);
		return destAddress ? destAddress : detourAddress;
	}

	inline std::uintptr_t DetourCall(std::uintptr_t a_target, std::uintptr_t a_function) noexcept
	{
		auto destAddress = DetourCheckRel32AndReturnDestAddress(a_target);
		auto detourAddress = Detours::X64::DetourFunction(a_target, a_function, Detours::X64Option::USE_REL32_CALL);
		return destAddress ? destAddress : detourAddress;
	}

	inline std::uintptr_t DetourJump(const REL::Relocation<>& a_target, std::uintptr_t a_function) noexcept
	{
		return DetourJump(a_target.address(), a_function);
	}

	inline std::uintptr_t DetourCall(const REL::Relocation<>& a_target, std::uintptr_t a_function) noexcept
	{
		return DetourCall(a_target.address(), a_function);
	}

	template<class T>
	inline static std::uintptr_t DetourClassJump(std::uintptr_t a_target, T a_function) noexcept
	{
		return DetourJump(a_target, *(std::uintptr_t*)&a_function);
	}

	template<class T>
	inline static std::uintptr_t DetourClassCall(std::uintptr_t a_target, T a_function) noexcept
	{
		return DetourCall(a_target, *(std::uintptr_t*)&a_function);
	}

	template<class T>
	inline static std::uintptr_t DetourClassJump(const REL::Relocation<>& a_target, T a_function) noexcept
	{
		return DetourJump(a_target, *(std::uintptr_t*)&a_function);
	}

	template<class T>
	inline static std::uintptr_t DetourClassCall(const REL::Relocation<>& a_target, T a_function) noexcept
	{
		return DetourCall(a_target, *(std::uintptr_t*)&a_function);
	}

	template<class T, class... _Types>
	inline static std::uintptr_t XbyakJump(std::uintptr_t a_target, _Types&&... _Args) noexcept
	{
		auto patch = new T(_Args...);
		return DetourJump(a_target, (std::uintptr_t)patch->getCode());
	}

	template<class T, class... _Types>
	inline static std::uintptr_t XbyakCall(std::uintptr_t a_target, _Types&&... _Args) noexcept
	{
		auto patch = new T(_Args...);
		return DetourCall(a_target, (std::uintptr_t)patch->getCode());
	}

	inline std::uintptr_t DetourVTable(std::uintptr_t a_target, std::uintptr_t a_function, uint32_t a_index) noexcept
	{
		return Detours::X64::DetourVTable(a_target, a_function, a_index);
	}

	inline std::uintptr_t DetourIAT(const char* a_importModule, const char* a_functionName, std::uintptr_t a_function) noexcept
	{
		return Detours::IATHook(REX::FModule::GetExecutingModule().GetBaseAddress(), a_importModule, a_functionName, a_function);
	}

	inline std::uintptr_t DetourIAT(std::uintptr_t a_targetModule, const char* a_importModule, const char* a_functionName, std::uintptr_t a_function) noexcept
	{
		return Detours::IATHook(a_targetModule, a_importModule, a_functionName, a_function);
	}

	inline std::uintptr_t DetourIATDelayed(const char* a_importModule, const char* a_functionName, std::uintptr_t a_function) noexcept
	{
		return Detours::IATDelayedHook(REX::FModule::GetExecutingModule().GetBaseAddress(), a_importModule, a_functionName, a_function);
	}

	inline std::uintptr_t DetourIATDelayed(std::uintptr_t a_targetModule, const char* a_importModule, const char* a_functionName, std::uintptr_t a_function) noexcept
	{
		return Detours::IATDelayedHook(a_targetModule, a_importModule, a_functionName, a_function);
	}

	inline bool Validate(std::uintptr_t a_target, const std::initializer_list<uint8_t>& a_expected) noexcept
	{
		return !std::memcmp(reinterpret_cast<const void*>(a_target), a_expected.begin(), a_expected.size());
	}

	[[nodiscard]] __forceinline bool Validate(const REL::Relocation<>& a_target, const std::initializer_list<uint8_t>& a_expected) noexcept
	{
		return Validate(a_target.address(), a_expected);
	}

	std::uintptr_t TryDetourJump(std::uintptr_t a_target, std::uintptr_t a_function, const std::initializer_list<uint8_t>& a_expected) noexcept
	{
		return Validate(a_target, a_expected) ? DetourJump(a_target, a_function) : 0;
	}

	std::uintptr_t TryDetourCall(std::uintptr_t a_target, std::uintptr_t a_function, const std::initializer_list<uint8_t>& a_expected) noexcept
	{
		return Validate(a_target, a_expected) ? DetourCall(a_target, a_function) : 0;
	}

	template<typename T>
	[[nodiscard]] inline static std::uintptr_t DetourClassVTable(std::uintptr_t a_target, T a_function, uint32_t a_index) noexcept
	{
		return DetourVTable(a_target, *(std::uintptr_t*)&a_function, a_index);
	}
}
