/**@class	Defines Stamina Consumer
 * @brief	Holds information about Stamina Consumer
 *
 * @param[in]	threshold	value needed to allow consume stamina
 * @param[in]	state		keeps state of the consumer non-depleted/depleted
 */
class StaminaConsumer
{
	protected float m_Threshold;
	protected bool m_State;
	
	void StaminaConsumer(float threshold, bool state)
	{
		m_Threshold = threshold;
		m_State = state;
	}
	
	bool GetState() { return m_State; }
	void SetState(bool state) { m_State = state; }
	
	float GetThreshold() { return m_Threshold; }
	void SetThreshold(float threshold) { m_Threshold = threshold; }	
}

class StaminaConsumers
{
	protected ref map<EStaminaConsumers, ref StaminaConsumer> m_StaminaConsumers;

	void StaminaConsumers()
	{
		m_StaminaConsumers = new map<EStaminaConsumers, ref StaminaConsumer>;
	}

	void RegisterConsumer(EStaminaConsumers consumer, float threshold)
	{
		if ( !m_StaminaConsumers.Contains(consumer) )
		{
			//! init of StaminaConsumer - threshold, state
			StaminaConsumer sc = new StaminaConsumer(threshold, true);
			m_StaminaConsumers.Set(consumer, sc);
		}
	}

	bool HasEnoughStaminaFor(EStaminaConsumers consumer, float curStamina, bool isDepleted)
	{
		if ( m_StaminaConsumers && m_StaminaConsumers.Contains(consumer) )
		{
			StaminaConsumer sc = m_StaminaConsumers.Get(consumer);
			
			if ( consumer != EStaminaConsumers.SPRINT )
			{
				if( (isDepleted || curStamina < sc.GetThreshold()) )
				{
					sc.SetState(false);
					return false;
				}
			}
			else
			{
				if( !isDepleted )
				{
					if ( sc.GetState() )
					{
						sc.SetState(true);
						return true;
					}
				}
				else
				{
					sc.SetState(false);
					return false;
				}
			}

			if( curStamina > sc.GetThreshold() )
			{
				sc.SetState(true);
				return true;
			}
		}

		return false;
	}
}


/**@class	Defines Stamina Modifier
 * @brief	Holds information about Stamina Modifier
 *
 * @param[in]	type	 	calculation method
 * @param[in]	minValue	min value substracted from stamina (if type > 0)
 * @param[in]	maxValue	max value substracted from stamina
 * @param[in]	cooldown	how many seconds will not be the stamina replenishing after depleting
 */
class StaminaModifier
{
	int m_Type;
	float m_MinValue, m_MaxValue, m_Cooldown;

	void StaminaModifier(int type, float min, float max, float cooldown)
	{
		m_Type = type;
		m_MinValue = min;
		m_MaxValue = max;
		m_Cooldown = cooldown;
	}
	
	int GetType() { return m_Type; }
	
	float GetMinValue() { return m_MinValue; }
	void SetMinValue(float val) { m_MinValue = val; } 
	
	float GetMaxValue() { return m_MaxValue; }
	void SetMaxValue(float val) { m_MaxValue = val; }

	float GetCooldown() { return m_Cooldown; }
	void SetCooldown(float val) { m_Cooldown = val; }
}

class StaminaModifiers
{
	const int FIXED 		= 0;
	const int RANDOMIZED 	= 1;

	protected ref map<EStaminaModifiers, ref StaminaModifier> m_StaminaModifiers;

	void StaminaModifiers()
	{
		m_StaminaModifiers = new map<EStaminaModifiers, ref StaminaModifier>;
	}

	//! register single value modifier - depletes stamina for that value
	void RegisterFixed(EStaminaModifiers modifier, float value, float cooldown = GameConstants.STAMINA_REGEN_COOLDOWN_DEPLETION)
	{	
		if ( !m_StaminaModifiers.Contains(modifier) )
		{
			//! init of StaminaModifier - type and min, max values (min is not relevant for that type)
			StaminaModifier sm = new StaminaModifier(FIXED, -1, value, cooldown);
			m_StaminaModifiers.Set(modifier, sm);
		}
	}
	
	//! register randomized modifier - stamina will be depleted by value between min and max value;
	void RegisterRandomized(EStaminaModifiers modifier, float minValue, float maxValue, float cooldown = GameConstants.STAMINA_REGEN_COOLDOWN_DEPLETION)
	{
		if ( !m_StaminaModifiers.Contains(modifier) )
		{
			//! init of StaminaModifier - type, min, max values
			StaminaModifier sm = new StaminaModifier(RANDOMIZED, minValue, maxValue, cooldown);
			m_StaminaModifiers.Set(modifier, sm);
		}
	}
	
	StaminaModifier GetModifierData(EStaminaModifiers modifier)
	{
		return m_StaminaModifiers.Get(modifier);
	}
}


class StaminaHandler
{	
	protected float 						m_PlayerLoad;
	protected float 						m_StaminaDelta;
	protected float 						m_Stamina;
	protected float 						m_StaminaCap;
	protected float							m_StaminaDepletion;
	protected float 						m_Time;
	protected ref Param3<float,float,bool>	m_StaminaParams; 
	protected ref HumanMovementState		m_State;
	protected PlayerBase					m_Player;

	protected bool 							m_Debug;
	protected bool							m_StaminaDepleted;

	protected ref Timer						m_CooldownTimer;
	protected bool							m_IsInCooldown;
	
	protected ref StaminaConsumers			m_StaminaConsumers;
	protected ref StaminaModifiers			m_StaminaModifiers;
	
	void StaminaHandler(PlayerBase player)
	{
		if ( GetGame().IsServer() && GetGame().IsMultiplayer() )
		{
			m_StaminaParams = new Param3<float,float,bool>(0,0, false);		
		}
		m_State 			= new HumanMovementState();
		m_Player 			= player;
		m_Stamina 			= GameConstants.STAMINA_MAX; 
		m_StaminaCap 		= GameConstants.STAMINA_MAX;
		m_StaminaDepletion 	= 0;
		m_Time 				= 0;
		m_StaminaDepleted	= false;
		m_CooldownTimer		= new Timer(CALL_CATEGORY_SYSTEM);
		m_IsInCooldown		= false;
		m_Debug 			= false;

		RegisterStaminaConsumers();
		RegisterStaminaModifiers();
	}

	void Update(float deltaT, int pCurrentCommandID)
	{
#ifdef DEVELOPER
		m_Debug = DiagMenu.GetBool(DiagMenuIDs.DM_CHEATS_STAMINA_DISABLE);
		if( m_Debug ) return;
#endif
		if( m_Player )
		{
			// Calculates actual max stamina based on player's load
			if ( GetGame().IsServer() )
			{
				//! gets stamina from PlayerStat
				m_Stamina = m_Player.GetStatStamina().Get();
				//! gets the actual players load
				m_PlayerLoad = m_Player.GetPlayerLoad();

				//! StaminaCap calculation starts when PlayerLoad exceeds STAMINA_WEIGHT_LIMIT_THRESHOLD
				if (m_PlayerLoad >= GameConstants.STAMINA_WEIGHT_LIMIT_THRESHOLD)
				{
					m_StaminaCap =  Math.Max((GameConstants.STAMINA_MAX - (((m_PlayerLoad - GameConstants.STAMINA_WEIGHT_LIMIT_THRESHOLD)/GameConstants.STAMINA_KG_TO_GRAMS) * GameConstants.STAMINA_KG_TO_STAMINAPERCENT_PENALTY)),GameConstants.STAMINA_MIN_CAP);
				}
				else
				{
					m_StaminaCap = GameConstants.STAMINA_MAX;
				}
			}
			
			// Calculates stamina gain/loss based on movement and load
			m_Player.GetMovementState(m_State);

			switch (pCurrentCommandID)
			{
			case DayZPlayerConstants.COMMANDID_MOVE:
				StaminaProcessor_Move(m_State);
			break;
			case DayZPlayerConstants.COMMANDID_LADDER:
				StaminaProcessor_Ladder(m_State);
			break;
			case DayZPlayerConstants.COMMANDID_SWIM:
				StaminaProcessor_Swimming(m_State);
			break;
			case DayZPlayerConstants.COMMANDID_FALL:	//! processed on event
			case DayZPlayerConstants.COMMANDID_MELEE2:  //! processed on event
			case DayZPlayerConstants.COMMANDID_CLIMB:  //! processed on event
			break;
			default:
				if( !m_IsInCooldown )
				{
					m_StaminaDelta = GameConstants.STAMINA_GAIN_IDLE_PER_SEC;
				}
			break;
			}
			
			//Sets current stamina & stores + syncs data with client
			if (m_Stamina < 0)
			{
				m_Stamina = 0;
			}
			else
			{
				m_Stamina = Math.Max(0,Math.Min((m_Stamina + m_StaminaDelta*deltaT),m_StaminaCap));
				m_Stamina = m_Stamina - m_StaminaDepletion;
			}

			if ( GetGame().IsServer() && GetGame().IsMultiplayer() )
			{
				m_Player.GetStatStamina().Set(m_Stamina);
				m_Time += deltaT;
				if ( m_Time >= GameConstants.STAMINA_SYNC_RATE )
				{
					m_Time = 0;
					SyncStamina(m_Stamina, m_StaminaCap, m_IsInCooldown);
				}
			}
			else
			{
				m_Player.SetStamina(m_Stamina, m_StaminaCap);
			}

			//! sets exhaustion look of player based on stamina level
			HumanCommandAdditives ad = m_Player.GetCommandModifier_Additives();
			float exhaustion_value = 1-((m_Stamina/(m_StaminaCap*0.01))*0.01);
			exhaustion_value = Math.Min(1,exhaustion_value);
			if ( ad )
			{
				// do not apply exhaustion on local client if player is in ADS/Optics (camera shakes)
				if ( m_Player.GetInstanceType() == DayZPlayerInstanceType.INSTANCETYPE_CLIENT && (m_Player.IsInOptics() || m_Player.IsInIronsights()) )
				{
					ad.SetExhaustion(0, true);
				}
				else
				{
					ad.SetExhaustion(exhaustion_value, true);
				}
			}

			CheckStaminaState();

			m_StaminaDelta = 0;
			m_StaminaDepletion = 0; // resets depletion modifier
		}
	}

	//! called from RPC on playerbase - syncs stamina values on server with client
	void OnRPC(float stamina, float stamina_cap, bool cooldown)
	{
		if ( Math.AbsFloat(stamina - m_Stamina) > 5 )
		{
			m_Stamina = stamina;
		}
		if ( stamina_cap != m_StaminaCap )
		{
			m_StaminaCap = stamina_cap;
		}

		m_IsInCooldown = cooldown;
	}
	
	protected void StaminaProcessor_Move(HumanMovementState pHumanMovementState)
	{
		switch ( pHumanMovementState.m_iMovement )
		{
		case DayZPlayerConstants.MOVEMENTIDX_SPRINT: //sprint
			if ( pHumanMovementState.m_iStanceIdx == DayZPlayerConstants.STANCEIDX_ERECT )
			{
				m_StaminaDelta = -GameConstants.STAMINA_DRAIN_STANDING_SPRINT_PER_SEC;
				SetCooldown(GameConstants.STAMINA_REGEN_COOLDOWN_DEPLETION);
				break;
			}
			if ( pHumanMovementState.m_iStanceIdx == DayZPlayerConstants.STANCEIDX_CROUCH)
			{
				m_StaminaDelta = -GameConstants.STAMINA_DRAIN_CROUCHED_SPRINT_PER_SEC;
				SetCooldown(GameConstants.STAMINA_REGEN_COOLDOWN_DEPLETION);
				break;
			}
			m_StaminaDelta = GameConstants.STAMINA_GAIN_JOG_PER_SEC;
		break;
			
		case DayZPlayerConstants.MOVEMENTIDX_RUN: //jog
			if (!m_IsInCooldown)
			{
				m_StaminaDelta = (GameConstants.STAMINA_GAIN_JOG_PER_SEC + CalcStaminaGainBonus());
			}
		break;
			
		case DayZPlayerConstants.MOVEMENTIDX_WALK: //walk
			if (!m_IsInCooldown)
			{
				m_StaminaDelta = (GameConstants.STAMINA_GAIN_WALK_PER_SEC + CalcStaminaGainBonus());
			}
		break;
			
		case DayZPlayerConstants.MOVEMENTIDX_IDLE: //idle
			if (!m_IsInCooldown)
			{
				m_StaminaDelta = (GameConstants.STAMINA_GAIN_IDLE_PER_SEC + CalcStaminaGainBonus());
			}
		break;
			
		default:
			if( !m_IsInCooldown )
			{
				m_StaminaDelta = GameConstants.STAMINA_GAIN_IDLE_PER_SEC;
			}
		break;
		}
	}
	
	protected void StaminaProcessor_Ladder(HumanMovementState pHumanMovementState)
	{
		switch ( pHumanMovementState.m_iMovement )
		{
		case 2: //climb up (fast)
			m_StaminaDelta = -GameConstants.STAMINA_DRAIN_LADDER_FAST_PER_SEC;
			SetCooldown(GameConstants.STAMINA_REGEN_COOLDOWN_DEPLETION);
			break;
		break;
			
		case 1: //climb up (slow)
			if (!m_IsInCooldown)
			{
				m_StaminaDelta = (GameConstants.STAMINA_GAIN_LADDER_PER_SEC + CalcStaminaGainBonus());
			}
		break;
			
		default:
			if( !m_IsInCooldown )
			{
				m_StaminaDelta = GameConstants.STAMINA_GAIN_IDLE_PER_SEC;
			}
		break;
		}
	}

	protected void StaminaProcessor_Swimming(HumanMovementState pHumanMovementState)
	{
		switch ( pHumanMovementState.m_iMovement )
		{
		case 3: //swim fast
			m_StaminaDelta = -GameConstants.STAMINA_DRAIN_SWIM_FAST_PER_SEC;
			SetCooldown(GameConstants.STAMINA_REGEN_COOLDOWN_DEPLETION);
			break;
		break;
			
		case 2: //swim slow
			if (!m_IsInCooldown)
			{
				m_StaminaDelta = (GameConstants.STAMINA_GAIN_SWIM_PER_SEC + CalcStaminaGainBonus());
			}
		break;
			
		default:
			if( !m_IsInCooldown )
			{
				m_StaminaDelta = GameConstants.STAMINA_GAIN_IDLE_PER_SEC;
			}
		break;
		}
	}
	
	//! stamina sync - server part
	protected void SyncStamina(float stamina, float stamina_cap, bool cooldown)
	{
		if( m_StaminaParams )
		{
			m_Player.GetStatStamina().Set(m_Stamina);
			m_StaminaParams.param1 = m_Stamina;
			m_StaminaParams.param2 = m_StaminaCap;
			m_StaminaParams.param3 = m_IsInCooldown;
			GetGame().RPCSingleParam(m_Player, ERPCs.RPC_STAMINA, m_StaminaParams, true, m_Player.GetIdentity());
		}
	}
	
	protected void RegisterStaminaConsumers()
	{
		//! stamina consumers registration
		m_StaminaConsumers = new StaminaConsumers;
		m_StaminaConsumers.RegisterConsumer(EStaminaConsumers.HOLD_BREATH, GameConstants.STAMINA_HOLD_BREATH_THRESHOLD);
		m_StaminaConsumers.RegisterConsumer(EStaminaConsumers.SPRINT, GameConstants.STAMINA_MIN_CAP + 15);
		m_StaminaConsumers.RegisterConsumer(EStaminaConsumers.JUMP, GameConstants.STAMINA_JUMP_THRESHOLD);
		m_StaminaConsumers.RegisterConsumer(EStaminaConsumers.VAULT, GameConstants.STAMINA_VAULT_THRESHOLD);
		m_StaminaConsumers.RegisterConsumer(EStaminaConsumers.CLIMB, GameConstants.STAMINA_CLIMB_THRESHOLD);
		m_StaminaConsumers.RegisterConsumer(EStaminaConsumers.MELEE_HEAVY, GameConstants.STAMINA_MELEE_HEAVY_THRESHOLD);
		m_StaminaConsumers.RegisterConsumer(EStaminaConsumers.MELEE_EVADE, GameConstants.STAMINA_MELEE_EVADE_THRESHOLD);
	}

	protected void RegisterStaminaModifiers()
	{
		//! stamina modifiers registration
		m_StaminaModifiers = new StaminaModifiers;
		m_StaminaModifiers.RegisterFixed(EStaminaModifiers.HOLD_BREATH, GameConstants.STAMINA_DRAIN_HOLD_BREATH);
		m_StaminaModifiers.RegisterFixed(EStaminaModifiers.JUMP, GameConstants.STAMINA_DRAIN_JUMP);
		m_StaminaModifiers.RegisterFixed(EStaminaModifiers.VAULT, GameConstants.STAMINA_DRAIN_VAULT);
		m_StaminaModifiers.RegisterFixed(EStaminaModifiers.CLIMB, GameConstants.STAMINA_DRAIN_CLIMB);
		m_StaminaModifiers.RegisterFixed(EStaminaModifiers.MELEE_LIGHT, GameConstants.STAMINA_DRAIN_MELEE_LIGHT);
		m_StaminaModifiers.RegisterFixed(EStaminaModifiers.MELEE_HEAVY, GameConstants.STAMINA_DRAIN_MELEE_HEAVY);
		m_StaminaModifiers.RegisterFixed(EStaminaModifiers.OVERALL_DRAIN, GameConstants.STAMINA_MAX, 5.0);
		m_StaminaModifiers.RegisterRandomized(EStaminaModifiers.MELEE_EVADE, 3, GameConstants.STAMINA_DRAIN_MELEE_EVADE);
	}

	protected float CalcStaminaGainBonus()// Calulates stamina regain bonus based on current stamina level (So it's better to wait for stamina to refill completely and avoid overloading)
	{
		if (m_StaminaDepletion > 0)
			return 0;

		if (m_Stamina > 25)
			return Math.Min((m_Stamina/10),GameConstants.STAMINA_GAIN_BONUS_CAP); // exp version
		else
			return GameConstants.STAMINA_GAIN_BONUS_CAP; // linear version
	}

	
	//! check if the stamina is completely depleted
	protected void CheckStaminaState()
	{
		if ( m_Stamina <= 0 )
		{
			m_StaminaDepleted = true;
			//! in case of complete depletion - start a cooldown timer before the regeneration cycle start
			if(!m_IsInCooldown) SetCooldown(GameConstants.STAMINA_REGEN_COOLDOWN_EXHAUSTION); // set this only once
		}
		else
			m_StaminaDepleted = false;
	}

	//! set cooldown timer between each consume of stamina	
	protected void SetCooldown(float time)
	{
		if( m_StaminaDepleted || m_Stamina <= 0.0 ) return;

		m_IsInCooldown = true;
		if( m_CooldownTimer.IsRunning() )
			m_CooldownTimer.Stop();
		m_CooldownTimer.Run(time, this, "ResetCooldown", null);
	}
	
	protected void ResetCooldown()
	{
		m_IsInCooldown = false;
	}
	
	// ---------------------------------------------------
	bool HasEnoughStaminaFor(EStaminaConsumers consumer)
	{
		return m_StaminaConsumers.HasEnoughStaminaFor(consumer, m_Stamina, m_StaminaDepleted);
	}

	void SetStamina(float stamina_value)
	{
		m_Stamina = Math.Clamp(stamina_value, 0, GameConstants.STAMINA_MAX);
		SyncStamina(m_Stamina, m_StaminaCap, m_IsInCooldown);
	}
	
	float GetStamina()
	{
		return m_Stamina;
	}
	
	float GetStaminaCap()
	{
		return m_StaminaCap;
	}
	
	float GetStaminaMax()
	{
		return GameConstants.STAMINA_MAX;
	}

	float GetStaminaNormalized()
	{	
		return m_Stamina / GameConstants.STAMINA_MAX;
	}
	
	void DepleteStamina(EStaminaModifiers modifier)
	{
		float val = 0.0;
		StaminaModifier sm = m_StaminaModifiers.GetModifierData(modifier);

		//! select by modifier type and drain stamina
		switch (sm.GetType())
		{
			case m_StaminaModifiers.FIXED:
				m_StaminaDepletion = m_StaminaDepletion + sm.GetMaxValue();
			break;
			
			case m_StaminaModifiers.RANDOMIZED:
				val = Math.RandomFloat(sm.GetMinValue(), sm.GetMaxValue());
				m_StaminaDepletion = m_StaminaDepletion + val;
			break;
		}

		//! run cooldown right after depletion
		SetCooldown(sm.GetCooldown());
		m_StaminaDepletion = Math.Clamp(m_StaminaDepletion, 0, GameConstants.STAMINA_MAX);
	}
};