
// ============================================================================
// 1. ДИНАМИЧЕСКИЕ ПАРАМЕТРЫ ИГРОКА (PLAYERBASE)
// ============================================================================
modded class PlayerBase
{
	// Переменная для хранения текущей фракции (0 - нет, 1 - Белые, 2 - Красные)
	protected int m_CurrentFaction = 0; 
	
	// Точка респауна, установленная повязкой
	protected vector m_CustomRespawnPoint = Vector.Zero;

	void SetPlayerFaction(int factionID)
	{
		m_CurrentFaction = factionID;
		UpdateArmbandRespawn();
	}

	int GetPlayerFaction()
	{
		return m_CurrentFaction;
	}

	void SetCustomRespawnPoint(vector pos)
	{
		m_CustomRespawnPoint = pos;
	}

	vector GetCustomRespawnPoint()
	{
		return m_CustomRespawnPoint;
	}

	// ============================================================================
	// СИСТЕМА СОХРАНЕНИЯ ДАННЫХ (DATABASE SERIALIZATION)
	// ============================================================================

	// Срабатывает на сервере, когда игра сохраняет персонажа (при выходе игрока или рестарте)
	override void OnStoreSave(ParamsWriteContext ctx)
	{
		super.OnStoreSave(ctx);

		// Записываем ID нашей фракции в поток данных персонажа
		// Сюда можно дописать и другие переменные, если решите расширять мод
		ctx.Write(m_CurrentFaction); 
	}

	// Срабатывает на сервере, когда игрок заходит на сервер и его персонаж загружается
	override bool OnStoreLoad(ParamsReadContext ctx, int version)
	{
		if (!super.OnStoreLoad(ctx, version))
			return false;

		// Считываем ID фракции из потока данных в том же порядке, в каком записывали
		if (!ctx.Read(m_CurrentFaction))
		{
			m_CurrentFaction = 0; // Если данных нет (первый запуск мода), ставим 0
		}

		// Важно: так как инвентарь (повязка) загружается чуть позже самого игрока,
		// мы делаем микро-задержку в 1 секунду перед проверкой повязки, чтобы движок успел выдать вещи в слоты
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(UpdateArmbandRespawn, 1000, false);

		return true;
	}

	// ============================================================================
	// ЛОГИКА ПРОВЕРКИ ПОВЯЗКИ И УВЕДОМЛЕНИЙ
	// ============================================================================
	void UpdateArmbandRespawn()
	{
		if (!GetGame().IsServer()) return;
        Print("[DEBUG SPAWN] Check updateArmbandRespawn");

		EntityAI armband = FindAttachmentBySlotName("Armband");//получаем повязку из слота
		
		if (!armband)
		{
			SetCustomRespawnPoint(Vector.Zero);// если слот пустой ставим ноль и отправляем на пляж
			return;
		}
		//проверям повязку на соответствие со спец повязкой
		CustomArmbandBase customArmband = CustomArmbandBase.Cast(armband);
		if (customArmband)
		{
			//если действительно спец повязка то получаем из нее ее принадлежность
			if (GetPlayerFaction() == customArmband.GetRequiredFactionID())
			{
				//если повязка принадлежит игроку то ставим координаты базы
				SetCustomRespawnPoint(customArmband.GetRespawnPosition());
				MessageToPlayer("✅ Повязка " + customArmband.GetFactionName() + " активна. Респаун зафиксирован.");
			}
			else
			{
				//если нет то отправляем нахуй
				SetCustomRespawnPoint(Vector.Zero);
				MessageToPlayer("⚠️ Эта повязка не дает вам бонуса респауна, так как вы в другой фракции.");
			}
		}
	}

	void MessageToPlayer(string text)
	{
		Param1<string> mParam = new Param1<string>(text);
		GetGame().RPCSingleParam(this, ERPCs.RPC_USER_ACTION_MESSAGE, mParam, true, this.GetIdentity());
	}
}



// ============================================================================
// 2. ОБНОВЛЕННАЯ ЛОГИКА ПОВЯЗОК
// ============================================================================
class CustomArmbandBase : Armband_ColorBase
{
	vector GetRespawnPosition() { return Vector.Zero; }
	string GetFactionName() { return "Нет"; }
	
	// Какой ID фракции требуется для работы этой повязки (переопределяется ниже)
	int GetRequiredFactionID() { return 0; }

	// Срабатывает, когда повязку надевают
	override void EEItemIn(InventoryLocation clNewLocation)
	{
		super.EEItemIn(clNewLocation);
		
		if (!GetGame().IsServer()) return;
		//проверяем где находится повязка
		if (clNewLocation && clNewLocation.GetType() == InventoryLocationType.ATTACHMENT)
		{
			PlayerBase player = PlayerBase.Cast(clNewLocation.GetHierarchyRootPlayer());
			if (player && clNewLocation.GetSlot() == InventorySlots.ARMBAND)
			{
				// Вместо выбрасывания просто запускаем проверку соответствия
				player.UpdateArmbandRespawn();
			}
		}
	}

	// Срабатывает, когда повязку снимают
	override void EEItemOut(InventoryLocation clOldLocation)
	{
		super.EEItemOut(clOldLocation);
		
		if (!GetGame().IsServer()) return;
		
		if (clOldLocation && clOldLocation.GetType() == InventoryLocationType.ATTACHMENT)
		{
			PlayerBase player = PlayerBase.Cast(clOldLocation.GetHierarchyRootPlayer());
			if (player && clOldLocation.GetSlot() == InventorySlots.ARMBAND)
			{
				// Сбрасываем точку спавна
				player.SetCustomRespawnPoint(Vector.Zero);
			}
		}
	}
}

// Реализация БЕЛОЙ повязки
class BandsArmband : CustomArmbandBase
{
	override string GetFactionName() { return "Бандиты"; }
	override int GetRequiredFactionID() { return 1; } // Требует фракцию 1 у игрока
	override vector GetRespawnPosition() { return "4500 0 10200"; } // заменить на конкретные с проверкой Y
}

// Реализация КРАСНОЙ повязки
class CopsArmband : CustomArmbandBase
{
	override string GetFactionName() { return "Менты"; }
	override int GetRequiredFactionID() { return 2; } // Требует фракцию 2 у игрока
	override vector GetRespawnPosition() { return "11400 0 8500"; } // заменить на конкретные с проверкой Y
}


// ============================================================================
// 3. КОНТРОЛЬ СЕРВЕРНОГО РЕСПАУНА
// ============================================================================
modded class MissionServer
{
	// Координаты баз (должны точно совпадать с координатами из классов повязок)
	// Объявляем координаты баз строго как векторы (vector), а не строки!
	// Высоту (Y) здесь можно оставить 0, так как мы не будем её учитывать при сравнении.
	protected vector m_WhiteBasePos = Vector(4500, 0, 10200);
	protected vector m_RedBasePos   = Vector(11400, 0, 8500);
	// НАШИ ДЕФОЛТНЫЕ КООРДИНАТЫ (для обычных игроков без фракции)
	// Высоту (Y) можно поставить 0, так как мы её автоматически пересчитаем
	protected vector m_DefaultSpawnPos = Vector(7500, 0, 7500); // Замените на ваши X, 0, Z


	override PlayerBase CreateCharacter(PlayerIdentity identity, vector pos, ParamsReadContext ctx, string characterName)
	{
		PlayerBase oldPlayer = PlayerBase.Cast(IdentityToPlayer(identity));
		// По умолчанию ставим фиксированную дефолтную точку спавна вместо ванильного пляжа!
		vector spawnPoint = m_DefaultSpawnPos; 
		bool spawnedOnFactionBase = false;
		int factionType = 0; // 1 - Белые, 2 - Красные

		// Проверяем, была ли у игрока активна точка фракционного респауна в момент смерти
		if (oldPlayer && oldPlayer.GetCustomRespawnPoint().LengthSq() > 0 )
		{
			vector savedPoint = oldPlayer.GetCustomRespawnPoint();
			
			// 1. Сравниваем только X и Z координаты (горизонтальную плоскость), игнорируя высоту Y
			if (savedPoint[0] == m_WhiteBasePos[0] && savedPoint[2] == m_WhiteBasePos[2])
			{
				factionType = 1;
				spawnedOnFactionBase = true;
			}
			else if (savedPoint[0] == m_RedBasePos[0] && savedPoint[2] == m_RedBasePos[2])
			{
				factionType = 2;
				spawnedOnFactionBase = true;
			}
			// 2. И только ПОСЛЕ проверки фракции безопасно корректируем высоту земли для спавна,
			// чтобы игрок не провалился под текстуры
			float groundHeight = GetGame().SurfaceY(spawnPoint[0], spawnPoint[2]);
			spawnPoint[1] = groundHeight;
		}

		// Создаем персонажа на выбранной точке спавна
		PlayerBase newPlayer = super.CreateCharacter(identity, spawnPoint, ctx, characterName);

		// Если игрок успешно создался и он респаунился на базе фракции
		if (newPlayer && spawnedOnFactionBase)
		{
			// Запускаем выдачу лута с микрозадержкой, чтобы ванильный скрипт выдачи пляжного лута 
			// (StartingEquipSetup) успел отработать, и мы могли его полностью очистить.
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(GiveFactionKit, 100, false, newPlayer, factionType);
		}

		return newPlayer;
	}

	// Функция полной очистки дефолтных вещей и выдачи фракционного кита
	void GiveFactionKit(PlayerBase player, int factionID)
{
	if (!player) return;

	// 1. Полностью удаляем стандартную одежду, сливы и яблоки
	array<EntityAI> itemsArray = new array<EntityAI>;
	player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);
	
	// ОБРАТНЫЙ ЦИКЛ: Идем с конца массива (от Count()-1 до 0). 
	// Это гарантирует, что сдвиг индексов при удалении не сломает итерацию.
	for (int i = itemsArray.Count() - 1; i >= 0; i--)
	{
		EntityAI item = itemsArray.Get(i);
		// Проверяем, что объект существует, и это НЕ сам игрок
		if (item && item != player)
		{
			item.Delete();
		}
	}

	// 2. Выдаем новый лут в зависимости от фракции
	if (factionID == 1) // БЕЛЫЕ
	{
		player.GetInventory().CreateInInventory("TTSKOPants");
		player.GetInventory().CreateInInventory("TTSKOJacket");
		player.GetInventory().CreateInInventory("CombatBoots_Black");
		player.GetInventory().CreateInInventory("BandsArmband"); 
		player.GetInventory().CreateInInventory("M4A1");
		player.GetInventory().CreateInInventory("Mag_M4_30Rnd");
		player.GetInventory().CreateInInventory("TacticalBaconCan");
		player.GetInventory().CreateInInventory("Canteen");

		player.MessageToPlayer("🎁 Вам выдан комплект фракции Бандиты!");
	}
	else if (factionID == 2) // КРАСНЫЕ
	{
		player.GetInventory().CreateInInventory("GorkaPants_Autumn");
		player.GetInventory().CreateInInventory("GorkaEJacket_Autumn");
		player.GetInventory().CreateInInventory("CombatBoots_Brown");
		player.GetInventory().CreateInInventory("CopsArmband");
		player.GetInventory().CreateInInventory("AKM");
		player.GetInventory().CreateInInventory("Mag_AKM_30Rnd");
		player.GetInventory().CreateInInventory("PeachesCan");
		player.GetInventory().CreateInInventory("WaterBottle");

		player.MessageToPlayer("🎁 Вам выдан комплект фракции Менты!");
	}
}

// ОПТИМИЗИРОВАННЫЙ ПОИСК ИГРОКА (Без тяжелых циклов)
PlayerBase IdentityToPlayer(PlayerIdentity identity)
{
	if (!identity) return null;
	
	// Используем встроенный хэндлер движка, который находит игрока мгновенно по его UID
	return PlayerBase.Cast(GetGame().GetPlayerByUID(identity.GetId()));
}

}

