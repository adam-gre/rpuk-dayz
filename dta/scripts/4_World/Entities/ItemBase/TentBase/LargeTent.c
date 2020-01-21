enum SoundTypeTent
{
	REPACK		= 1,
	NONE		= 0,	
}

class LargeTent extends TentBase
{
	ref protected EffectSound 	m_RepackingLoopSound;	
	
	void LargeTent()
	{		
		m_ToggleAnimations.Insert( new ToggleAnimations("EntranceO", "EntranceC"), 0 );
		m_ToggleAnimations.Insert( new ToggleAnimations("Window1O", "Window1C"), 0 );
		m_ToggleAnimations.Insert( new ToggleAnimations("Window2O", "Window2C"), 0 );
		m_ToggleAnimations.Insert( new ToggleAnimations("Window3O", "Window3C"), 0 );
		m_ToggleAnimations.Insert( new ToggleAnimations("Window4O", "Window4C"), 0 );
		m_ToggleAnimations.Insert( new ToggleAnimations("Window5O", "Window5C"), 0 );
		m_ToggleAnimations.Insert( new ToggleAnimations("Window6O", "Window6C"), 0 );
		m_ToggleAnimations.Insert( new ToggleAnimations("Window7O", "Window7C"), 0 );
			
		m_ShowAnimationsWhenPitched.Insert( "Body" );
		m_ShowAnimationsWhenPitched.Insert( "EntranceO" );
		m_ShowAnimationsWhenPitched.Insert( "Window1O" );
		m_ShowAnimationsWhenPitched.Insert( "Window2O" );
		m_ShowAnimationsWhenPitched.Insert( "Window3O" );
		m_ShowAnimationsWhenPitched.Insert( "Window4O" );
		m_ShowAnimationsWhenPitched.Insert( "Window5O" );
		m_ShowAnimationsWhenPitched.Insert( "Window6O" );
		m_ShowAnimationsWhenPitched.Insert( "Window7O" );
		m_ShowAnimationsWhenPitched.Insert( "Pack" );
		
		m_ShowAnimationsWhenPacked.Insert( "Inventory" );
		
		m_RepackingLoopSound  = new EffectSound;	
	}
	
	void ~LargeTent()
	{		
		SEffectManager.DestroySound( m_RepackingLoopSound );		
	}
	
	override void OnRPC(PlayerIdentity sender, int rpc_type,ParamsReadContext  ctx) 
	{
		super.OnRPC(sender, rpc_type, ctx);
		
		ref Param1<bool> p = new Param1<bool>(false);
				
		if (ctx.Read(p))
		{
			bool play = p.param1;
		}
		
		switch(rpc_type)
		{
			case SoundTypeTent.REPACK:
			
				if ( play )
				{
					PlayRepackingLoopSound();
				}
				
				if ( !play )
				{
					StopRepackingLoopSound();
				}
			
			break;
		}
	}
	
	void PlayRepackingLoopSound()
	{
		if ( !m_RepackingLoopSound.IsSoundPlaying() )
		{
			m_RepackingLoopSound = SEffectManager.PlaySound( "largetent_deploy_SoundSet", GetPosition(), 0.5, 0.5 );
		}
	}
	
	void StopRepackingLoopSound()
	{
		m_RepackingLoopSound.SetSoundFadeOut(0.5);
		m_RepackingLoopSound.SoundStop();
	}
	
	override void EEInit()
	{		
		super.EEInit();
	}
	
	override void OnItemLocationChanged(EntityAI old_owner, EntityAI new_owner)
	{		
		super.OnItemLocationChanged(old_owner, new_owner);
	}
	
	override string GetSoundOpen()
	{
		return "LargeTent_Door_Open_SoundSet";
	}
	
	override string GetSoundClose()
	{
		return "LargeTent_Door_Close_SoundSet";
	}
	
	override string GetSoundOpenWindow()
	{
		return "LargeTent_Window_Open_SoundSet";
	}
	
	override string GetSoundCloseWindow()
	{
		return "LargeTent_Window_Close_SoundSet";
	}
		
	override bool HasClutterCutter()
	{
		return true;
	}
	
	override string GetClutterCutter()
	{
		return "LargeTentClutterCutter";
	}
	
	//================================================================
	// ADVANCED PLACEMENT
	//================================================================

	override void OnPlacementComplete( Man player )
	{		
		super.OnPlacementComplete( player );
			
		PlayerBase pb = PlayerBase.Cast( player );
		
		if ( GetGame().IsServer() )
		{
			if ( !m_ClutterCutter )
			{
				m_ClutterCutter = GetGame().CreateObject( "LargeTentClutterCutter", pb.GetLocalProjectionPosition(), false );
				m_ClutterCutter.SetOrientation( pb.GetLocalProjectionOrientation() );	
			}
		}	
	}
	
	override string GetDeploySoundset()
	{
		return "placeLargeTent_SoundSet";
	}
	
	override string GetLoopDeploySoundset()
	{
		return "largetent_deploy_SoundSet";
	}
	
	override void SetActions()
	{
		super.SetActions();
		
		//AddAction(ActionRepackTent);
	}
};
