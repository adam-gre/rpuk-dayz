class ActionDigWormsCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		float time_spent;
		time_spent = UATimeSpent.DIG_WORMS;
		if (m_ActionData.m_MainItem.KindOf("Knife"))
			time_spent = time_spent * 1.2;
		
		m_ActionData.m_ActionComponent = new CAContinuousTime(time_spent);
	}
};

class ActionDigWorms: ActionContinuousBase
{
	void ActionDigWorms()
	{
		m_CallbackClass = ActionDigWormsCB;
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_DIGMANIPULATE;
		m_FullBody = true;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT;
		m_SpecialtyWeight = UASoftSkillsWeight.ROUGH_MEDIUM;
	}
	
	override void CreateConditionComponents()  
	{	
		m_ConditionItem = new CCINonRuined;
		m_ConditionTarget = new CCTSurface(UAMaxDistances.DEFAULT);
	}
	
	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
		if ( !GetGame().IsMultiplayer() || GetGame().IsClient() )
		{
			if ( !player.IsPlacingLocal() && player.IsCurrentCameraAimedAtGround() )
			{
				if ( target )
				{
					string surface_type;
					vector position;
					position = target.GetCursorHitPos();
					
					GetGame().SurfaceGetType( position[0], position[2], surface_type );
					
					if ( GetGame().IsSurfaceFertile(surface_type) )
					{
						return true;
					}
				}
			}
		
			return false;
		}
		else
		{
			return true;
		}
	}
	
	override bool ActionConditionContinue( ActionData action_data )
	{
		return true;
	}
	
	override bool SetupAction( PlayerBase player, ActionTarget target, ItemBase item, out ActionData action_data, Param extra_data = NULL )
	{	
		if( super.SetupAction( player, target, item, action_data, extra_data ) )
		{
			if ( item )
			{
				SetDiggingAnimation( item );
			}
			
			return true;
		}
		
		return false;
	}

	override bool HasTarget()
	{
		return true;
	}

	override string GetText()
	{
		return "#dig_up_worms";
	}

	override void OnFinishProgressServer( ActionData action_data )
	{	
		ItemBase worms;
		Class.CastTo(worms,  GetGame().CreateObject("Worm", action_data.m_Player.GetPosition()) );
		worms.SetQuantity(10,false);
		action_data.m_Player.GetSoftSkillsManager().AddSpecialty( m_SpecialtyWeight );
	}
	
	void SetDiggingAnimation( ItemBase item )
	{
		if (item.KindOf("Knife"))
		{
			m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_DEPLOY_1HD;
			m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
		}
		else
		{
			m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_DIGMANIPULATE;
			m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT;
		}
	}
};