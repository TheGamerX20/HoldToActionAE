#pragma once

namespace HoldToAction
{
	static REX::TOML::Bool bHoldToSneak{ "Configs"sv, "bHoldToSneak"sv, true };
	static REX::TOML::Bool bHoldToSprint{ "Configs"sv, "bHoldToSprint"sv, true };

	inline void* StopSneaking(RE::Actor* a_actor)
	{
		using func_t = decltype(&StopSneaking);
		static REL::Relocation<func_t> func{ REL::ID{ 564788, 2229631 } };
		return func(a_actor);
	}

	struct SneakHandler_HandleEvent
	{
		static void thunk(void* a_a1, RE::ButtonEvent* a_event)
		{
			RE::PlayerCharacter* playerCharacter = RE::PlayerCharacter::GetSingleton();
			if (!playerCharacter)
				return func(a_a1, a_event);

			// Pressed / Held
			if (a_event->value > 0.0f)
			{
				if (!playerCharacter->IsSneaking())
				{
					if (RE::PlayerControls* playerControls = RE::PlayerControls::GetSingleton())
						playerControls->DoAction(RE::DEFAULT_OBJECT::kActionSneak, RE::ActionInput::ACTIONPRIORITY::kTry);
				}

				a_event->handled = RE::InputEvent::HANDLED_RESULT::kContinue;
				return;
			}

			// Released
			if (playerCharacter->IsSneaking())
			{
				StopSneaking(playerCharacter);
				a_event->handled = RE::InputEvent::HANDLED_RESULT::kContinue;
				return;
			}

			return func(a_a1, a_event);
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};

	struct SprintHandler_HandleEvent
	{
		static void thunk(void* a_a1, RE::ButtonEvent* a_event)
		{
			// Pressed / Held
			if (a_event->value > 0.0f)
				return func(a_a1, a_event);

			// Released
			if (RE::PlayerCharacter* playerCharacter = RE::PlayerCharacter::GetSingleton())
			{
				playerCharacter->sprintToggled = false;
				a_event->handled = RE::InputEvent::HANDLED_RESULT::kContinue;
				return;
			}

			return func(a_a1, a_event);
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};
	
	inline bool Install()
	{
		if (bHoldToSneak.GetValue())
		{
			SneakHandler_HandleEvent::func = RELEX::DetourClassJump(REL::ID{ 399406, 2234888 }.address(), &SneakHandler_HandleEvent::thunk);
			REX::INFO("Hooked the Sneak Handler.");
		}

		if (bHoldToSprint.GetValue())
		{
			SprintHandler_HandleEvent::func = RELEX::DetourClassJump(REL::ID{ 1356251, 2234889 }.address(), &SprintHandler_HandleEvent::thunk);
			REX::INFO("Hooked the Sprint Handler");
		}
		
		return true;
	}
}
