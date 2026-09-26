
// ============================================================================
// 1. КАСТОМНОЕ ОДНОРАЗОВОЕ ДЕЙСТВИЕ "ПОДПИСАТЬ КОНТРАКТ"
// ============================================================================
class ActionSignContract: ActionSingleUseBase
{
	void ActionSignContract()
	{
		// Анимация чтения/рассматривания предмета в руках
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_LOOKATNOTE; 
		m_Text = "Подписать контракт фракции";
	}

	override void CreateConditionComponents()
	{
		// Контракт в руках не должен быть полностью уничтожен (Ruined)
		m_ConditionItem = new CCINonRuined(); 
		// Цель не нужна, действие совершается на себя
		m_ConditionTarget = new CCTNone();
	}

	// На клиенте можем запустить локальный звук шуршания бумаги
	override void OnExecuteClient(ActionData action_data)
	{
		super.OnExecuteClient(action_data);
		
		SEffectManager.PlaySound(" there_is_no_vanilla_paper_sound_so_we_can_use_map_sound ", action_data.m_Player.GetPosition());
	}

	// Основная логика смены фракции происходит строго на СЕРВЕРЕ
	override void OnExecuteServer(ActionData action_data)
	{
		super.OnExecuteServer(action_data);

		PlayerBase player = action_data.m_Player;
		// Получаем предмет-контракт, который игрок держит в руках
		FactionContractBase contract = FactionContractBase.Cast(action_data.m_MainItem);

		if (player && contract)
		{
			int targetFaction = contract.GetTargetFactionID();
			string factionName = contract.GetFactionName();

			// Проверяем, не состоит ли игрок уже в этой фракции
			if (player.GetPlayerFaction() == targetFaction)
			{
				player.MessageToPlayer("❌ Вы уже состоите во фракции " + factionName + "!");
				return;
			}

			// Меняем фракцию игрока (метод SetPlayerFaction автоматически обновит респаун повязки)
			player.SetPlayerFaction(targetFaction);
			player.MessageToPlayer("📝 Вы подписали контракт. Ваша новая фракция: " + factionName + "!");

			// Уничтожаем контракт после успешного подписания (одноразовое использование)
			contract.Delete();
		}
	}
}


// ============================================================================
// 2. КЛАССЫ ПРЕДМЕТОВ КОНТРАКТОВ
// ============================================================================

// Базовый класс для контрактов на базе ванильной бумаги (Paper)
class FactionContractBase : Paper
{
	// Переопределяется в дочерних классах
	int GetTargetFactionID() { return 0; }
	string GetFactionName() { return "Нет"; }

	override void SetActions()
	{
		super.SetActions();
		// Привязываем наше новое действие к этому предмету
		AddAction(ActionSignContract);
	}
}

// Контракт Белой Фракции (Имя должно совпадать с config.cpp)
class WhiteFactionContract : FactionContractBase
{
	override int GetTargetFactionID() { return 1; } // Назначает фракцию ID 1 (Белые)
	override string GetFactionName() { return "БЕЛЫЕ"; }
}

// Контракт Красной Фракции (Имя должно совпадать с config.cpp)
class RedFactionContract : FactionContractBase
{
	override int GetTargetFactionID() { return 2; } // Назначает фракцию ID 2 (Красные)
	override string GetFactionName() { return "КРАСНЫЕ"; }
}
