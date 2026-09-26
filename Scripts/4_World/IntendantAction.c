
// ============================================================================
// 1. КАСТОМНОЕ ДЕЙСТВИЕ ВЗАИМОДЕЙСТВИЯ С NPC
// ============================================================================
class ActionGetFactionContract: ActionInteractBase
{
	void ActionGetFactionContract()
	{
		// Используем анимацию разговора/взаимодействия
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
		m_Text = "Запросить контракт фракции";
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINone(); // Предмет в руках не важен
		// Условие: целью должен быть наш кастомный NPC на расстоянии до 3 метров
		m_ConditionTarget = new CCTObject(3.0); 
	}

	// Проверка: можно ли сейчас взять контракт
	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		// Проверяем, что мы смотрим именно на нашего фракционного NPC
		FactionNPCBase npc = FactionNPCBase.Cast(target.GetObject());
		if (npc && npc.IsAlive())
		{
			return true;
		}
		return false;
	}

	// Логика выдачи контракта на СЕРВЕРЕ
	override void OnExecuteServer(ActionData action_data)
	{
		super.OnExecuteServer(action_data);

		PlayerBase player = action_data.m_Player;
		FactionNPCBase npc = FactionNPCBase.Cast(action_data.m_Target.GetObject());

		if (player && npc)
		{
			string contractClass = npc.GetContractClassToGive();
			
			// Спавним контракт прямо в инвентарь игрока (в руки или в карман)
			EntityAI contract = player.GetInventory().CreateInInventory(contractClass);
			
			if (contract)
			{
				player.MessageToPlayer("📋 Интендант выдал вам " + contract.GetDisplayName() + ".");
			}
			else
			{
				// Если инвентарь забит, спавним под ноги
				player.GetInventory().CreateAnItemInSafePlayerInvertedPos(contractClass);
				player.MessageToPlayer("⚠️ Ваш инвентарь полон! Контракт упал на землю.");
			}
		}
	}
}

// ============================================================================
// 2. КЛАССЫ ФРАКЦИОННЫХ NPC
// ============================================================================

// Базовый класс для наших NPC с защитой от урона
class FactionNPCBase : SurvivorBase
{
	// Переопределяется у конкретных NPC для выдачи нужного контракта
	string GetContractClassToGive() { return "Paper"; }

	// Добавляем возможность взаимодействовать с NPC
	override void SetActions()
	{
		super.SetActions();
		// Привязываем действие получения контракта к этому NPC
		AddAction(ActionGetFactionContract);
	}

	// КРИТИЧЕСКИ ВАЖНО: Полная блокировка любого урона (бессмертие)
	override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int componentIdx, string dmgZone, string ammo, vector samplePos, float speedCoef)
	{
		// Возвращаем false — урон не рассчитывается и не наносится вообще
		return false; 
	}

	// Дополнительная защита: отключаем кровотечение и переломы
	override bool CanBleed() { return false; }
	override bool CanBeWounded() { return false; }
}

// NPC для БЕЛОЙ базы (Имя должно совпадать с config.cpp)
class WhiteFactionNPC : FactionNPCBase
{
	override string GetContractClassToGive() { return "WhiteFactionContract"; }
}

// NPC для КРАСНОЙ базы (Имя должно совпадать с config.cpp)
class RedFactionNPC : FactionNPCBase
{
	override string GetContractClassToGive() { return "RedFactionContract"; }
}
