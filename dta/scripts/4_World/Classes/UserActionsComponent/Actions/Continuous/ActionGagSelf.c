class ActionGagSelf: ActionContinuousBase
{	
	void ActionGagSelf()
	{
		m_CallbackClass = ActionCoverHeadSelfCB;
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_COVERHEAD_SELF;
		//m_FullBody = true;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
		m_SpecialtyWeight = UASoftSkillsWeight.ROUGH_LOW;
	}

	override void CreateConditionComponents()  
	{	
		m_ConditionItem = new CCINonRuined;
		m_ConditionTarget = new CCTSelf;
	}

	override bool HasTarget()
	{
		return false;
	}
		
	override string GetText()
	{
		return "#gag_self";
	}

	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
		if (item.GetQuantity() > 1)
			return false;
		
		if ( !IsWearingMask(player) ) 
		{
			ItemBase headgear = ItemBase.Cast(player.FindAttachmentBySlotName( "Headgear" ));
			if ( headgear )
			{		
				bool headgear_restricted;
				bool exists = headgear.ConfigIsExisting("noMask");
				headgear_restricted = headgear.ConfigGetBool( "noMask" );
				if (headgear_restricted)
				{
					return false;
				}
			}
			return true;
		}
		
		return false;
	}

	override void OnFinishProgressServer( ActionData action_data )
	{
		action_data.m_Player.GetInventory().CreateInInventory("MouthRag");
		action_data.m_MainItem.TransferModifiers(action_data.m_Player);
		action_data.m_MainItem.Delete();
		//action_data.m_Player.GetSoftSkillsManager().AddSpecialty( m_SpecialtyWeight );
	}

	bool IsWearingMask( PlayerBase player)
	{
		if ( player.GetInventory().FindAttachment(InventorySlots.MASK) )
		{
			return true;
		}
		return false;
	}
};