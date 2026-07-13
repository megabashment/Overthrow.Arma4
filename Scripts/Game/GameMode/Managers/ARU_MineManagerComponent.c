//! Tracks all ARU_MineComponent instances in the world and pays resistance income for
//! player-owned mines on the same 6-in-game-hour cadence as OVT_EconomyManagerComponent's
//! tax/donation income. Deliberately self-contained (own timer, own registry) so it does not
//! require modifying Overthrow's own economy/game-mode files.
class ARU_MineManagerComponentClass: OVT_ComponentClass
{
};

class ARU_MineManagerComponent: OVT_Component
{
	protected ref array<ARU_MineComponent> m_aMines = {};
	protected int m_iHourPaidIncome = -1;

	const int UPDATE_FREQUENCY = 60000; //!< ms, matches OVT_EconomyManagerComponent.ECONOMY_UPDATE_FREQUENCY

	//------------------------------------------------------------------------------------------------
	static ARU_MineManagerComponent s_Instance;

	static ARU_MineManagerComponent GetInstance()
	{
		if (!s_Instance)
		{
			BaseGameMode pGameMode = GetGame().GetGameMode();
			if (pGameMode)
				s_Instance = ARU_MineManagerComponent.Cast(pGameMode.FindComponent(ARU_MineManagerComponent));
		}
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (SCR_Global.IsEditMode())
			return;

		if (!Replication.IsServer())
			return;

		float timeMul = 6;
		OVT_TimeAndWeatherHandlerComponent tw = OVT_TimeAndWeatherHandlerComponent.Cast(GetGame().GetGameMode().FindComponent(OVT_TimeAndWeatherHandlerComponent));
		if (tw)
			timeMul = tw.GetDayTimeMultiplier();

		GetGame().GetCallqueue().CallLater(CheckUpdate, UPDATE_FREQUENCY / timeMul, true, owner);
	}

	//------------------------------------------------------------------------------------------------
	void RegisterMine(ARU_MineComponent mine)
	{
		if (m_aMines.Contains(mine))
			return;

		m_aMines.Insert(mine);
		Print(string.Format("[ARU] Mine registered at %1, daily income %2. Total mines: %3", mine.GetPosition(), mine.m_iDailyIncome, m_aMines.Count()));
	}

	//------------------------------------------------------------------------------------------------
	void UnregisterMine(ARU_MineComponent mine)
	{
		m_aMines.RemoveItem(mine);
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckUpdate()
	{
		ChimeraWorld world = GetOwner().GetWorld();
		if (!world)
			return;

		TimeAndWeatherManagerEntity timeMgr = world.GetTimeAndWeatherManager();
		if (!timeMgr)
			return;

		TimeContainer t = timeMgr.GetTime();

		bool isPayHour = (t.m_iHours == 0 || t.m_iHours == 6 || t.m_iHours == 12 || t.m_iHours == 18);
		if (!isPayHour || m_iHourPaidIncome == t.m_iHours)
			return;

		m_iHourPaidIncome = t.m_iHours;
		PayMineIncome();
	}

	//------------------------------------------------------------------------------------------------
	//! Pays 1/4 of each owned mine's daily income into the resistance treasury (called 4x/day)
	protected void PayMineIncome()
	{
		OVT_EconomyManagerComponent economy = OVT_Global.GetEconomy();
		if (!economy)
			return;

		int total = 0;
		foreach (ARU_MineComponent mine : m_aMines)
		{
			if (!mine || !mine.IsGeneratingIncome())
				continue;

			total += (int)Math.Round(mine.m_iDailyIncome / 4.0);
		}

		Print(string.Format("[ARU] PayMineIncome tick: %1 mines registered, paying %2 to resistance treasury", m_aMines.Count(), total));

		if (total > 0)
			economy.AddResistanceMoney(total);
	}
}
