class HoldBreathSoundEventBase extends PlayerSoundEventBase
{
	void HoldBreathSoundEventBase()
	{
		m_HasPriorityOverTypes = -1;
		m_SkipForControlledCharacter = true;
	}
	
	override bool CanPlay(PlayerBase player)
	{
		return true;
	}
}


class HoldBreathSoundEvent extends HoldBreathSoundEventBase
{
	void HoldBreathSoundEvent()
	{
		m_Type = EPlayerSoundEventType.GENERAL;
		m_ID = EPlayerSoundEventID.HOLD_BREATH;
		m_SoundVoiceAnimEventClassID = 20;
	}
	
	override bool HasPriorityOverCurrent(PlayerBase player, EPlayerSoundEventID other_state_id, EPlayerSoundEventType type_other)
	{
		/*
		if( other_state_id == EPlayerSoundEventID.RELEASE_BREATH)
		{
			return true;
		}
		return false;
		*/
		
		if( other_state_id == EPlayerSoundEventID.RELEASE_BREATH)
		{
			return false;
		}
		return true;
		
	}
}

class ReleaseBreathSoundEvent extends HoldBreathSoundEventBase
{
	void ReleaseBreathSoundEvent()
	{
		m_Type = EPlayerSoundEventType.GENERAL;
		m_ID = EPlayerSoundEventID.RELEASE_BREATH;
		m_SoundVoiceAnimEventClassID = 21;
	}
	
	override bool HasPriorityOverCurrent(PlayerBase player, EPlayerSoundEventID other_state_id, EPlayerSoundEventType type_other)
	{
		/*
		if( other_state_id == EPlayerSoundEventID.HOLD_BREATH)
		{
			return true;
		}
		return false;
		*/
		if( other_state_id == EPlayerSoundEventID.HOLD_BREATH)
		{
			return false;
		}
		return true;
	}
}