class ActionWashHandsWaterCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousRepeat(UATimeSpent.WASH_HANDS);
	}
};

class ActionWashHandsWater: ActionContinuousBase
{
	void ActionWashHandsWater()
	{
		m_CallbackClass 	= ActionWashHandsWaterCB;
		m_CommandUID		= DayZPlayerConstants.CMD_ACTIONFB_WASHHANDSPOND;
		m_FullBody			= true;
		m_StanceMask		= DayZPlayerConstants.STANCEMASK_CROUCH;
	}

	override string GetText()
	{
		return "#wash_hands";
	}

	override void CreateConditionComponents()  
	{		
		m_ConditionItem = new CCINone;
		m_ConditionTarget = new CCTNone;
	}

	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
		if ( GetGame().IsMultiplayer() && GetGame().IsServer() )
		{
			return true;
		}
		
		if ( player.HasBloodyHands() && !player.GetItemInHands() )
		{	
			vector pos_cursor = target.GetCursorHitPos();
			if( (vector.Distance(player.GetPosition(), pos_cursor) < UAMaxDistances.DEFAULT) && (GetGame().SurfaceIsPond(pos_cursor[0], pos_cursor[2]) || GetGame().SurfaceIsSea(pos_cursor[0], pos_cursor[2])))
			{
				return true;
			}
		}

		return false;
	}
	
	override void OnFinishProgressServer( ActionData action_data )
	{
		PluginLifespan module_lifespan = PluginLifespan.Cast( GetPlugin( PluginLifespan ) );
		module_lifespan.UpdateBloodyHandsVisibility( action_data.m_Player, false );
	}
};