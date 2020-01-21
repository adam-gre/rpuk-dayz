class MouthRag extends Clothing
{
	bool m_IncomingLambdaChange;
	
	void MouthRag()
	{
		m_IncomingLambdaChange = false;
	}
	
	void ~MouthRag()
	{
		PlayerBase player;
		Class.CastTo(player, GetHierarchyRootPlayer());
		if( player )
		{
			MutePlayer(player,false);
		}
		
	}
	
	override bool CanDetachAttachment(EntityAI parent)
	{
		return m_IncomingLambdaChange;
	}
	
	override void OnWasAttached(EntityAI parent, int slot_id)
	{
		super.OnWasAttached(parent,slot_id);
		
		PlayerBase player;
		Class.CastTo(player, parent.GetHierarchyRootPlayer());

		if ( player && slot_id == InventorySlots.MASK )
		{
			MutePlayer(player,true);
		}
	}

	override void OnWasDetached(EntityAI parent, int slot_id)
	{
		super.OnWasDetached(parent,slot_id);
		
		PlayerBase player = PlayerBase.Cast(parent);
		
		if ( player )
		{
			MutePlayer(player,false);
		}
	}
	
	//! Enables/disables player's VON; false DISABLES VON
	override void MutePlayer(PlayerBase player, bool state)
	{
		if( GetGame() )
		{	
			if (( GetGame().IsServer() && GetGame().IsMultiplayer() ) || ( GetGame().GetPlayer() == player ))
			{
				GetGame().SetVoiceEffect(player, VoiceEffectMumbling, state);
			}
		}
	}
	
	override bool IsObstructingVoice()
	{
		return true;
	}
	
	void SetIncomingLambaBool(bool state)
	{
		m_IncomingLambdaChange = state;
	}
	
	bool GetIncomingLambdaBool()
	{
		return m_IncomingLambdaChange;
	}
};