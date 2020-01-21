class ActionDismantlePartCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime( UATimeSpent.DEFAULT_DECONSTRUCT );
	}
};

class ActionDismantlePart: ActionContinuousBase
{
	void ActionDismantlePart()
	{
		m_CallbackClass = ActionDismantlePartCB;
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_DISASSEMBLE;
		m_FullBody = true;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT;	
		
		m_SpecialtyWeight = UASoftSkillsWeight.ROUGH_HIGH;
	}
	
	override void CreateConditionComponents()  
	{	
		m_ConditionItem = new CCINonRuined;
		m_ConditionTarget = new CCTNonRuined( UAMaxDistances.BASEBUILDING );
	}
		
	override string GetText()
	{
		PlayerBase player = PlayerBase.Cast( GetGame().GetPlayer() );
		if ( player )
		{
			ConstructionActionData construction_action_data = player.GetConstructionActionData();
			ConstructionPart constrution_part = construction_action_data.GetTargetPart();
			
			if ( constrution_part )
			{
				return "#dismantle" + " " + constrution_part.GetName();
			}
		}
		
		return "";
	}

	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{	
		return DismantleCondition( player, target, item, true );
	}
	
	override bool ActionConditionContinue( ActionData action_data )
	{	
		return DismantleCondition( action_data.m_Player, action_data.m_Target, action_data.m_MainItem , false );
	}	
	
	override void OnFinishProgressServer( ActionData action_data )
	{	
		BaseBuildingBase base_building = BaseBuildingBase.Cast( action_data.m_Target.GetObject() );
		Construction construction = base_building.GetConstruction();
		ConstructionActionData construction_action_data = action_data.m_Player.GetConstructionActionData();
		ConstructionPart construction_part = construction_action_data.GetTargetPart();
		
		if ( construction.CanDismantlePart( construction_part.GetPartName(), action_data.m_MainItem ) )
		{
			//build
			construction.DismantlePartServer( action_data.m_Player, construction_part.GetPartName(), AT_DISMANTLE_PART );
			
			//add damage to tool
			action_data.m_MainItem.DecreaseHealth( UADamageApplied.DISMANTLE, false );
		}

		action_data.m_Player.GetSoftSkillsManager().AddSpecialty( m_SpecialtyWeight );
	}
		
	//setup
	override bool SetupAction( PlayerBase player, ActionTarget target, ItemBase item, out ActionData action_data, Param extra_data = NULL )
	{	
		if( super.SetupAction( player, target, item, action_data, extra_data ) )
		{
			SetBuildingAnimation( item );
			
			return true;
		}
		
		return false;
	}
	
	protected void SetBuildingAnimation( ItemBase item )
	{
		switch( item.Type() )
		{
			case Shovel:
			case FieldShovel:
				m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_DIGMANIPULATE;
				break;
			case Pliers:
				m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_INTERACT;
				break;				
			default:
				m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_DISASSEMBLE;
				break;
		}
	}	
	
	protected bool DismantleCondition( PlayerBase player, ActionTarget target, ItemBase item, bool camera_check )
	{	
		if ( player && !player.IsLeaning() )
		{
			Object target_object = target.GetObject();
			if ( target_object && target_object.CanUseConstruction() )
			{
				string part_name = target_object.GetActionComponentName( target.GetComponentIndex() );
				
				BaseBuildingBase base_building = BaseBuildingBase.Cast( target_object );
				Construction construction = base_building.GetConstruction();		
				ConstructionPart construction_part = construction.GetConstructionPartToDismantle( part_name, item );
				
				if ( construction_part )
				{
					//camera and position checks
					if ( !base_building.IsFacingPlayer( player, part_name ) && !player.GetInputController().CameraIsFreeLook() && base_building.HasProperDistance( construction_part.GetMainPartName(), player ) )
					{
						//Camera check (client-only)
						if ( camera_check )
						{
							if ( GetGame() && ( !GetGame().IsMultiplayer() || GetGame().IsClient() ) )
							{
								if ( base_building.IsFacingCamera( part_name ) )
								{
									return false;
								}
							}
						}

						ConstructionActionData construction_action_data = player.GetConstructionActionData();
						construction_action_data.SetTargetPart( construction_part );
					
						return true;
					}
				}
			}
		}
		
		return false;
	}
	
	override string GetAdminLogMessage(ActionData action_data)
	{
		return " dismantled " + action_data.m_Target.GetObject().GetDisplayName() + " with " + action_data.m_MainItem.GetDisplayName();
	}
}