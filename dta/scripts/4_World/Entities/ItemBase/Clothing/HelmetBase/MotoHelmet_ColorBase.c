class MotoHelmet_ColorBase extends HelmetBase
{
	void ~MotoHelmet_ColorBase()
	{
		PlayerBase player;
		Class.CastTo(player, GetHierarchyRootPlayer());
		if( player )
		{
			MutePlayer(player,false);
		}
	}
	
	override bool CanPutAsAttachment( EntityAI parent )
	{
		if(!super.CanPutAsAttachment(parent)) {return false;}
		
		if ( ( GetNumberOfItems() == 0 || !parent || parent.IsMan() ) )
		{
			return true;
		}
		return false;
	}
	
	override void OnWasAttached(EntityAI parent, int slot_id)
	{
		super.OnWasAttached(parent,slot_id);
		
		PlayerBase player;
		Class.CastTo(player, parent.GetHierarchyRootPlayer());

		if ( player && slot_id == InventorySlots.HEADGEAR )
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
	
	override void MutePlayer(PlayerBase player, bool state)
	{
		if( GetGame() )
		{	
			if (( GetGame().IsServer() && GetGame().IsMultiplayer() ) || ( GetGame().GetPlayer() == player ))
			{
				GetGame().SetVoiceEffect(player, VoiceEffectObstruction, state);
			}
		}
	}
	
	override bool IsObstructingVoice()
	{
		return true;
	}
}

class MotoHelmet_Black extends MotoHelmet_ColorBase {};
class MotoHelmet_Grey extends MotoHelmet_ColorBase {};
class MotoHelmet_Green extends MotoHelmet_ColorBase {};
class MotoHelmet_Lime extends MotoHelmet_ColorBase {};
class MotoHelmet_Blue extends MotoHelmet_ColorBase {};
class MotoHelmet_Red extends MotoHelmet_ColorBase {};
class MotoHelmet_White extends MotoHelmet_ColorBase {};