class ActionOpenCarDoors: ActionInteractBase
{
	string m_AnimSource = "";
	
	void ActionOpenCarDoors()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_OPENDOORFW;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ALL;
		//m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
		m_HUDCursorIcon = CursorIcons.OpenDoors;
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINone;
		m_ConditionTarget = new CCTNone;
	}

	override string GetText()
	{
		return "#open";
	}

	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{	
		CarScript car;

		//! player inside vehicle
		if( player && player.GetCommand_Vehicle() )
		{
			if ( Class.CastTo(car, player.GetCommand_Vehicle().GetTransport()) )
			{
				int crewIdx = car.CrewMemberIndex( player );

				if( car.GetCarDoorsState(car.GetDoorInvSlotNameFromSeatPos(crewIdx)) == CarDoorState.DOORS_MISSING )
					return false;

				m_AnimSource = car.GetAnimSourceFromSelection( car.GetDoorSelectionNameFromSeatPos(crewIdx) );
				if ( crewIdx < 0 || !car.CanReachDoorsFromSeat( m_AnimSource, crewIdx ) )
					return false;
	
				if ( crewIdx == 0 || crewIdx == 2 )
					m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_DRIVER_DOOR_OPEN;
	
				if ( crewIdx == 1 || crewIdx == 3 )
					m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_CODRIVER_DOOROPEN;
				
				//! is in reach, should open the door
				if ( car.GetAnimationPhase( m_AnimSource ) <= 0.5 )
					return true;
			}
		}
		else
		{
			//! reach check from outside of car
			if( !IsInReach(player, target, UAMaxDistances.DEFAULT) ) return false;

			//! player is outside of vehicle
			if ( Class.CastTo(car, target.GetParent()) )
			{
				array<string> selections = new array<string>();
				
				CarDoor carDoor = CarDoor.Cast(target.GetObject());
				if (carDoor)
				{
					carDoor.GetActionComponentNameList(target.GetComponentIndex(), selections);
					
					for (int i = 0; i < selections.Count(); i++)
					{
						m_AnimSource = car.GetAnimSourceFromSelection( selections[i]);
						if ( m_AnimSource != "" )
						{
							//! if player is in car and cannot reach doors
							m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_OPENDOORFW;
	
							//! is in reach, should open the door
							if ( car.GetAnimationPhase( m_AnimSource ) <= 0.5 )
								return true;
						}
					}
				}
			}
		}
		
		return false;
	}
	
	override void OnStartServer( ActionData action_data )
	{
		Car car;
		//! player inside vehicle
		if( action_data.m_Player && action_data.m_Player.GetCommand_Vehicle() )
		{
			car = Car.Cast(action_data.m_Player.GetCommand_Vehicle().GetTransport());
		}
		else
		{
			car = Car.Cast(action_data.m_Target.GetParent());
		}

		if( car )
		{
			car.SetAnimationPhase( m_AnimSource, 1.0);
		}
	}

	override void OnStartClient( ActionData action_data )
	{
		Car car;
		//! player inside vehicle
		if( action_data.m_Player && action_data.m_Player.GetCommand_Vehicle() )
		{
			car = Car.Cast(action_data.m_Player.GetCommand_Vehicle().GetTransport());
		}
		else
		{
			car = Car.Cast(action_data.m_Target.GetParent());
		}
	
		if( car )
		{
			car.SetAnimationPhase( m_AnimSource, 1.0);
			
			if ( !GetGame().IsMultiplayer() || GetGame().IsClient() )
				SEffectManager.PlaySound("offroad_door_open_SoundSet", car.GetPosition() );
		}
	}
	
	override bool CanBeUsedInVehicle()
	{
		return true;
	}
};