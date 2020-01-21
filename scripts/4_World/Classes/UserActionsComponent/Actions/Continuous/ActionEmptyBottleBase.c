class ActionEmptyBottleBaseCB : ActionContinuousBaseCB
{	
	override void CreateActionComponent()
	{
		float EmptiedQuantity;// = QUANTITY_EMPTIED_PER_SEC;
		Bottle_Base bottle = Bottle_Base.Cast(m_ActionData.m_MainItem);
		if (bottle)
			EmptiedQuantity = bottle.GetLiquidEmptyRate();
		m_ActionData.m_ActionComponent = new CAContinuousEmpty(EmptiedQuantity);
	}
	
	override void OnAnimationEvent(int pEventID)	
	{
		super.OnAnimationEvent( pEventID );
				
		switch (pEventID)
		{
			case UA_ANIM_EVENT:			
				if ( !GetGame().IsMultiplayer() || GetGame().IsServer() )
				{
					Bottle_Base vessel_in_hands = Bottle_Base.Cast( m_ActionData.m_MainItem );
					Param1<bool> play = new Param1<bool>( true );
					GetGame().RPCSingleParam( vessel_in_hands, SoundTypeBottle.EMPTYING, play, true );
				}

			break;
		}
	}
	
	override void EndActionComponent()
	{
		super.EndActionComponent();
		
		if ( !GetGame().IsMultiplayer() || GetGame().IsServer() )
		{
			Bottle_Base target_vessel = Bottle_Base.Cast( m_ActionData.m_MainItem );
			Param1<bool> play = new Param1<bool>( false );
			GetGame().RPCSingleParam( target_vessel, SoundTypeBottle.EMPTYING, play, true );
		}
	}
};

class ActionEmptyBottleBase: ActionContinuousBase
{
	void ActionEmptyBottleBase()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_EMPTY_VESSEL;
		m_SpecialtyWeight = UASoftSkillsWeight.PRECISE_LOW;
		
		m_CallbackClass = ActionEmptyBottleBaseCB;
		m_FullBody = false;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
	}
	
	override void CreateConditionComponents()  
	{
		m_ConditionItem = new CCINotRuinedAndEmpty;
		m_ConditionTarget = new CCTNone;
	}

	override bool HasTarget()
	{
		return false;
	}

	override string GetText()
	{
		return "#empty";
	}

	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
		vector 	pos_cursor = target.GetCursorHitPos();
		vector 	pos_head;
		MiscGameplayFunctions.GetHeadBonePos( player, pos_head );
		float distance = vector.Distance(pos_cursor, pos_head);
				
		if ( GetGame().IsServer() && GetGame().IsMultiplayer() )
			return true;
		
		if ( item.IsLiquidPresent() && !target.GetObject() && player.IsCurrentCameraAimedAtGround() )
		{
			return true;
		}
		return false;
	}
	
	override bool ActionConditionContinue( ActionData action_data )
	{
		if (action_data.m_MainItem.GetQuantity() > action_data.m_MainItem.GetQuantityMin())
		{
			return true;
		}
		return false;
	}
	/*override void OnEndServer( ActionData action_data )
	{
		if ( action_data.m_MainItem.GetQuantity() < 0.01 )
		{
			action_data.m_MainItem.SetQuantity(0);
			action_data.m_MainItem.SetLiquidType(0);
		}
	}*/
};