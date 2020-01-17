class ActionPullBodyFromTransport: ActionInteractBase
{
	private Transport m_transport;
	private int       m_crewIdx;


	void ActionGetOutTransport()
	{
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ALL;
		m_HUDCursorIcon = "GetInDriver";
	}


	override void CreateConditionComponents()  
	{
		m_ConditionItem = new CCINone;
		m_ConditionTarget = new CCTNone;
		//m_ConditionTarget = new CCTMan(UAMaxDistances.DEFAULT,false);
	}

	override string GetText()
	{
		return "Pull out body - TEMP";
	}

	override typename GetInputType()
	{
		return ContinuousInteractActionInput;
	}
	
	override bool HasProgress()
	{
		return false;
	}

	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
 		m_transport = null;
		
		
		
		
		PlayerBase targetPlayer = PlayerBase.Cast(target.GetObject());
		
		Transport trans = Transport.Cast(target.GetObject());
		
		if(!trans)
			return false;
		
		if ( player.GetCommand_Vehicle() )
			return false;
		
		int componentIndex = target.GetComponentIndex();
		
		m_crewIdx = trans.CrewPositionIndex(componentIndex);
		if ( m_crewIdx < 0 )
			return false;

		Human crew = trans.CrewMember( m_crewIdx );
		if ( !crew )
			return false;
		
		if ( crew.IsAlive() )
			return false;
		
		//if ( !trans.CrewCanGetThrough( m_crewIdx ) )
			//return false;

		return true;
	}

	override void OnStartServer( ActionData action_data )
	{
		super.OnStartServer( action_data );
		Transport trans = Transport.Cast(action_data.m_Target.GetObject());
		
		if( trans )
		{
			int componentIndex = action_data.m_Target.GetComponentIndex();
			
			m_crewIdx = trans.CrewPositionIndex(componentIndex);
			if ( m_crewIdx < 0 )
				return;
			
			Human crew = trans.CrewMember( m_crewIdx );
			if ( !crew )
				return;
			
			trans.CrewGetOut( m_crewIdx );
			
		
		
		} 
		
		
		
		
/*		PlayerBase targetPlayer = PlayerBase.Cast(action_data.m_Target.GetObject());
		if(targetPlayer && !targetPlayer.IsAlive())
		{
			HumanCommandVehicle vehCommand = targetPlayer.GetCommand_Vehicle();
			if( vehCommand )
			{
				Transport trans = vehCommand.GetTransport();
				if ( trans )
				{
					Print("JUUUUUU");
					Car car;
					/*if ( Class.CastTo(car, trans) )
					{
						float speed = car.GetSpeedometer();
						if ( speed <= 8 )
						{
							vehCommand.GetOutVehicle();
						}
						else
						{
							vehCommand.JumpOutVehicle();
						}
					//action_data.m_Player.GetItemAccessor().HideItemInHands(false);
					//action_data.m_Player.GetItemAccessor().OnItemInHandsChanged();
					
					//GetDayZGame().GetBacklit().OnLeaveCar();									
					}
				}
			}
		}*/
	}
	
	/*override bool CanBeUsedInVehicle()
	{
		return true;
	}*/
	
	override bool IsInstant()
	{
		return true;
	}
};
