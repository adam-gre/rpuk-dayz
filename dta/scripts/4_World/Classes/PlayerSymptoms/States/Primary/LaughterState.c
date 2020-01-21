class LaughterSymptom extends SymptomBase
{
	//this is just for the Symptom parameters set-up and is called even if the Symptom doesn't execute, don't put any gameplay code in here
	override void OnInit()
	{
		m_SymptomType = SymptomTypes.PRIMARY;
		m_Priority = 0;
		m_ID = SymptomIDs.SYMPTOM_LAUGHTER;
		m_DestroyOnAnimFinish = true;
		m_SyncToClient = false;
	}
	
	//!gets called every frame
	override void OnUpdateServer(PlayerBase player, float deltatime)
	{

	}

	override void OnUpdateClient(PlayerBase player, float deltatime)
	{
		int i = 1 + 1;
	}
	
	//!gets called once on an Symptom which is being activated
	override void OnGetActivatedServer(PlayerBase player)
	{
		PlaySound(EPlayerSoundEventID.SYMPTOM_LAUGHTER);
		player.SpreadAgents();
	}

	//!gets called once on a Symptom which is being activated
	override void OnGetActivatedClient(PlayerBase player)
	{
	}

	//!only gets called once on an active Symptom that is being deactivated
	override void OnGetDeactivatedServer(PlayerBase player)
	{
		Debug.Log("OnGetDeactivated CoughSymptom called", "PlayerSymptom");
	}

	//!only gets called once on an active Symptom that is being deactivated
	override void OnGetDeactivatedClient(PlayerBase player)
	{
		Debug.Log("OnGetDeactivated CoughSymptom called", "PlayerSymptom");
	}
}
