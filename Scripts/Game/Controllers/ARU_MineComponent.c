//! Marks a base as a mine: generates resistance income while owned by the player/resistance faction.
//! Must sit on the same entity as OVT_BaseControllerComponent + SCR_FactionAffiliationComponent
//! (built from the OVT_BaseController.et prefab so capture/QRF behavior comes for free).
class ARU_MineComponentClass: OVT_ComponentClass
{
};

class ARU_MineComponent: OVT_Component
{
	[Attribute("500", UIWidgets.EditBox, desc: "Cash paid to the resistance treasury per full game day while this mine is player-owned")]
	int m_iDailyIncome;

	protected OVT_BaseControllerComponent m_BaseController;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (SCR_Global.IsEditMode())
			return;

		if (!Replication.IsServer())
			return;

		m_BaseController = OVT_BaseControllerComponent.Cast(owner.FindComponent(OVT_BaseControllerComponent));
		if (!m_BaseController)
		{
			Print("[ARU] ARU_MineComponent requires OVT_BaseControllerComponent on the same entity!", LogLevel.ERROR);
			return;
		}

		ARU_MineManagerComponent mineManager = ARU_MineManagerComponent.GetInstance();
		if (mineManager)
			mineManager.RegisterMine(this);
	}

	//------------------------------------------------------------------------------------------------
	//! Whether this mine currently pays out (i.e. not held by the occupying faction)
	bool IsGeneratingIncome()
	{
		if (!m_BaseController)
			return false;

		return !m_BaseController.IsOccupyingFaction();
	}

	vector GetPosition()
	{
		return GetOwner().GetOrigin();
	}
}
