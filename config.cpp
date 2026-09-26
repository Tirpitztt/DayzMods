
class CfgPatches
{
	class MyFactionArmbands
	{
		units[] = {"BandsArmband", "CopsArmband","WhiteFactionContract", "RedFactionContract","WhiteFactionNPC","WhiteFactionNPC"};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data", "DZ_Scripts", "DZ_Gear_Clothing", "DZ_Gear_Consumables"}; // Зависимость от одежды (Clothing)
	};
};

class CfgVehicles
{
	class Armband_White;
	class Armband_Red;
	class SurvivorM_Mirek;
	
	// Наша кастомная белая повязка
	class BandsArmband: Armband_White
	{
		scope = 2; // Доступна в админке / спавне
		displayName = "Бандитская повязка";
		descriptionShort = "Цвет повязки означает принадлежность к фракции";
	};

	// Наша кастомная красная повязка
	class CopsArmband: Armband_Red
	{
		scope = 2;
		displayName = "Ментовская повязка";
		descriptionShort = "Цвет повязки означает принадлежность к фракции";
	};
	class WhiteFactionContract: Paper
	{
		scope = 2; // Можно спавнить в игре
		displayName = "Контракт фракции Бандиты";
		descriptionShort = "Официальный документ. Подпишите, чтобы официально вступить в Бандиты.";
	};

	class RedFactionContract: Paper
	{
		scope = 2;
		displayName = "Контракт фракции Менты";
		descriptionShort = "Официальный документ. Подпишите, чтобы официально вступить во Менты.";
	};
	class WhiteFactionNPC: SurvivorM_Mirek
	{
		scope = 2;
		displayName = "Интендант фракции БЕЛЫЕ";
		descriptionShort = "Офицер снабжения. Поговорите с ним, чтобы получить контракт.";
	};

	class RedFactionNPC: SurvivorM_Mirek
	{
		scope = 2;
		displayName = "Интендант фракции КРАСНЫЕ";
		descriptionShort = "Офицер снабжения. Поговорите с ним, чтобы получить контракт.";
	};
};

class CfgMods
{
	class MyFactionArmbands
	{
		dir = "testSpawnMod";
		name = "Fraction Armbands Respawn Mod";
		credits = "tirpitz";
		author = "tirpitz";
		type = "mod";

		dependencies[] = {"World"};

		class CfgScriptModule
		{
			class worldScriptModule
			{
				value = "";
				files[] = {"testSpawnMod/scripts/4_World"};
			};
		};
	};
};