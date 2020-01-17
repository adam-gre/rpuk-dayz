typedef ItemBase Inventory_Base;

class DummyItem extends ItemBase {};

//const bool QUANTITY_DEBUG_REMOVE_ME = false;

class ItemBase extends InventoryItem
{
	static ref map<typename, ref TInputActionMap> m_ItemTypeActionsMap = new map<typename, ref TInputActionMap>;
	TInputActionMap m_InputActionMap;
	bool	m_ActionsInitialize;
	
	static int	m_DebugActionsMask;
	bool		m_RecipesInitialized;
	// ============================================
	// Variable Manipulation System
	// ============================================
	//ref map<string,string> 	m_ItemVarsString;
	
	int		m_VariablesMask;//this holds information about which vars have been changed from their default values
	float 	m_VarQuantity;
	float 	m_VarTemperature;
	float 	m_VarWet;
	float	m_HeatIsolation;
	float 	m_Absorbency; 
	float 	m_ItemModelLength;
	int 	m_VarLiquidType;
	int 	m_ItemBehaviour = -1; // -1 = not specified; 0 = heavy item; 1= onehanded item; 2 = twohanded item
	bool	m_IsBeingPlaced;
	bool	m_IsHologram;
	bool	m_IsPlaceSound;
	bool	m_IsDeploySound;
	bool	m_IsTakeable;
	bool	m_IsSoundSynchRemote;
	string	m_SoundAttType;
	// items color variables
	int 	m_ColorComponentR;
	int 	m_ColorComponentG;
	int 	m_ColorComponentB;
	int 	m_ColorComponentA;
	//-------------------------------------------------------
	
	ref TIntArray m_SingleUseActions;
	ref TIntArray m_ContinuousActions;
	ref TIntArray m_InteractActions;
	
	//==============================================
	// agent system
	private int	m_AttachedAgents;

	// Declarations
	void TransferModifiers( PlayerBase reciever );
	
	
	// Weapons & suppressors particle effects
	ref static map<int, ref array<ref WeaponParticlesOnFire>> 				m_OnFireEffect;
	ref static map<int, ref array<ref WeaponParticlesOnBulletCasingEject>> 	m_OnBulletCasingEjectEffect;
	ref map<int, ref array<ref WeaponParticlesOnOverheating>> 				m_OnOverheatingEffect;
	ref static map<string, int> m_WeaponTypeToID;
	static int m_LastRegisteredWeaponID = 0;
	
	// Overheating effects
	bool 								m_IsOverheatingEffectActive;
	float								m_OverheatingShots;
	ref Timer 							m_CheckOverheating;
	int 								m_ShotsToStartOverheating = 0; // After these many shots, the overheating effect begins
	int 								m_MaxOverheatingValue = 0; // Limits the number of shots that will be tracked
	float 								m_OverheatingDecayInterval = 1; // Timer's interval for decrementing overheat effect's lifespan
	ref array <ref OverheatingParticle> m_OverheatingParticles;
	
	protected ref TStringArray m_HeadHidingSelections;
	
	// Admin Log
	PluginAdminLog			m_AdminLog;
	
	// -------------------------------------------------------------------------
	void ItemBase()
	{
		SetEventMask(EntityEvent.INIT); // Enable EOnInit event
		InitItemVariables();

		m_SingleUseActions = new TIntArray;
		m_ContinuousActions = new TIntArray;
		m_InteractActions = new TIntArray;
		
		if (HasMuzzle())
		{
			LoadParticleConfigOnFire( GetMuzzleID() );
			
			if ( m_ShotsToStartOverheating == 0 )
			{
				LoadParticleConfigOnOverheating( GetMuzzleID() );
			}
		}

		if (GetGame().IsClient() || !GetGame().IsMultiplayer())
		{
			PreLoadSoundAttachmentType();
			m_ActionsInitialize = false;
		}
		m_OldLocation = null;
		
		if ( GetGame().IsServer() )
		{
			m_AdminLog = PluginAdminLog.Cast( GetPlugin(PluginAdminLog) );
		}
		
		if ( ConfigIsExisting("headSelectionsToHide") )
		{
			m_HeadHidingSelections = new TStringArray;
			ConfigGetTextArray("headSelectionsToHide",m_HeadHidingSelections);
		}
	}
	
	void InitItemVariables()
	{
		m_VarTemperature = GetTemperatureInit();
		m_VarWet = GetWetInit();
		m_VarLiquidType = GetLiquidTypeInit();
		m_VarQuantity = GetQuantityInit();//should be by the CE, this is just a precaution
		m_IsBeingPlaced = false;
		m_IsHologram = false;
		m_IsPlaceSound = false;
		m_IsDeploySound = false;
		m_IsTakeable = true;
		m_IsSoundSynchRemote = false;
		m_HeatIsolation = GetHeatIsolationInit();
		m_Absorbency = GetAbsorbencyInit();
		m_ItemModelLength = GetItemModelLength();
		if(ConfigIsExisting("itemBehaviour"))
		{
			m_ItemBehaviour = ConfigGetInt("itemBehaviour");
		}
		
		//RegisterNetSyncVariableInt("m_VariablesMask");
		if ( HasQuantity() ) RegisterNetSyncVariableFloat("m_VarQuantity", GetQuantityMin(), GetQuantityMax() );
		RegisterNetSyncVariableFloat("m_VarTemperature", GetTemperatureMin(),GetTemperatureMax() );
		RegisterNetSyncVariableFloat("m_VarWet", GetWetMin(), GetWetMax(), 2 );
		RegisterNetSyncVariableInt("m_VarLiquidType");
		
		RegisterNetSyncVariableInt("m_ColorComponentR", 0, 255);
		RegisterNetSyncVariableInt("m_ColorComponentG", 0, 255);
		RegisterNetSyncVariableInt("m_ColorComponentB", 0, 255);
		RegisterNetSyncVariableInt("m_ColorComponentA", 0, 255);
		
		RegisterNetSyncVariableBool("m_IsBeingPlaced");
		RegisterNetSyncVariableBool("m_IsTakeable");
		RegisterNetSyncVariableBool("m_IsHologram");
	}

	void InitializeActions()
	{
		m_InputActionMap = m_ItemTypeActionsMap.Get( this.Type() );
		if(!m_InputActionMap)
		{
			TInputActionMap iam = new TInputActionMap;
			m_InputActionMap = iam;
			SetActions();
			m_ItemTypeActionsMap.Insert( this.Type(), m_InputActionMap);
		}
	}
	
	override void GetActions(typename action_input_type, out array<ActionBase_Basic> actions)
	{
		if(!m_ActionsInitialize)
		{
			m_ActionsInitialize = true;
			InitializeActions();
		}
		
		actions = m_InputActionMap.Get(action_input_type);
	}
	
	void SetActions()
	{
		AddAction(ActionTakeItem);
		AddAction(ActionTakeItemToHands);
		AddAction(ActionWorldCraft);
		AddAction(ActionWorldCraftSwitch);
		AddAction(ActionDropItem);
		//AddAction();
		//AddAction();
	}
	
	void AddAction(typename actionName)
	{
		ActionBase action = ActionManagerBase.GetAction(actionName);

		if(!action)
		{
			Debug.LogError("Action " + actionName + " dosn't exist!");
			return;
		}		
		
		typename ai = action.GetInputType();
		if(!ai)
		{
			m_ActionsInitialize = false;
			return;
		}
		ref array<ActionBase_Basic> action_array = m_InputActionMap.Get( ai );
		
		if(!action_array)
		{
			action_array = new array<ActionBase_Basic>;
			m_InputActionMap.Insert(ai, action_array);
		}
		
		Debug.Log("+ " + this + " add action: " + action + " input " + ai);

		action_array.Insert(action);
	}
	
	void RemoveAction(typename actionName)
	{
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		ActionBase action = player.GetActionManager().GetAction(actionName);
		typename ai = action.GetInputType();
		ref array<ActionBase_Basic> action_array = m_InputActionMap.Get( ai );
		
		if(action_array)
		{
			action_array.RemoveItem(action);
		}
	}
	
	// Loads muzzle flash particle configuration from config and saves it to a map for faster access
	void LoadParticleConfigOnFire(int id)
	{
		if (!m_OnFireEffect)
			m_OnFireEffect = new map<int, ref array<ref WeaponParticlesOnFire>>;
		
		if (!m_OnBulletCasingEjectEffect)
			m_OnBulletCasingEjectEffect = new map<int, ref array<ref WeaponParticlesOnBulletCasingEject>>;
		
		string config_to_search = "CfgVehicles";
		string muzzle_owner_config;
		
		if ( !m_OnFireEffect.Contains(id) )
		{
			if (IsInherited(Weapon))
				config_to_search = "CfgWeapons";	
			
			muzzle_owner_config = config_to_search + " " + GetType() + " ";
			
			string config_OnFire_class = muzzle_owner_config + "Particles " + "OnFire ";
			
			int config_OnFire_subclass_count = GetGame().ConfigGetChildrenCount(config_OnFire_class);
			
			if (config_OnFire_subclass_count > 0)
			{
				ref array<ref WeaponParticlesOnFire> WPOF_array = new array<ref WeaponParticlesOnFire>;
				
				for (int i = 0; i < config_OnFire_subclass_count; i++)
				{
					string particle_class = "";
					GetGame().ConfigGetChildName(config_OnFire_class, i, particle_class);
					string config_OnFire_entry = config_OnFire_class + particle_class;
					WeaponParticlesOnFire WPOF = new WeaponParticlesOnFire(this, config_OnFire_entry);
					WPOF_array.Insert(WPOF);
				}
				
				
				m_OnFireEffect.Insert(id, WPOF_array);
			}
		}
		
		if ( !m_OnBulletCasingEjectEffect.Contains(id) )
		{
			config_to_search = "CfgWeapons"; // Bullet Eject efect is supported on weapons only.
			muzzle_owner_config = config_to_search + " " + GetType() + " ";
			
			string config_OnBulletCasingEject_class = muzzle_owner_config + "Particles " + "OnBulletCasingEject ";
			
			int config_OnBulletCasingEject_count = GetGame().ConfigGetChildrenCount(config_OnBulletCasingEject_class);
			
			if (config_OnBulletCasingEject_count > 0  &&  IsInherited(Weapon))
			{
				ref array<ref WeaponParticlesOnBulletCasingEject> WPOBE_array = new array<ref WeaponParticlesOnBulletCasingEject>;
				
				for (i = 0; i < config_OnBulletCasingEject_count; i++)
				{
					string particle_class2 = "";
					GetGame().ConfigGetChildName(config_OnBulletCasingEject_class, i, particle_class2);
					string config_OnBulletCasingEject_entry = config_OnBulletCasingEject_class + particle_class2;
					WeaponParticlesOnBulletCasingEject WPOBE = new WeaponParticlesOnBulletCasingEject(this, config_OnBulletCasingEject_entry);
					WPOBE_array.Insert(WPOBE);
				}
				
				
				m_OnBulletCasingEjectEffect.Insert(id, WPOBE_array);
			}
		}
	}
	
	// Loads muzzle flash particle configuration from config and saves it to a map for faster access
	void LoadParticleConfigOnOverheating(int id)
	{
		if (!m_OnOverheatingEffect)
			m_OnOverheatingEffect = new map<int, ref array<ref WeaponParticlesOnOverheating>>;
		
		if ( !m_OnOverheatingEffect.Contains(id) )
		{
			string config_to_search = "CfgVehicles";
			
			if (IsInherited(Weapon))
				config_to_search = "CfgWeapons";
			
			string muzzle_owner_config = config_to_search + " " + GetType() + " ";
			string config_OnOverheating_class = muzzle_owner_config + "Particles " + "OnOverheating ";
			
			if (GetGame().ConfigIsExisting(config_OnOverheating_class))
			{
				
				m_ShotsToStartOverheating = GetGame().ConfigGetFloat(config_OnOverheating_class + "shotsToStartOverheating");
				
				if (m_ShotsToStartOverheating == 0)
				{
					m_ShotsToStartOverheating = -1; // This prevents futher readings from config for future creations of this item
					string error = "Error reading config " + GetType() + ">Particles>OnOverheating - Parameter shotsToStartOverheating is configured wrong or is missing! Its value must be 1 or higher!";
					Error(error);
					return;
				}
				
				m_OverheatingDecayInterval = GetGame().ConfigGetFloat(config_OnOverheating_class + "overheatingDecayInterval");
				m_MaxOverheatingValue = GetGame().ConfigGetFloat(config_OnOverheating_class + "maxOverheatingValue");
				
				
				
				int config_OnOverheating_subclass_count = GetGame().ConfigGetChildrenCount(config_OnOverheating_class);
				ref array<ref WeaponParticlesOnOverheating> WPOOH_array = new array<ref WeaponParticlesOnOverheating>;
				
				for (int i = 0; i < config_OnOverheating_subclass_count; i++)
				{
					string particle_class = "";
					GetGame().ConfigGetChildName(config_OnOverheating_class, i, particle_class);
					string config_OnOverheating_entry = config_OnOverheating_class + particle_class;
					int  entry_type = GetGame().ConfigGetType(config_OnOverheating_entry);
					
					if ( entry_type == CT_CLASS )
					{
						WeaponParticlesOnOverheating WPOF = new WeaponParticlesOnOverheating(this, config_OnOverheating_entry);
						WPOOH_array.Insert(WPOF);
					}
				}
				
				
				m_OnOverheatingEffect.Insert(id, WPOOH_array);
			}
		}
	}
	
	float GetOverheatingValue()
	{
		return m_OverheatingShots;
	}
	
	void IncreaseOverheating(ItemBase weapon, string ammoType, ItemBase muzzle_owner, ItemBase suppressor, string config_to_search)
	{
		if (m_MaxOverheatingValue > 0)
		{
			m_OverheatingShots++;
			
			if (!m_CheckOverheating)
					m_CheckOverheating = new Timer( CALL_CATEGORY_SYSTEM );
			
			m_CheckOverheating.Stop();
			m_CheckOverheating.Run(m_OverheatingDecayInterval, this, "OnOverheatingDecay");
			
			CheckOverheating(weapon, ammoType, muzzle_owner, suppressor, config_to_search);
		}
	}
	
	void CheckOverheating(ItemBase weapon = null, string ammoType = "", ItemBase muzzle_owner = null, ItemBase suppressor = null, string config_to_search = "")
	{
		if (m_OverheatingShots >= m_ShotsToStartOverheating  &&  IsOverheatingEffectActive())
			UpdateOverheating(weapon, ammoType, muzzle_owner, suppressor, config_to_search);
		
		if (m_OverheatingShots >= m_ShotsToStartOverheating  &&  !IsOverheatingEffectActive())
			StartOverheating(weapon, ammoType, muzzle_owner, suppressor, config_to_search);
		
		if (m_OverheatingShots < m_ShotsToStartOverheating  &&  IsOverheatingEffectActive())
			StopOverheating(weapon, ammoType, muzzle_owner, suppressor, config_to_search);
		
		if (m_OverheatingShots > m_MaxOverheatingValue)
		{
			m_OverheatingShots = m_MaxOverheatingValue;
		}
	}
	
	bool IsOverheatingEffectActive()
	{
		return m_IsOverheatingEffectActive;
	}
	
	void OnOverheatingDecay()
	{
		if (m_MaxOverheatingValue > 0)
			m_OverheatingShots -= 1 + m_OverheatingShots / m_MaxOverheatingValue; // The hotter a barrel is, the faster it needs to cool down.
		else
			m_OverheatingShots--;
		
		if (m_OverheatingShots <= 0)
		{
			m_CheckOverheating.Stop();
			m_OverheatingShots = 0;
		}
		else
		{
			if (!m_CheckOverheating)
				m_CheckOverheating = new Timer( CALL_CATEGORY_GAMEPLAY );
			
			m_CheckOverheating.Stop();
			m_CheckOverheating.Run(m_OverheatingDecayInterval, this, "OnOverheatingDecay");
		}
		
		CheckOverheating(this, "", this);
	}

	void StartOverheating(ItemBase weapon = null, string ammoType = "", ItemBase muzzle_owner = null, ItemBase suppressor = null, string config_to_search = "")
	{
		m_IsOverheatingEffectActive = true;
		ItemBase.PlayOverheatingParticles(this, ammoType, this, suppressor, "CfgWeapons" );
	}
	
	void UpdateOverheating(ItemBase weapon = null, string ammoType = "", ItemBase muzzle_owner = null, ItemBase suppressor = null, string config_to_search = "")
	{
		KillAllOverheatingParticles();
		ItemBase.UpdateOverheatingParticles(this, ammoType, this, suppressor, "CfgWeapons" );
		UpdateAllOverheatingParticles();
	}
	
	void StopOverheating(ItemBase weapon = null, string ammoType = "", ItemBase muzzle_owner = null, ItemBase suppressor = null, string config_to_search = "")
	{
		m_IsOverheatingEffectActive = false;
		ItemBase.StopOverheatingParticles(weapon, ammoType, muzzle_owner, suppressor, config_to_search);
	}
	
	void RegisterOverheatingParticle(Particle p, float min_heat_coef, float max_heat_coef, int particle_id, Object parent, vector local_pos, vector local_ori)
	{
		if (!m_OverheatingParticles)
			m_OverheatingParticles = new array <ref OverheatingParticle>;
		
		OverheatingParticle OP = new OverheatingParticle();
		OP.RegisterParticle(p);
		OP.SetOverheatingLimitMin(min_heat_coef);
		OP.SetOverheatingLimitMax(max_heat_coef);
		OP.SetParticleParams(particle_id, parent, local_pos, local_ori);
		
		m_OverheatingParticles.Insert(OP);
	}
	
	float GetOverheatingCoef()
	{
		if (m_MaxOverheatingValue > 0)
			return (m_OverheatingShots - m_ShotsToStartOverheating) / m_MaxOverheatingValue;
		
		return -1;
	}
	
	void UpdateAllOverheatingParticles()
	{
		if (m_OverheatingParticles)
		{
			float overheat_coef = GetOverheatingCoef();
			int count = m_OverheatingParticles.Count();
			
			for (int i = count; i > 0; --i)
			{
				int id = i - 1;
				OverheatingParticle OP = m_OverheatingParticles.Get(id);
				Particle p = OP.GetParticle();
				
				float overheat_min = OP.GetOverheatingLimitMin();
				float overheat_max = OP.GetOverheatingLimitMax();
				
				if (overheat_coef < overheat_min  &&  overheat_coef >= overheat_max)
				{
					if (p)
					{
						p.Stop();
						OP.RegisterParticle(NULL);
					}
				}
			}
		}
	}
	
	void KillAllOverheatingParticles()
	{
		if (m_OverheatingParticles)
		{
			for (int i = m_OverheatingParticles.Count(); i > 0; i--)
			{
				int id = i - 1;
				OverheatingParticle OP = m_OverheatingParticles.Get(id);
				
				if (OP)
				{
					Particle p = OP.GetParticle();
					
					if (p)
					{
						p.Stop();
					}
					
					delete OP;
				}
			}
			
			m_OverheatingParticles.Clear();
			delete m_OverheatingParticles;
		}
	}
	
	//! Returns true if this item has a muzzle (weapons, suppressors)
	bool HasMuzzle()
	{
		if (IsInherited(Weapon)  ||  IsInherited(SuppressorBase))
			return true;
		
		return false;
	}
	
	//! Returns global muzzle ID. If not found, then it gets automatically registered.
	int GetMuzzleID()
	{
		if (!m_WeaponTypeToID)
			m_WeaponTypeToID = new map<string, int>;
		
		if ( m_WeaponTypeToID.Contains( GetType() ) )
		{
			return m_WeaponTypeToID.Get( GetType() );
		}
		else 
		{
			// Register new weapon ID
			m_WeaponTypeToID.Insert(GetType(), ++m_LastRegisteredWeaponID);
		}
		
		return m_LastRegisteredWeaponID;
	}
	
	// -------------------------------------------------------------------------
	void ~ItemBase()
	{
		if( GetGame() && GetGame().GetPlayer() && ( !GetGame().IsMultiplayer() || GetGame().IsClient() ) )
		{
			GetGame().GetPlayer().GetHumanInventory().ClearUserReservedLocation( this );
			GetOnReleaseLock().Invoke( this );
		}
	}

	// -------------------------------------------------------------------------
	static int GetDebugActionsMask()
	{
		return ItemBase.m_DebugActionsMask;	
	}
	
	static void SetDebugActionsMask(int mask)
	{
		ItemBase.m_DebugActionsMask = mask;
	}
	
	void SetCEBasedQuantity()
	{
		if( GetEconomyProfile() )
		{
			float q_min = GetEconomyProfile().GetQuantityMin();
			float q_max = GetEconomyProfile().GetQuantityMax();
			if( q_max > 0 )
			{
				float quantity_randomized = Math.RandomFloatInclusive(q_min, q_max);
				//PrintString("<==> Normalized quantity for item: "+ GetType()+", qmin:"+q_min.ToString()+"; qmax:"+q_max.ToString()+";quantity:" +quantity_randomized.ToString());
					SetQuantityNormalized(quantity_randomized, false);
				}
		}
	}
	
	//! Locks this item in it's current attachment slot of its parent. This makes the "locked" icon visible in inventory over this item.
	void LockToParent()
	{
		EntityAI parent = GetHierarchyParent();
		
		if (parent)
		{
			InventoryLocation inventory_location_to_lock = new InventoryLocation;
			GetInventory().GetCurrentInventoryLocation( inventory_location_to_lock );
			parent.GetInventory().SetSlotLock( inventory_location_to_lock.GetSlot(), true );
		}
	}
	
	//! Unlocks this item from its attachment slot of its parent.
	void UnlockFromParent()
	{
		EntityAI parent = GetHierarchyParent();
		
		if (parent)
		{
			InventoryLocation inventory_location_to_unlock = new InventoryLocation;
			GetInventory().GetCurrentInventoryLocation( inventory_location_to_unlock );
			parent.GetInventory().SetSlotLock( inventory_location_to_unlock.GetSlot(), false );
		}
	}
	
	void CombineItemsClient(ItemBase entity2, bool use_stack_max = false )
	{
		/*
		ref Param1<EntityAI> item = new Param1<EntityAI>(entity2);
		RPCSingleParam( ERPCs.RPC_ITEM_COMBINE, item, GetGame().GetPlayer() );
		*/
		
		if( GetGame().IsClient() )
		{
			if (ScriptInputUserData.CanStoreInputUserData())
			{
				ScriptInputUserData ctx = new ScriptInputUserData;
				ctx.Write(INPUT_UDT_ITEM_MANIPULATION);
				ctx.Write(-1);
				ItemBase i1 = this; // @NOTE: workaround for correct serialization
				ctx.Write(i1);
				ctx.Write(entity2);
				ctx.Write(use_stack_max);
				ctx.Write(-1);
				ctx.Send();
				
				if( IsCombineAll(entity2, use_stack_max) )
				{
					InventoryLocation il = new InventoryLocation;
					entity2.GetInventory().GetCurrentInventoryLocation(il);
					GetGame().GetPlayer().GetInventory().AddInventoryReservation(entity2,il,GameInventory.c_InventoryReservationTimeoutShortMS);
					//entity2.GetInventory().GetCurrentInventoryLocation
				}
			}
		}
		else if( !GetGame().IsMultiplayer() )
		{
			CombineItems(entity2, use_stack_max);
		}
	}
	
	bool IsLiquidPresent()
	{
		
		if(GetLiquidType() != 0 && HasQuantity() ) return true;
		else return false;
	}
	
	bool IsLiquidContainer()
	{
		if( ConfigGetFloat("liquidContainerType")!=0 ) return true;
		else return false;
	}
	
	bool IsBloodContainer()
	{
		return false;
	}
	
	bool IsNVG()
	{
		return false;
	}
	
	bool IsExplosive()
	{
		return false;
	}
	
	bool IsLightSource()
	{
		return false;
	}
	
	override bool IsBeingPlaced()
	{
		return m_IsBeingPlaced;
	}
	
	void SetIsBeingPlaced( bool is_being_placed )
	{
		m_IsBeingPlaced = is_being_placed;
		SetSynchDirty();
	}
	
	override bool IsHologram()
	{
		return m_IsHologram;
	}
	
	bool CanMakeGardenplot()
	{
		return false;
	}
	
	void SetIsHologram( bool is_hologram )
	{
		m_IsHologram = is_hologram;
		SetSynchDirty();
	}
	/*
	protected float GetNutritionalEnergy()
	{
		Edible_Base edible = Edible_Base.Cast( this );
		return edible.GetFoodEnergy();
	}
	
	protected float GetNutritionalWaterContent()
	{
		Edible_Base edible = Edible_Base.Cast( this );
		return edible.GetFoodWater();
	}
	
	protected float GetNutritionalIndex()
	{
		Edible_Base edible = Edible_Base.Cast( this );
		return edible.GetFoodNutritionalIndex();
	}
	
	protected float GetNutritionalFullnessIndex()
	{
		Edible_Base edible = Edible_Base.Cast( this );
		return edible.GetFoodTotalVolume();
	}
	
	protected float GetNutritionalToxicity()
	{
		Edible_Base edible = Edible_Base.Cast( this );
		return edible.GetFoodToxicity();

	}
*/
	
	
	// -------------------------------------------------------------------------
	override void EECargoIn(EntityAI item)
	{
		super.EECargoIn(item);
	}
	
	override void EEItemLocationChanged(notnull InventoryLocation oldLoc, notnull InventoryLocation newLoc)
	{
		super.EEItemLocationChanged(oldLoc,newLoc);
		
		PlayerBase new_player = null;
		PlayerBase old_player = null;
		
		if( newLoc.GetParent() )
			new_player = PlayerBase.Cast(newLoc.GetParent().GetHierarchyRootPlayer());
		
		if( oldLoc.GetParent() )
			old_player = PlayerBase.Cast(oldLoc.GetParent().GetHierarchyRootPlayer());
		
		if(old_player && oldLoc.GetType() == InventoryLocationType.HANDS)
		{
			int r_index = old_player.GetHumanInventory().FindUserReservedLocationIndex(this);

			if(r_index >= 0)
			{
					InventoryLocation r_il = new InventoryLocation;
					old_player.GetHumanInventory().GetUserReservedLocation(r_index,r_il);

					old_player.GetHumanInventory().ClearUserReservedLocationAtIndex(r_index);
					int r_type = r_il.GetType();
					if( r_type == InventoryLocationType.CARGO || r_type == InventoryLocationType.PROXYCARGO )
					{
						r_il.GetParent().GetOnReleaseLock().Invoke( this );
					}
					else if( r_type == InventoryLocationType.ATTACHMENT )
					{
						r_il.GetParent().GetOnAttachmentReleaseLock().Invoke( this, r_il.GetSlot() );
					}
			
			}
			//old_player.GetHumanInventory().ClearUserReservedLocation(this);
			//GetOnReleaseLock().Invoke(this);
		}
		
		if(newLoc.GetType() == InventoryLocationType.HANDS)
		{
			if(new_player == old_player)
			{
				if(new_player.GetHumanInventory().LocationGetEntity(oldLoc) == NULL )
				{
					new_player.GetHumanInventory().SetUserReservedLocation(this,oldLoc);
				}
				
				if( new_player.GetHumanInventory().FindUserReservedLocationIndex( this ) >= 0 )
				{
					int type = oldLoc.GetType();
					if( type == InventoryLocationType.CARGO || type == InventoryLocationType.PROXYCARGO )
					{
						oldLoc.GetParent().GetOnSetLock().Invoke( this );
					}
					else if( type == InventoryLocationType.ATTACHMENT )
					{
						oldLoc.GetParent().GetOnAttachmentSetLock().Invoke( this, oldLoc.GetSlot() );
					}
				}
				if (!m_OldLocation)
				{
					m_OldLocation = new InventoryLocation;
				}
				m_OldLocation.Copy(oldLoc);
			}
			else
			{
				if (m_OldLocation)
				{
					m_OldLocation.Reset();
				}
			}
		}
		else
		{
			if(new_player)
			{
				int res_index = new_player.GetHumanInventory().FindCollidingUserReservedLocationIndex(this, newLoc );
				if(res_index >= 0)
				{
					InventoryLocation il = new InventoryLocation;
					new_player.GetHumanInventory().GetUserReservedLocation(res_index,il);
					ItemBase it = ItemBase.Cast(il.GetItem());
					new_player.GetHumanInventory().ClearUserReservedLocationAtIndex(res_index);
					int rel_type = il.GetType();
					if( rel_type == InventoryLocationType.CARGO || rel_type == InventoryLocationType.PROXYCARGO )
					{
						il.GetParent().GetOnReleaseLock().Invoke( it );
					}
					else if( rel_type == InventoryLocationType.ATTACHMENT )
					{
						il.GetParent().GetOnAttachmentReleaseLock().Invoke( it, il.GetSlot() );
					}
					//it.GetOnReleaseLock().Invoke(it);
				}
			}
		
			if (m_OldLocation)
			{
				m_OldLocation.Reset();
			}
		}
	}
	
	override void OnItemAttachmentSlotChanged(notnull InventoryLocation oldLoc, notnull InventoryLocation newLoc)
	{
		
	}
	// -------------------------------------------------------------------------
	override void OnItemLocationChanged(EntityAI old_owner, EntityAI new_owner)
	{
		super.OnItemLocationChanged(old_owner, new_owner);
		
		Man owner_player_old = NULL;
		Man owner_player_new = NULL;
	
		if (old_owner)   
		{
			if (old_owner.IsMan())
			{
				owner_player_old = Man.Cast( old_owner );
			}
			else
			{
				owner_player_old = Man.Cast( old_owner.GetHierarchyRootPlayer() );
			}
		}
		
		if (new_owner)
		{
			if ( new_owner.IsMan() )
			{
				owner_player_new = Man.Cast( new_owner );
			}
			else
			{
				owner_player_new = Man.Cast( new_owner.GetHierarchyRootPlayer() );
			}
		}
		
		if ( owner_player_old != owner_player_new )
		{
			if ( owner_player_old )
			{
				array<EntityAI> subItemsExit = new array<EntityAI>;
				GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER,subItemsExit);
				for (int i = 0; i < subItemsExit.Count(); i++)
				{
					ItemBase item_exit = ItemBase.Cast(subItemsExit.Get(i));
					item_exit.OnInventoryExit(owner_player_old);
				}
			}

			if ( owner_player_new )
			{
				array<EntityAI> subItemsEnter = new array<EntityAI>;
				GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER,subItemsEnter);
				for (int j = 0; j < subItemsEnter.Count(); j++)
				{
					ItemBase item_enter = ItemBase.Cast(subItemsEnter.Get(j));
					item_enter.OnInventoryEnter(owner_player_new);
				}
			}
		}
	}

	// -------------------------------------------------------------------------------
	override void EOnInit(IEntity other, int extra)
	{
		
	}
	// -------------------------------------------------------------------------------
	override void EEDelete(EntityAI parent)
	{
		super.EEDelete(parent);
		PlayerBase player = PlayerBase.Cast( GetHierarchyRootPlayer() );
		if(player)
		{
			int index = player.GetHumanInventory().FindUserReservedLocationIndex(this);
			if(index >= 0 )
				player.GetHumanInventory().ClearUserReservedLocationAtIndex(index);
			player.RemoveQuickBarEntityShortcut(this);
			OnInventoryExit(player);
		}
	}
	// -------------------------------------------------------------------------------
	override void EEKilled( Object killer)
	{
		super.EEKilled( killer );
		
		//if item is able to explode in fire
		if ( CanExplodeInFire() )
		{
			float min_temp_to_explode	= 100;		//min temperature for item to explode
			
			if ( GetTemperature() >= min_temp_to_explode )		//TODO ? add check for parent -> fireplace
			{
				if ( IsMagazine() )
				{
					Magazine magazine = Magazine.Cast( this );					
					
					//explode ammo
					if ( magazine.GetAmmoCount() > 0 )
					{
						ExplodeAmmo();
					}
				}
				else
				{
					//explode item
					Explode(DT_EXPLOSION);
				}
			}
		}
	}
	override void OnWasAttached( EntityAI parent, int slot_id )
	{
		/*PlayerBase player = PlayerBase.Cast(parent);
		if (player)
			//player.SwitchItemTypeAttach(this, slot_id);
		//{
		//	player.UpdateQuickBarEntityVisibility(this);
		//}*/
		//Print("OnWasAttached: " + GetType());
	}
	
	override void OnWasDetached ( EntityAI parent, int slot_id )
	{
		//Print("OnWasDetached: " + GetType());
		PlayerBase player = PlayerBase.Cast(parent);
		if (player)
		{
			if( !GetGame().IsServer() )
			return;
			
			/*if (this.ConfigIsExisting("ChangeIntoOnDetach"))
			{
				string str = ChangeIntoOnDetach(slot_id);
				if (str  != "")
				//quantity override is needed here, otherwise optional
				{
					TurnItemIntoItemLambda lamb = new TurnItemIntoItemLambda(this, str, player);
					lamb.SetTransferParams(true, true, true, false,1);
					MiscGameplayFunctions.TurnItemIntoItemEx(player, lamb);
				}
			}*/
		}
	}
	
	override string ChangeIntoOnAttach(string slot)
	{
		int idx;
		TStringArray inventory_slots = new TStringArray;
		TStringArray attach_types = new TStringArray;
		
		ConfigGetTextArray("ChangeInventorySlot",inventory_slots);
		if (inventory_slots.Count() < 1) //is string
		{
			inventory_slots.Insert(ConfigGetString("ChangeInventorySlot"));
			attach_types.Insert(ConfigGetString("ChangeIntoOnAttach"));
		}
		else //is array
		{
			ConfigGetTextArray("ChangeIntoOnAttach",attach_types);
		}
		
		idx = inventory_slots.Find(slot);
		if (idx < 0)
			return "";
		
		return attach_types.Get(idx);
	}
	
	override string ChangeIntoOnDetach()
	{
		int idx = -1;
		string slot;
		
		TStringArray inventory_slots = new TStringArray;
		TStringArray detach_types = new TStringArray;
		
		this.ConfigGetTextArray("ChangeInventorySlot",inventory_slots);
		if (inventory_slots.Count() < 1) //is string
		{
			inventory_slots.Insert(this.ConfigGetString("ChangeInventorySlot"));
			detach_types.Insert(this.ConfigGetString("ChangeIntoOnDetach"));
		}
		else //is array
		{
			this.ConfigGetTextArray("ChangeIntoOnDetach",detach_types);
			if (detach_types.Count() < 1)
				detach_types.Insert(this.ConfigGetString("ChangeIntoOnDetach"));
		}
		
		for (int i = 0; i < inventory_slots.Count(); i++)
		{
			slot = inventory_slots.Get(i);
		}
		
		if (slot != "")
		{
			if (detach_types.Count() == 1)
				idx = 0;
			else
				idx = inventory_slots.Find(slot);
		}
		if (idx < 0)
			return "";
	
		return detach_types.Get(idx);
	}
	
	void ExplodeAmmo()
	{
		//timer
		ref Timer explode_timer = new Timer( CALL_CATEGORY_SYSTEM );
		
		//min/max time
		float min_time = 1;
		float max_time = 3;
		float delay = Math.RandomFloat( min_time, max_time );
		
		explode_timer.Run( delay, this, "DoAmmoExplosion" );
	}
	
	void DoAmmoExplosion()
	{
		Magazine magazine = Magazine.Cast( this );
		int pop_sounds_count = 6;
		string pop_sounds[ 6 ] = { "ammopops_1","ammopops_2","ammopops_3","ammopops_4","ammopops_5","ammopops_6" };
		
		//play sound
		int sound_idx = Math.RandomInt( 0, pop_sounds_count - 1 );
		string sound_name = pop_sounds[ sound_idx ];
		GetGame().CreateSoundOnObject( this, sound_name, 20, false );
		
		//remove ammo count
		magazine.ServerAddAmmoCount( -1 );
		
		//if condition then repeat -> ExplodeAmmo
		float min_temp_to_explode	= 100;		//min temperature for item to explode
				
		if ( magazine.GetAmmoCount() > 0 && GetTemperature() >= min_temp_to_explode )	//TODO ? add check for parent -> fireplace
		{
			ExplodeAmmo();
		}
	}
	
	// -------------------------------------------------------------------------------
	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		const int CHANCE_DAMAGE_CARGO = 4;
		const int CHANCE_DAMAGE_ATTACHMENT = 1;
		const int CHANCE_DAMAGE_NOTHING = 2;
		
		if ( IsClothing() || IsContainer() )
		{
			float dmg = damageResult.GetDamage("","Health") / -2;
			int chances;
			int rnd;
			
			if (GetInventory().GetCargo())
			{
				chances = CHANCE_DAMAGE_CARGO + CHANCE_DAMAGE_ATTACHMENT + CHANCE_DAMAGE_NOTHING;
				rnd = Math.RandomInt(0,chances);
				
				if (rnd < CHANCE_DAMAGE_CARGO)
				{
					DamageItemInCargo(dmg);
				}
				else if (rnd < (chances - CHANCE_DAMAGE_NOTHING) )
				{
					DamageItemAttachments(dmg);
				}
			}
			else
			{
				chances = CHANCE_DAMAGE_ATTACHMENT + CHANCE_DAMAGE_NOTHING;
				rnd = Math.RandomInt(0,chances);
				
				if (rnd < CHANCE_DAMAGE_ATTACHMENT)
				{
					DamageItemAttachments(dmg);
				}
			}
		}
	}
	
	bool DamageItemInCargo(float damage)
	{
		if ( GetInventory().GetCargo() )
		{
			int item_count = GetInventory().GetCargo().GetItemCount();
			if ( item_count > 0 )
			{
				int random_pick = Math.RandomInt(0, item_count);
				ItemBase item = ItemBase.Cast( GetInventory().GetCargo().GetItem(random_pick) );
				if (!item.IsExplosive())
				{
					item.AddHealth("","",damage);
					return true;
				}
			}
		}
		return false;
	}
	
	bool DamageItemAttachments(float damage)
	{
		int attachment_count = GetInventory().AttachmentCount();
		if ( attachment_count > 0 )
		{
			int random_pick = Math.RandomInt(0, attachment_count);
			ItemBase attachment = ItemBase.Cast( GetInventory().GetAttachmentFromIndex(random_pick));
			if (!attachment.IsExplosive())
			{
				attachment.AddHealth("","",damage);
				return true;
			}
		}
		return false;
	}
	
	//----------------
	bool CanBeSplit()
	{
		if( ConfigGetBool("canBeSplit") )
		{
			if( GetQuantity() > 1 )
			{
				return true;
			}
			else 
			{
				return false;
			}
		}
		
		return false;
	}
	
	void SplitIntoStackMaxClient( EntityAI destination_entity, int slot_id  )
	{
		if( GetGame().IsClient() )
		{
			if (ScriptInputUserData.CanStoreInputUserData())
			{
				ScriptInputUserData ctx = new ScriptInputUserData;
				ctx.Write(INPUT_UDT_ITEM_MANIPULATION);
				ctx.Write(1);
				ItemBase i1 = this; // @NOTE: workaround for correct serialization
				ctx.Write(i1);
				ctx.Write(destination_entity);
				ctx.Write(true);
				ctx.Write(slot_id);
				ctx.Send();
			}
		}
		else if( !GetGame().IsMultiplayer() )
		{
			SplitIntoStackMax( destination_entity, slot_id, PlayerBase.Cast( GetGame().GetPlayer() ) );
		}
	}

	void SplitIntoStackMax( EntityAI destination_entity, int slot_id, PlayerBase player )
	{
		float split_quantity_new;
		ref ItemBase new_item;
		float quantity = GetQuantity();
		float stackable = ConfigGetFloat("varStackMax");
		InventoryLocation loc = new InventoryLocation;
		
		if( destination_entity && slot_id != -1 && InventorySlots.IsSlotIdValid( slot_id ) )
		{
			int stack_max = InventorySlots.GetStackMaxForSlotId( slot_id );
			if( stack_max <= GetQuantity() )
				split_quantity_new = stack_max;
			else
				split_quantity_new = GetQuantity();
			
			new_item = ItemBase.Cast( destination_entity.GetInventory().CreateAttachmentEx( this.GetType(), slot_id ) );
			if( new_item )
			{			
				MiscGameplayFunctions.TransferItemProperties( this, new_item );
				AddQuantity( -split_quantity_new );
				new_item.SetQuantity( split_quantity_new );
			}
		}
		else if( destination_entity && slot_id == -1 )
		{
			if( quantity > stackable )
				split_quantity_new = stackable;
			else
				split_quantity_new = quantity;
			
			if (destination_entity.GetInventory().FindFreeLocationFor( this, FindInventoryLocationType.ANY, loc ))
			{
				Object o = destination_entity.GetInventory().LocationCreateEntity( loc, GetType(), ECE_IN_INVENTORY, RF_DEFAULT );
				new_item = ItemBase.Cast( o );
			}

			if( new_item )
			{			
				MiscGameplayFunctions.TransferItemProperties( this, new_item );
				AddQuantity( -split_quantity_new );
				new_item.SetQuantity( split_quantity_new );
			}
		}
		else
		{
			if( stackable != 0 )
			{
				if( stackable < GetQuantity() )
				{
					split_quantity_new = GetQuantity() - stackable;
				}
				
				if( split_quantity_new == 0 )
				{
					if( !GetGame().IsMultiplayer() )
						player.PredictiveDropEntity( this );
					else
						player.ServerDropEntity( this );
					return;
				}
				
				EntityAI parent = GetHierarchyParent();
				
				if( parent )
				{
					new_item = ItemBase.Cast( SpawnEntityOnGroundPos( GetType(), parent.GetWorldPosition() ) );
					new_item.PlaceOnSurface();
				}
				else
					new_item = ItemBase.Cast( GetGame().CreateObject( GetType(), player.GetWorldPosition() ) );
				
				if( new_item )
				{
					MiscGameplayFunctions.TransferItemProperties( this, new_item );
					SetQuantity( split_quantity_new );
					new_item.SetQuantity( stackable );
					new_item.PlaceOnSurface();
				}
			}
		}
	}
	
	void SplitIntoStackMaxToInventoryLocationClient ( notnull InventoryLocation dst )
	{
		if( GetGame().IsClient() )
		{
			if (ScriptInputUserData.CanStoreInputUserData())
			{
				ScriptInputUserData ctx = new ScriptInputUserData;
				ctx.Write(INPUT_UDT_ITEM_MANIPULATION);
				ctx.Write(4);
				ItemBase thiz = this; // @NOTE: workaround for correct serialization
				ctx.Write(thiz);
				dst.WriteToContext(ctx);
				ctx.Send();
			}
		}
		else if( !GetGame().IsMultiplayer() )
		{
			SplitIntoStackMaxToInventoryLocation( dst );
		}
	}
	
	void SplitIntoStackMaxCargoClient( EntityAI destination_entity, int idx, int row, int col )
	{
		if( GetGame().IsClient() )
		{
			if (ScriptInputUserData.CanStoreInputUserData())
			{
				ScriptInputUserData ctx = new ScriptInputUserData;
				ctx.Write(INPUT_UDT_ITEM_MANIPULATION);
				ctx.Write(2);
				ItemBase dummy = this; // @NOTE: workaround for correct serialization
				ctx.Write(dummy);
				ctx.Write(destination_entity);
				ctx.Write(true);
				ctx.Write(idx);
				ctx.Write(row);
				ctx.Write(col);
				ctx.Send();
			}
		}
		else if( !GetGame().IsMultiplayer() )
		{
			SplitIntoStackMaxCargo( destination_entity, idx, row, col );
		}
	}

	void SplitIntoStackMaxToInventoryLocation ( notnull InventoryLocation dst )
	{
		float quantity = GetQuantity();
		float split_quantity_new;
		ref ItemBase new_item;
		if( dst.IsValid() )
		{
			float stackable = ConfigGetFloat("varStackMax");
			if( quantity > stackable )
				split_quantity_new = stackable;
			else
				split_quantity_new = quantity;
			
			new_item = ItemBase.Cast( GameInventory.LocationCreateEntity( dst, this.GetType(), ECE_IN_INVENTORY, RF_DEFAULT ) );
			
			if( new_item )
			{			
				MiscGameplayFunctions.TransferItemProperties(this,new_item);
				AddQuantity( -split_quantity_new );
				new_item.SetQuantity( split_quantity_new );
			}
		}
	}
	
	void SplitIntoStackMaxCargo( EntityAI destination_entity, int idx, int row, int col )
	{
		float quantity = GetQuantity();
		float split_quantity_new;
		ref ItemBase new_item;
		if( destination_entity )
		{
			float stackable = ConfigGetFloat("varStackMax");
			if( quantity > stackable )
				split_quantity_new = stackable;
			else
				split_quantity_new = quantity;
			
			new_item = ItemBase.Cast( destination_entity.GetInventory().CreateEntityInCargoEx( this.GetType(), idx, row, col, false ) );
			if( new_item )
			{			
				MiscGameplayFunctions.TransferItemProperties(this,new_item);
				AddQuantity( -split_quantity_new );
				new_item.SetQuantity( split_quantity_new );
			}
		}
	}
	
	void SplitIntoStackMaxHandsClient( PlayerBase player )
	{
		if( GetGame().IsClient() )
		{
			if (ScriptInputUserData.CanStoreInputUserData())
			{
				ScriptInputUserData ctx = new ScriptInputUserData;
				ctx.Write(INPUT_UDT_ITEM_MANIPULATION);
				ctx.Write(3);
				ItemBase i1 = this; // @NOTE: workaround for correct serialization
				ctx.Write(i1);
				ItemBase destination_entity = this;
				ctx.Write(destination_entity);
				ctx.Write(true);
				ctx.Write(0);
				ctx.Send();
			}
		}
		else if( !GetGame().IsMultiplayer() )
		{
			SplitIntoStackMaxHands( player );
		}
	}

	void SplitIntoStackMaxHands( PlayerBase player )
	{
		float quantity = GetQuantity();
		float split_quantity_new;
		ref ItemBase new_item;
		if( player )
		{
			float stackable = ConfigGetFloat("varStackMax");
			if( quantity > stackable )
				split_quantity_new = stackable;
			else
				split_quantity_new = quantity;
			
			EntityAI in_hands = player.GetHumanInventory().CreateInHands(this.GetType());
			new_item = ItemBase.Cast(in_hands);
			if( new_item )
			{			
				MiscGameplayFunctions.TransferItemProperties(this,new_item);
				AddQuantity( -split_quantity_new );
				new_item.SetQuantity( split_quantity_new );
			}
		}
	}
	
	void SplitItem( PlayerBase player )
	{
		if ( !CanBeSplit() )
		{
			return;
		}
		
		float quantity = GetQuantity();
		float split_quantity_new = Math.Floor( quantity / 2 );
		
		InventoryLocation invloc = new InventoryLocation;
		bool found = player.GetInventory().FindFirstFreeLocationForNewEntity(GetType(), FindInventoryLocationType.ATTACHMENT, invloc);
		
		ItemBase new_item;
		new_item = player.CreateCopyOfItemInInventoryOrGround(this);
		if( new_item )
		{
			if (found && invloc.IsValid() && invloc.GetType() == InventoryLocationType.ATTACHMENT && split_quantity_new > 1)
			{
				AddQuantity(-1);
				new_item.SetQuantity(1);
			}
			else
			{
				AddQuantity(-split_quantity_new);
				new_item.SetQuantity( split_quantity_new );
			}
		}	
	}
	
	//! Called on server side when this item's quantity is changed. Call super.OnQuantityChanged(); first when overriding this event.
	void OnQuantityChanged()
	{
		ItemBase parent = ItemBase.Cast( GetHierarchyParent() );
		
		if (parent)
		{
			parent.OnAttachmentQuantityChanged(this);
		}
		UpdateWeight();
	}
	
	//! Called on server side when some attachment's quantity is changed. Call super.OnAttachmentQuantityChanged(item); first when overriding this event.
	void OnAttachmentQuantityChanged( ItemBase item )
	{
		// insert code here
	}
	
	override void OnRightClick()
	{
		super.OnRightClick();
		
		if( CanBeSplit() && !GetDayZGame().IsLeftCtrlDown() )
		{
			if( GetGame().IsClient() )
			{
				if (ScriptInputUserData.CanStoreInputUserData())
				{
					ScriptInputUserData ctx = new ScriptInputUserData;
					ctx.Write(INPUT_UDT_ITEM_MANIPULATION);
					ctx.Write(1);
					ItemBase i1 = this; // @NOTE: workaround for correct serialization
					ctx.Write(i1);
					ItemBase i2 = null; // @NOTE: workaround for correct serialization
					ctx.Write(i2);
					ctx.Write(false);
					ctx.Write(-1);
					ctx.Send();
				}
			}
			else if( !GetGame().IsMultiplayer() )
			{
				SplitItem( PlayerBase.Cast( GetGame().GetPlayer() ) );
			}
		}
	}
	
	bool CanBeCombined( EntityAI other_item, PlayerBase player = null )
	{
		bool can_this_be_combined = ConfigGetBool("canBeSplit");
		
		if ( GetInventory().HasInventoryReservation( this, null ) || other_item.GetInventory().HasInventoryReservation( other_item, null ))
			return false;
		
		if( CastTo( player, GetHierarchyRootPlayer() ) ) //false when attached to player's attachment slot
		{
			if( player.GetInventory().HasAttachment( this ) )
			{
				return false;
			}
		}
		
		if( !can_this_be_combined || !other_item ) 
		{
			return false;
		}
		
		if( IsFullQuantity() )
		{
			return false;
		}
		
		//if( player && (player.GetHumanInventory().GetEntityInHands() == this || player.GetHumanInventory().GetEntityInHands() == other_item )) return false;
		
		return ( this.GetType() == other_item.GetType() );
	}
	
	bool IsCombineAll( ItemBase other_item, bool use_stack_max = false )
	{
		return ComputeQuantityUsed( other_item, use_stack_max ) == other_item.GetQuantity();
	}
	
	int ComputeQuantityUsed( ItemBase other_item, bool use_stack_max = false )
	{
		float other_item_quantity = other_item.GetQuantity();
		float this_free_space;
			
		int max_quantity;
		int stack_max;
		
		InventoryLocation il = new InventoryLocation;
		GetInventory().GetCurrentInventoryLocation( il );
		if( il.GetSlot() != -1 )
			stack_max = InventorySlots.GetStackMaxForSlotId( il.GetSlot() );
		if( use_stack_max && stack_max > 0 )
		{
			max_quantity = stack_max;
		}
		else
		{
			max_quantity = GetQuantityMax();
		}
		this_free_space = max_quantity - GetQuantity();
			
		if( other_item_quantity > this_free_space )
		{
			return this_free_space;
		}
		else
		{
			return other_item_quantity;
		}
	}
	
	void CombineItems( ItemBase other_item, bool use_stack_max = false )
	{
		if( !CanBeCombined(other_item) )
			return;
		
		if( !IsMagazine() && other_item )
		{
			int quantity_used = ComputeQuantityUsed(other_item,use_stack_max);
			if( quantity_used != 0 )
			{
				this.AddQuantity(quantity_used);
				other_item.AddQuantity(-quantity_used);
			}
		}
	}
	// -------------------------------------------------------------------------
	// Mirek: whole user action system moved to script
	// -------------------------------------------------------------------------	
	void GetRecipesActions( Man player, out TSelectableActionInfoArray outputList )
	{
		PlayerBase p = PlayerBase.Cast( player );
			
		array<int> recipes_ids = p.m_Recipes;
		PluginRecipesManager module_recipes_manager = PluginRecipesManager.Cast( GetPlugin(PluginRecipesManager) );
		if( module_recipes_manager )
		{
			EntityAI item_in_hands = player.GetHumanInventory().GetEntityInHands();
			module_recipes_manager.GetValidRecipes( ItemBase.Cast( this ), ItemBase.Cast( item_in_hands ),recipes_ids, PlayerBase.Cast( player ) );
		}
		for(int i = 0;i < recipes_ids.Count();i++)
		{
			int key = recipes_ids.Get(i);
			string recipe_name = module_recipes_manager.GetRecipeName(key);
			outputList.Insert( new TSelectableActionInfo( SAT_CRAFTING, key, recipe_name ) );
		}
	}
	
	// -------------------------------------------------------------------------	
	void GetDebugActions(out TSelectableActionInfoArray outputList)
	{
		//weight
		outputList.Insert(new TSelectableActionInfo(SAT_DEBUG_ACTION, EActions.GET_TOTAL_WEIGHT, "Print Weight"));
		
		//quantity
		outputList.Insert(new TSelectableActionInfo(SAT_DEBUG_ACTION, EActions.REMOVE_QUANTITY, "Quantity -20%"));
		outputList.Insert(new TSelectableActionInfo(SAT_DEBUG_ACTION, EActions.ADD_QUANTITY, "Quantity +20%"));
		outputList.Insert(new TSelectableActionInfo(SAT_DEBUG_ACTION, EActions.SET_QUANTITY_0, "Set Quantity 0"));
		outputList.Insert(new TSelectableActionInfo(SAT_DEBUG_ACTION, EActions.SET_MAX_QUANTITY, "Set Quantity Max"));
		
		//health
		outputList.Insert(new TSelectableActionInfo(SAT_DEBUG_ACTION, EActions.ADD_HEALTH, "Health +20"));
		outputList.Insert(new TSelectableActionInfo(SAT_DEBUG_ACTION, EActions.REMOVE_HEALTH, "Health -20"));

		//temperature
		outputList.Insert(new TSelectableActionInfo(SAT_DEBUG_ACTION, EActions.ADD_TEMPERATURE, "Temperature +20"));
		outputList.Insert(new TSelectableActionInfo(SAT_DEBUG_ACTION, EActions.REMOVE_TEMPERATURE, "Temperature -20"));
		
		//wet
		outputList.Insert(new TSelectableActionInfo(SAT_DEBUG_ACTION, EActions.ADD_WETNESS, "Wetness +20"));
		outputList.Insert(new TSelectableActionInfo(SAT_DEBUG_ACTION, EActions.REMOVE_WETNESS, "Wetness -20"));

		//liquidtype
		if( IsLiquidContainer() )
		{
			outputList.Insert(new TSelectableActionInfo(SAT_DEBUG_ACTION, EActions.LIQUIDTYPE_UP, "LiquidType Next"));
			outputList.Insert(new TSelectableActionInfo(SAT_DEBUG_ACTION, EActions.LIQUIDTYPE_DOWN, "LiquidType Previous"));
		}
		
		//strings
		outputList.Insert(new TSelectableActionInfo(SAT_DEBUG_ACTION, EActions.INJECT_STRING_TIGER, "Inject String Tiger"));
		outputList.Insert(new TSelectableActionInfo(SAT_DEBUG_ACTION, EActions.INJECT_STRING_RABBIT, "Inject String Rabbit"));
		
		// watch
		outputList.Insert(new TSelectableActionInfo(SAT_DEBUG_ACTION, EActions.WATCH_ITEM, "Watch"));

		// print bullets
		//outputList.Insert(new TSelectableActionInfo(SAT_DEBUG_ACTION, EActions.PRINT_BULLETS, "Print Bullets"));
		
		//ctx.AddAction("ShowID",USE_ENUM_HERE,NULL,2000,false,false);
		//ctx.AddAction("Predend Consume",USE_ENUM_HERE,NULL, 1011,false,false);
		//ctx.AddAction("IsEmpty",USE_ENUM_HERE,NULL, 1012,false,false);
		//ctx.AddAction("HasAnyCargo",USE_ENUM_HERE,NULL, 1013,false,false);
		//ctx.AddAction("LightOn",USE_ENUM_HERE,NULL, 1013,false,false);
		//ctx.AddAction("Set Health 200", USE_ENUM_HERE, NULL, 1003, false, false);
		//ctx.AddAction("PrintQuantityMax", USE_ENUM_HERE, NULL, 1000, false, false);
		//ctx.AddAction("Print Classname", USE_ENUM_HERE, NULL, 1000, false, false);
		//ctx.AddAction("Has In Cargo", USE_ENUM_HERE, NULL, 1000, false, false);
		//ctx.AddAction("Print Health(new)", USE_ENUM_HERE, NULL, 1000, false, false);
		//ctx.AddAction("Quantity -1", USE_ENUM_HERE, NULL, 1004, false, false);
		//ctx.AddAction("IsMagazine", USE_ENUM_HERE, NULL, 1005, false, false);
		//ctx.AddAction("Ammo +1", USE_ENUM_HERE, NULL, 1006, false, false);
		//ctx.AddAction("Set Health 1", USE_ENUM_HERE, NULL, 1002, false, false);
		//ctx.AddAction("Ammo -1", USE_ENUM_HERE, NULL, 1007, false, false);
		//ctx.AddAction("Ammo SetMax", USE_ENUM_HERE, NULL, 1008, false, false);
		//ctx.AddAction("LightOff",USE_ENUM_HERE,NULL, 1022,false,false);
	}
	
	// -------------------------------------------------------------------------	
	// -------------------------------------------------------------------------		
	// -------------------------------------------------------------------------	
	bool OnAction(int action_id, Man player, ParamsReadContext ctx)
	{
		if(action_id >= EActions.RECIPES_RANGE_START && action_id < EActions.RECIPES_RANGE_END)
		{
			PluginRecipesManager plugin_recipes_manager = PluginRecipesManager.Cast( GetPlugin(PluginRecipesManager) );
			int idWithoutOffset = action_id - EActions.RECIPES_RANGE_START;
			PlayerBase p = PlayerBase.Cast( player );
			if( EActions.RECIPES_RANGE_START  < 1000 )
			{
				float anim_length = plugin_recipes_manager.GetRecipeLengthInSecs(idWithoutOffset);
				float specialty_weight = plugin_recipes_manager.GetRecipeSpecialty(idWithoutOffset);
				//p.SetUpCrafting( idWithoutOffset, this, player.GetHierarchyRootPlayer().GetHumanInventory().GetEntityInHands(),anim_length, specialty_weight);
			}
			else//this part is for the [DEBUG] actions
			{
				//plugin_recipes_manager.PerformRecipeClientRequest( idWithoutOffset, this, player.GetHierarchyRootPlayer().GetHumanInventory().GetEntityInHands() );
			}
		}
		
		if( GetGame().IsServer() )
		{
			if(action_id >= EActions.DEBUG_AGENTS_RANGE_INJECT_START && action_id < EActions.DEBUG_AGENTS_RANGE_INJECT_END)
			{
				int agent_id = action_id - EActions.DEBUG_AGENTS_RANGE_INJECT_START;
				InsertAgent(agent_id,100);
			}
	
			if(action_id >= EActions.DEBUG_AGENTS_RANGE_REMOVE_START && action_id < EActions.DEBUG_AGENTS_RANGE_REMOVE_END)
			{
				int agent_id2 = action_id - EActions.DEBUG_AGENTS_RANGE_REMOVE_START;
				RemoveAgent(agent_id2);
			}
			
			if( action_id == EActions.SET_MAX_QUANTITY ) //SetMaxQuantity
			{
				SetQuantityMax();
			}
			
			
			if( action_id == EActions.REMOVE_QUANTITY ) //Quantity -20%
			{
				AddQuantity(GetQuantityMax()/-5);
				//PrintVariables();
			}
			
			if( action_id == EActions.GET_TOTAL_WEIGHT ) //Prints total weight of item + its contents
			{
				Print(GetWeight());
			}
	
			/*
			if( action_id == EActions.INJECT_STRING_TIGER ) 
			{
				SetItemVariableString("varNote", "The tiger (Panthera tigris) is the largest cat species, most recognisable for their pattern of dark vertical stripes on reddish-orange fur with a lighter underside. The largest tigers have reached a total body length of up to 3.38 m (11.1 ft) over curves and have weighed up to 388.7 kg (857 lb) in the wild. The species is classified in the genus Panthera with the lion, leopard, jaguar and snow leopard. Tigers are apex predators, primarily preying on ungulates such as deer and bovids. They are territorial and generally solitary but social animals, often requiring large contiguous areas of habitat that support their prey requirements. This, coupled with the fact that they are indigenous to some of the more densely populated places on Earth, has caused significant conflicts with humans.");
			}
			if( action_id == EActions.INJECT_STRING_RABBIT ) 
			{
				SetItemVariableString("varNote", "Rabbits are small mammals in the family Leporidae of the order Lagomorpha, found in several parts of the world. There are eight different genera in the family classified as rabbits, including the European rabbit (Oryctolagus cuniculus), cottontail rabbits (genus Sylvilagus; 13 species), and the Amami rabbit (Pentalagus furnessi, an endangered species on Amami ?shima, Japan). There are many other species of rabbit, and these, along with pikas and hares, make up the order Lagomorpha. The male is called a buck and the female is a doe; a young rabbit is a kitten or kit.");
			}
			*/
	
			if( action_id == EActions.ADD_HEALTH ) 
			{
				this.AddHealth("","",20);
			}
			if( action_id == EActions.REMOVE_HEALTH ) 
			{
				this.AddHealth("","",-20);
			}
			
			if( action_id == EActions.SET_QUANTITY_0 ) //SetMaxQuantity
			{
				SetQuantity(0);
			}
			
			if( action_id == EActions.WATCH_ITEM ) //SetMaxQuantity
			{
				PluginItemDiagnostic mid = PluginItemDiagnostic.Cast( GetPlugin(PluginItemDiagnostic) );
				mid.RegisterDebugItem( ItemBase.Cast( this ), PlayerBase.Cast( player ));
			}
			
			if( action_id == EActions.ADD_QUANTITY )
			{
				AddQuantity(GetQuantityMax()/5);
				//PrintVariables();
			}
	
			if( action_id == EActions.ADD_TEMPERATURE )
			{
				AddTemperature(1);
				//PrintVariables();
			}
			
			if( action_id == EActions.REMOVE_TEMPERATURE )
			{
				AddTemperature(-1);
				//PrintVariables();
			}
			
			if( action_id == EActions.ADD_WETNESS )
			{
				AddWet(GetWetMax()/5);
				//PrintVariables();
			}
			
			if( action_id == EActions.REMOVE_WETNESS )
			{
				AddWet(-GetWetMax()/5);
				//PrintVariables();
			}
	
			if( action_id == EActions.LIQUIDTYPE_UP )
			{
				int curr_type = GetLiquidType();
				SetLiquidType(curr_type * 2);
				//AddWet(1);
				//PrintVariables();
			}
			
			if( action_id == EActions.LIQUIDTYPE_DOWN )
			{
				int curr_type2 = GetLiquidType();
				SetLiquidType(curr_type2 / 2);
			}

			if( action_id == EActions.PRINT_BULLETS )
			{
				if( IsMagazine() )
				{
					Magazine this_mag;
					Class.CastTo(this_mag, this);
					for(int i = 0; i < this_mag.GetAmmoCount(); i++)
					{
						float damage;
						string class_name;
						this_mag.GetCartridgeAtIndex(i, damage, class_name);
						PrintString("Bullet: " + class_name +", " + "Damage: "+ damage.ToString() );
					}					
				}
			}
		}

		
		return false;
	}

	// -------------------------------------------------------------------------
	
	
	//! Called when this item is activated from a trip wire that was stepped on.
	void OnActivatedByTripWire()
	{
		// Override this method for Trip Wire functionality
	}

	//----------------------------------------------------------------
	//returns true if item is able to explode when put in fire
	bool CanExplodeInFire()
	{
		return false;
	}
	
	//----------------------------------------------------------------
	bool CanEat()
	{
		return true;
	}
	
	//----------------------------------------------------------------
	//has FoodStages in config?
	bool HasFoodStage()
	{
		string config_path = "CfgVehicles" + " " + GetType() + " " + "Food" + " " + "FoodStages";
		return GetGame().ConfigIsExisting ( config_path );
	}
	
	bool CanBeCooked()
	{
		return false;
	}
	
	bool CanBeCookedOnStick()
	{
		return false;
	}		
	
	//----------------------------------------------------------------
	bool CanRepair(ItemBase item_repair_kit)
	{
		PluginRepairing module_repairing = PluginRepairing.Cast( GetPlugin(PluginRepairing) );
		return module_repairing.CanRepair(this, item_repair_kit);
	}

	//----------------------------------------------------------------
	bool Repair(PlayerBase player, ItemBase item_repair_kit, float specialty_weight)
	{
		PluginRepairing module_repairing = PluginRepairing.Cast( GetPlugin(PluginRepairing) );
		return module_repairing.Repair(player, this, item_repair_kit, specialty_weight);
	}

	//----------------------------------------------------------------
	int GetItemSize()
	{
		/*
		vector v_size = this.ConfigGetVector("itemSize");
		int v_size_x = v_size[0];
		int v_size_y = v_size[1];
		int size = v_size_x * v_size_y;
		return size;
		*/
		
		return 1;
	}

	//----------------------------------------------------------------
	/**
	\brief Send message to owner player in grey color
		\return \p void
		@code
			item_stone.MessageToOwnerStatus("Some Status Message");
		@endcode
	*/
	void MessageToOwnerStatus( string text )
	{
		PlayerBase player = PlayerBase.Cast( this.GetHierarchyRootPlayer() );
		
		if ( player )
		{
			player.MessageStatus( text );
		}
	}

	//----------------------------------------------------------------
	/**
	\brief Send message to owner player in yellow color
		\return \p void
		@code
			item_stone.MessageToOwnerAction("Some Action Message");
		@endcode
	*/
	void MessageToOwnerAction( string text )
	{
		PlayerBase player = PlayerBase.Cast( this.GetHierarchyRootPlayer() );
		
		if ( player )
		{
			player.MessageAction( text );
		}
	}

	//----------------------------------------------------------------
	/**
	\brief Send message to owner player in green color
		\return \p void
		@code
			item_stone.MessageToOwnerFriendly("Some Friendly Message");
		@endcode
	*/
	void MessageToOwnerFriendly( string text )
	{
		PlayerBase player = PlayerBase.Cast( this.GetHierarchyRootPlayer() );
		
		if ( player )
		{
			player.MessageFriendly( text );
		}
	}

	//----------------------------------------------------------------
	/**
	\brief Send message to owner player in red color
		\return \p void
		@code
			item_stone.MessageToOwnerImportant("Some Important Message");
		@endcode
	*/
	void MessageToOwnerImportant( string text )
	{
		PlayerBase player = PlayerBase.Cast( this.GetHierarchyRootPlayer() );
		
		if ( player )
		{
			player.MessageImportant( text );
		}
	}

	override bool IsItemBase()
	{
		return true;
	}

	// Checks if item is of questioned kind
	override bool KindOf( string tag )
	{
		bool found = false;
		string item_name = this.GetType();
		ref TStringArray item_tag_array = new TStringArray;
		GetGame().ConfigGetTextArray("cfgVehicles " + item_name + " itemInfo", item_tag_array);	
		
		int array_size = item_tag_array.Count();
		for (int i = 0; i < array_size; i++)
		{
			if ( item_tag_array.Get(i) == tag )
			{
				found = true;
				break;
			}
		}
		return found;
	}

	override void OnRPC(PlayerIdentity sender, int rpc_type,ParamsReadContext  ctx) 
	{
		//Debug.Log("OnRPC called");
		super.OnRPC(sender, rpc_type,ctx);
		//if( rpc_type == ERPCs.RPC_ITEM_SPLIT ) SplitItem();
		/*
		if( rpc_type == ERPCs.RPC_ITEM_COMBINE ) 
		{

			ref Param1<EntityAI> item = new Param1<EntityAI>(NULL);

			ctx.Read(item);
			ItemBase item_other = item.param1;
			
			CombineItems( item_other );
		}
		*/
		if( GetGame().IsDebug() )
		{
			if( rpc_type == ERPCs.RPC_ITEM_DIAG )
			{	
				PluginItemDiagnostic mid = PluginItemDiagnostic.Cast( GetPlugin(PluginItemDiagnostic) );
				mid.OnRPC(this,ctx);
			}
		}
	}

	//-----------------------------
	// VARIABLE MANIPULATION SYSTEM
	//-----------------------------

	void TransferVariablesFloat(array<float> float_vars)
	{
		DeSerializeNumericalVars(float_vars);
	}
		
	array<float> GetVariablesFloat()
	{
		CachedObjectsArrays.ARRAY_FLOAT.Clear();
		SerializeNumericalVars(CachedObjectsArrays.ARRAY_FLOAT);
		return CachedObjectsArrays.ARRAY_FLOAT;
	}

	int NameToID(string name)
	{
		PluginVariables plugin = PluginVariables.Cast( GetPlugin(PluginVariables) );
		return plugin.GetID(name);
	}

	string IDToName(int id)
	{
		PluginVariables plugin = PluginVariables.Cast( GetPlugin(PluginVariables) );
		return plugin.GetName(id);
	}

	void OnSyncVariables(ParamsReadContext ctx)//with ID optimization
	{
		//Debug.Log("OnSyncVariables called for item:  "+ ToString(this.GetType()),"varSync");
		//read the flags
		//ref Param1<int> pflags = new Param1<int>(0);
		ctx.Read(CachedObjectsParams.PARAM1_INT);
		
		int varFlags = CachedObjectsParams.PARAM1_INT.param1;
		//--------------
		
		
		if( varFlags & ItemVariableFlags.FLOAT )
		{
			ReadVarsFromCTX(ctx);
		}
		/*
		if( varFlags & ItemVariableFlags.STRING )
		{
			OnSyncStrings(ctx);
		}
		*/
	}	
		
	void SerializeNumericalVars(array<float> floats_out)
	{
		// the order of serialization must be the same as the order of de-serialization
		floats_out.Insert(m_VariablesMask);
		//--------------------------------------------
		if( IsVariableSet(VARIABLE_QUANTITY) )
		{
			floats_out.Insert(m_VarQuantity);
		}
		//--------------------------------------------
		if( IsVariableSet(VARIABLE_TEMPERATURE) )
		{
			floats_out.Insert(m_VarTemperature);
		}
		//--------------------------------------------
		if( IsVariableSet(VARIABLE_WET) )
		{
			floats_out.Insert(m_VarWet);
		}
		//--------------------------------------------
		if( IsVariableSet(VARIABLE_LIQUIDTYPE) )
		{
			floats_out.Insert(m_VarLiquidType);
		}
		//--------------------------------------------
		if( IsVariableSet(VARIABLE_COLOR) )
		{
			floats_out.Insert(m_ColorComponentR);
			floats_out.Insert(m_ColorComponentG);
			floats_out.Insert(m_ColorComponentB);
			floats_out.Insert(m_ColorComponentA);
		}
		//--------------------------------------------
	}
	
	void DeSerializeNumericalVars(array<float> floats)
	{
		// the order of serialization must be the same as the order of de-serialization
		int index = 0;
		int mask = Math.Round(floats.Get(index));
		
		index++;
		//--------------------------------------------
		if( mask & VARIABLE_QUANTITY )
		{
			float quantity = floats.Get(index);
			SetQuantity(quantity, true );
			index++;
		}
		//--------------------------------------------
		if( mask & VARIABLE_TEMPERATURE )
		{
			float temperature = floats.Get(index);
			SetTemperature(temperature);
			index++;
		}
		//--------------------------------------------
		if( mask & VARIABLE_WET )
		{
			float wet = floats.Get(index);
			SetWet(wet);
			index++;
		}
		//--------------------------------------------
		if( mask & VARIABLE_LIQUIDTYPE )
		{
			int liquidtype = Math.Round(floats.Get(index));
			SetLiquidType(liquidtype);
			index++;
		}
		//--------------------------------------------
		if( mask & VARIABLE_COLOR )
		{
			m_ColorComponentR = Math.Round(floats.Get(index));
			index++;
			m_ColorComponentG = Math.Round(floats.Get(index));
			index++;
			m_ColorComponentB = Math.Round(floats.Get(index));
			index++;
			m_ColorComponentA = Math.Round(floats.Get(index));
			index++;
		}
	}

	
	void WriteVarsToCTX(ParamsWriteContext ctx)
	{
		CachedObjectsArrays.ARRAY_FLOAT.Clear();
		SerializeNumericalVars(CachedObjectsArrays.ARRAY_FLOAT);
		
		int array_size = CachedObjectsArrays.ARRAY_FLOAT.Count();
		
		CachedObjectsParams.PARAM1_INT.param1 = array_size;
		ctx.Write(CachedObjectsParams.PARAM1_INT);
		
		for(int i = 0; i < array_size; i++)
		{
			CachedObjectsParams.PARAM1_FLOAT.param1 = CachedObjectsArrays.ARRAY_FLOAT.Get(i);
			ctx.Write(CachedObjectsParams.PARAM1_FLOAT);
		}
	}
	
	bool ReadVarsFromCTX(ParamsReadContext ctx)//with ID optimization
	{
		if(!ctx.Read(CachedObjectsParams.PARAM1_INT))
			return false;
		
		int numOfItems = CachedObjectsParams.PARAM1_INT.param1;
		CachedObjectsArrays.ARRAY_FLOAT.Clear();
		
		for(int i = 0; i < numOfItems; i++)
		{
			if(!ctx.Read(CachedObjectsParams.PARAM1_FLOAT))
				return false;
			float value = CachedObjectsParams.PARAM1_FLOAT.param1;
			
			CachedObjectsArrays.ARRAY_FLOAT.Insert(value);
		}
		
		DeSerializeNumericalVars(CachedObjectsArrays.ARRAY_FLOAT);
		return true;
	}
	
	void SaveVariables(ParamsWriteContext ctx)
	{
		//Debug.Log("Saving Item Stage 0 "+ClassName(this)+" " + ToString(this));
		
		//first set the flags

		int varFlags = 0;
		
		if( m_VariablesMask )
		{
			varFlags = varFlags | ItemVariableFlags.FLOAT;
		}

		//ref Param1<int> pflags = new Param1<int>( varFlags );
		CachedObjectsParams.PARAM1_INT.param1 = varFlags;
		ctx.Write(CachedObjectsParams.PARAM1_INT);
		//-------------------
			
		//now serialize the variables
		
		//floats
		if( varFlags & ItemVariableFlags.FLOAT )
		{
			WriteVarsToCTX(ctx);
		}

	}

		
	//----------------------------------------------------------------
	bool LoadVariables(ParamsReadContext ctx, int version)
	{
		//read the flags
		if(!ctx.Read(CachedObjectsParams.PARAM1_INT))
		{
			return false;
		}
		else
		{
			int varFlags = CachedObjectsParams.PARAM1_INT.param1;
			//--------------
			if( varFlags & ItemVariableFlags.FLOAT )
			{
				if(!ReadVarsFromCTX(ctx))
					return false;
			}
		}
		return true;
	}


	//----------------------------------------------------------------
	override bool OnStoreLoad(ParamsReadContext ctx, int version)
	{   
		if ( !super.OnStoreLoad(ctx, version) )
			return false;

		PlayerBase player;
		int itemQBIndex;
		if(version == int.MAX)
		{
			if(!ctx.Read(itemQBIndex))
				return false;
		}
		else if( Class.CastTo(player, GetHierarchyRootPlayer()) )
		{
			//Load quickbar item bind
			if(!ctx.Read(itemQBIndex))
				return false;
			if( itemQBIndex != -1 && player )
			{
				player.SetLoadedQuickBarItemBind(this,itemQBIndex);
			}
		}
		
		// variable management system
		if ( !LoadVariables(ctx, version) )
			return false;

		//agent trasmission system
		if ( !LoadAgents(ctx, version) )
			return false;

		return true;
	}

	//----------------------------------------------------------------

	override void OnStoreSave(ParamsWriteContext ctx)
	{
		super.OnStoreSave(ctx);
		PlayerBase player;
		if(PlayerBase.CastTo(player,GetHierarchyRootPlayer()))
		{
			//Save quickbar item bind
			int itemQBIndex = -1;
			itemQBIndex = player.FindQuickBarEntityIndex(this);
			ctx.Write(itemQBIndex);			
		}
		SaveVariables(ctx);// variable management system
		SaveAgents(ctx);//agent trasmission system
	}
	//----------------------------------------------------------------

	override void OnVariablesSynchronized()
	{
		/*
		if(QUANTITY_DEBUG_REMOVE_ME)
		{
			PrintString("==================== CLIENT ==========================");
			int low, high;
			GetNetworkID(low, high);
			PrintString("entity:"+low.ToString()+"| high:"+high.ToString());
			PrintString("getting quantity, current:"+m_VarQuantity.ToString());
		}
		*/
		super.OnVariablesSynchronized();
	}
	
	//bool Consume(float amount, PlayerBase consumer);

	//-------------------------	Quantity
	//----------------------------------------------------------------
	//! Set item quantity[related to varQuantity... config entry], destroy_config = true > if the quantity reaches varQuantityMin or lower and the item config contains the varQuantityDestroyOnMin = true entry, the item gets destroyed. destroy_forced = true means item gets destroyed when quantity reaches varQuantityMin or lower regardless of config setting, returns true if the item gets deleted
	bool SetQuantity(float value, bool destroy_config = true, bool destroy_forced = false, bool allow_client = false)
	{
		if( !IsServerCheck(allow_client) ) return false;
		if( !HasQuantity() ) return false;
		if( IsLiquidContainer() && GetLiquidType() == 0 )
		{
			Debug.LogError("No LiquidType specified, try setting 'varLiquidTypeInit' to a particular liquid type");
			return false;
		}
		
		bool on_min_value = value <= (GetQuantityMin() + 0.001); //workaround, items with "varQuantityDestroyOnMin = true;" get destroyed
		
		if( on_min_value )
		{
			if( destroy_config )
			{
				bool dstr = ConfigGetBool("varQuantityDestroyOnMin");
				if( dstr )
				{
					this.Delete();
					return true;
				}
			}
			else if( destroy_forced )
			{
				this.Delete();
				return true;
			}
			// we get here if destroy_config IS true AND dstr(config destroy param) IS false;
			RemoveAllAgents();//we remove all agents when we got to the min value, but the item is not getting deleted
		}
		
		float min = GetQuantityMin();
		float max = GetQuantityMax();
		/*
		if(QUANTITY_DEBUG_REMOVE_ME)
		{
			PrintString("======================== SERVER ========================");
			int low, high;
			GetNetworkID(low, high);
			PrintString("entity:"+low.ToString()+"| high:"+high.ToString());
			PrintString("setting quantity, current:"+m_VarQuantity.ToString());
			PrintString("setting quantity, new:"+value.ToString());
		}*/
		
		m_VarQuantity = Math.Clamp(value, min, max);
		SetVariableMask(VARIABLE_QUANTITY);
		OnQuantityChanged();
		return false;
	}

	//----------------------------------------------------------------
	//! add item quantity[related to varQuantity... config entry], destroy_config = true > if the quantity reaches varQuantityMin or lower and the item config contains the varQuantityDestroyOnMin = true entry, the item gets destroyed. destroy_forced = true means item gets destroyed when quantity reaches varQuantityMin or lower regardless of config setting, returns true if the item gets deleted
	bool AddQuantity(float value, bool destroy_config = true, bool destroy_forced = false)
	{	
		return SetQuantity(GetQuantity() + value, destroy_config, destroy_forced);
	}
	//----------------------------------------------------------------
	void SetQuantityMax()
	{
		float max = GetQuantityMax();
		SetQuantity(max);
	}
	//----------------------------------------------------------------
	//! Sets quantity in normalized 0..1 form between the item's Min a Max values as defined by item's config(for Min 0 and Max 5000, setting 0.5 will result in value 2500)
	void SetQuantityNormalized(float value, bool destroy_config = true, bool destroy_forced = false)
	{
		float value_clamped = Math.Clamp(value, 0, 1);//just to make sure
		int result = Math.Round(Math.Lerp(GetQuantityMin(), GetQuantityMax(), value_clamped));
		SetQuantity(result, destroy_config, destroy_forced);
	}

	/*void SetAmmoNormalized(float value)
	{
		float value_clamped = Math.Clamp(value, 0, 1);
		Magazine this_mag = Magazine.Cast( this );
		int max_rounds = this_mag.GetAmmoMax();
		int result = value * max_rounds;//can the rounded if higher precision is required
		this_mag.SetAmmoCount(result);
	}*/
	//----------------------------------------------------------------
	int GetQuantityMax()
	{
		return ConfigGetInt("varQuantityMax");
	}
	//----------------------------------------------------------------
	int GetQuantityMin()
	{
		return ConfigGetInt("varQuantityMin");
	}
	//----------------------------------------------------------------
	int GetQuantityInit()
	{
		return ConfigGetInt("varQuantityInit");
	}
	
	bool HasQuantity()
	{
		if( GetQuantityMax() - GetQuantityMin() == 0 )
		{
			return false;			
		}
		else
		{
			return true;			
		}
	}

	float GetQuantity()
	{
		return m_VarQuantity;
	}
	
	bool IsFullQuantity()
	{
		InventoryLocation loc = new InventoryLocation;
		GetInventory().GetCurrentInventoryLocation(loc);
		int slot = loc.GetSlot();
		float stackable = ConfigGetFloat("varStackMax");
		if( slot != -1 )
		{
			float slot_stack = InventorySlots.GetStackMaxForSlotId( slot );
			if( slot_stack == 0 && stackable == 0 )
				slot_stack = GetQuantityMax();
			else if( slot_stack == 0 )
				slot_stack = stackable;
			
			if( GetQuantity() >= slot_stack )
			{
				return true;
			}
			return false;
		}
		else
		{
			if( stackable != 0 && GetQuantity() >= stackable )
			{
				return true;			
			}
			else if( GetQuantity() == GetQuantityMax() )
			{
				return true;			
			}
			return false;
		}
	}
	
	//Calculates weight of single item without attachments
	float GetSingleInventoryItemWeight()
	{
		int ConfWeight = ConfigGetInt("weight");
		float item_wetness = GetWet();
		float itemQuantity = 0;
		float Weight = 0;
		if (GetQuantity() != 0)
		{
			itemQuantity = GetQuantity();
		}

		if (ConfigGetBool("canBeSplit")) //quantity determines size of the stack
		{
			Weight = ((item_wetness + 1) * ConfWeight * itemQuantity);
		}
		else if (ConfigGetString("stackedUnit") == "cm") //duct tape, at the moment
		{
			int MaxLength = ConfigGetInt("varQuantityMax");
			Weight = ((item_wetness + 1) * ConfWeight * itemQuantity/MaxLength);
		}
		else if (itemQuantity != 1) //quantity determines weight of item without container (i.e. sardines in a can)
		{
			Weight = ((item_wetness + 1) * (ConfWeight + itemQuantity));
		}
		else
		{
			Weight = ((item_wetness + 1) * ConfWeight);
		}
		return Math.Round(Weight);
	}
	
	override void UpdateWeight()
	{
		int i = 0;
		float totalWeight;
		float item_wetness = this.GetWet();
		int ConfWeight;
		
		int AttachmentsCount = 0;
		CargoBase cargo;
		if (GetInventory())
		{
			AttachmentsCount = GetInventory().AttachmentCount();
			cargo = GetInventory().GetCargo();
		}
		
		ConfWeight = this.ConfigGetInt("weight");
		
		//attachments?
		if (AttachmentsCount > 0)
		{
			for (i = 0; i < AttachmentsCount; i++)
			{
				totalWeight += GetInventory().GetAttachmentFromIndex(i).GetWeight();
			}
		}
		
		//cargo?
		if (cargo != NULL )
		{
			for (i = 0; i < cargo.GetItemCount(); i++)
			{
				totalWeight += cargo.GetItem(i).GetWeight();
			}
		}

		//other
		{
			if (this.ConfigGetBool("canBeSplit")) //quantity determines size of the stack
			{
				totalWeight += Math.Round((item_wetness + 1) * this.GetQuantity() * ConfWeight);
			}
			else if (this.ConfigGetString("stackedUnit") == "cm") //duct tape, at the moment
			{
				int MaxLength = this.ConfigGetInt("varQuantityMax");
				totalWeight += Math.Round(((item_wetness + 1) * ConfWeight * (this.GetQuantity()/MaxLength)));
			}
			else //quantity determines weight of item without container (i.e. sardines in a can)
			{
				totalWeight += Math.Round((item_wetness + 1) * (this.GetQuantity() + ConfWeight));
			}
		}
		
		m_Weight = Math.Round(totalWeight);
	}

	//! Returns the number of items in cargo, otherwise returns 0(non-cargo objects). Recursive.
	int GetNumberOfItems()
	{
		int item_count = 0;
		ItemBase item;
		
		if ( GetInventory().GetCargo() != NULL )
		{
			item_count = GetInventory().GetCargo().GetItemCount();
		}
		
		for ( int i = 0; i < GetInventory().AttachmentCount(); i++ )
		{
			Class.CastTo(item,GetInventory().GetAttachmentFromIndex(i));
			if ( item )
				item_count += item.GetNumberOfItems();
		}
		return item_count;
	}
	
	void SetVariableMask(int variable)
	{
		m_VariablesMask = variable | m_VariablesMask; 
		if( GetGame().IsServer() ) 
		{
			SetSynchDirty();
		}
	}
	
	//!Removes variable from variable mask, making it appear as though the variable has never been changed from default
	void RemoveItemVariable(int variable)
	{
		m_VariablesMask = ~variable & m_VariablesMask;
	}
	
	//!'true' if this variable has ever been changed from default
	bool IsVariableSet(int variable)
	{
		return (variable & m_VariablesMask);
	}
	
	//-------------------------	Energy

	//----------------------------------------------------------------
	float GetEnergy()
	{
		float energy = 0;
		if( this.HasEnergyManager() )
		{
			energy = this.GetCompEM().GetEnergy();
		}
		return energy;
	}
	
	
	override void OnEnergyConsumed()
	{
		super.OnEnergyConsumed();
		
		ConvertEnergyToQuantity();
	}

	override void OnEnergyAdded() 
	{
		super.OnEnergyAdded();
		
		ConvertEnergyToQuantity();
	}
	
	// Converts energy (from Energy Manager) to quantity, if enabled.
	void ConvertEnergyToQuantity()
	{
		if ( GetGame().IsServer()  &&  HasEnergyManager()  &&  GetCompEM().HasConversionOfEnergyToQuantity() )
		{
			if ( HasQuantity() )
			{
				float energy_0to1 = GetCompEM().GetEnergy0To1();
				SetQuantityNormalized( energy_0to1 );
			}
		}
	}

	void SetTemperature(float value, bool allow_client = false)
	{
		if( !IsServerCheck(allow_client) ) return;
		float min = ConfigGetFloat("varTemperatureMin");
		float max = ConfigGetFloat("varTemperatureMax");
		
		m_VarTemperature = Math.Clamp(value, min, max);
		SetVariableMask(VARIABLE_TEMPERATURE);
	}
	//----------------------------------------------------------------
	void AddTemperature(float value)
	{
		SetTemperature( value + GetTemperature() );
	}
	//----------------------------------------------------------------
	void SetTemperatureMax()
	{
		float max = ConfigGetFloat("varTemperatureMax");
		SetTemperature(max);
	}
	//----------------------------------------------------------------
	float GetTemperature()
	{
		return m_VarTemperature;
	}
	
	float GetTemperatureInit()
	{
		return ConfigGetFloat("varTemperatureInit");
	}
	
	float GetTemperatureMin()
	{
		return ConfigGetFloat("varTemperatureMin");
	}

	float GetTemperatureMax()
	{
		return ConfigGetFloat("varTemperatureMax");
	}
	//----------------------------------------------------------------
	float GetHeatIsolationInit()
	{
		return ConfigGetFloat("heatIsolation");
	}
	float GetHeatIsolation()
	{
		return m_HeatIsolation;
	}
	//----------------------------------------------------------------
	void SetWet(float value, bool allow_client = false)
	{
		if( !IsServerCheck(allow_client) ) return;
		float min = ConfigGetFloat("varWetMin");
		float max = ConfigGetFloat("varWetMax");
		
		m_VarWet = Math.Clamp(value, min, max);
		SetVariableMask(VARIABLE_WET);
	}
	//----------------------------------------------------------------
	void AddWet(float value)
	{
		SetWet( GetWet() + value );
	}
	//----------------------------------------------------------------
	void SetWetMax()
	{
		float max = ConfigGetFloat("varWetMax");
		SetWet(max);
	}
	//----------------------------------------------------------------
	override float GetWet()
	{
		return m_VarWet;
	}
	//----------------------------------------------------------------
	float GetWetMax()
	{
		return ConfigGetFloat("varWetMax");
	}
	//----------------------------------------------------------------
	float GetWetMin()
	{
		return ConfigGetFloat("varWetMin");
	}
	//----------------------------------------------------------------
	float GetWetInit()
	{
		return ConfigGetFloat("varWetInit");
	}
	//----------------------------------------------------------------
	float GetAbsorbencyInit()
	{
		return ConfigGetFloat("absorbency");
	}
	
	float GetAbsorbency()
	{
		return m_Absorbency;
	}
	//----------------------------------------------------------------
	bool IsServerCheck(bool allow_client)
	{
		if(allow_client) return true;
		if( GetGame().IsClient() && GetGame().IsMultiplayer() ) 
		{
			Error("Attempting to change variable client side, variables are supposed to be changed on server only !!");
			return false;
		}
		else
		{
			return true;			
		}
	}
	
	float GetItemModelLength()
	{
		if (ConfigIsExisting("itemModelLength"))
		{
			return ConfigGetFloat("itemModelLength");
		}
		return 0;
	}

	//----------------------------------------------------------------
	//-------------------------	Color
	// sets items color variable given color components
	void SetColor(int r, int g, int b, int a)
	{
		m_ColorComponentR = r;
		m_ColorComponentG = g;
		m_ColorComponentB = b;
		m_ColorComponentA = a;
		SetVariableMask(VARIABLE_COLOR);
	}
	//! gets item's color variable as components
	void GetColor(out int r,out int g,out int b,out int a)
	{
		r = m_ColorComponentR;
		g = m_ColorComponentG;
		b = m_ColorComponentB;
		a = m_ColorComponentA;
	}
	
	bool IsColorSet()
	{
		if(!IsVariableSet(VARIABLE_COLOR))
		{
			return false;
		}
		return true;
	}
	
	//! Returns item's PROCEDURAL color as formated string, i.e. "#(argb,8,8,3)color(0.15,0.15,0.15,1.0,CO)"
	string GetColorString()
	{
		int r,g,b,a;
		GetColor(r,g,b,a);
		r = r/255;
		g = g/255;
		b = b/255;
		a = a/255;
		string save_color = r.ToString()+","+g.ToString()+","+b.ToString()+","+a.ToString();
		string color = "#(argb,8,8,3)color("+save_color+",CO)";
		
		return color;
	}
	//----------------------------------------------------------------
	//-------------------------	LiquidType

	void SetLiquidType(int value, bool allow_client = false)
	{
		if( !IsServerCheck(allow_client) ) return;
		m_VarLiquidType = value;
		SetVariableMask(VARIABLE_LIQUIDTYPE);
	}
	
	int GetLiquidTypeInit()
	{
		return ConfigGetInt("varLiquidTypeInit");
	}
	
	int GetLiquidType()
	{
		return m_VarLiquidType;
	}

	// -------------------------------------------------------------------------
	//! Event called on item when it is placed in the player(Man) inventory, passes the owner as a parameter
	void OnInventoryEnter(Man player)
	{
		PlayerBase nplayer;
		if ( PlayerBase.CastTo(nplayer, player) )
		{
			//nplayer.OnItemInventoryEnter(this);
			nplayer.SetEnableQuickBarEntityShortcut(this,true);	
		}
	}
	
	// -------------------------------------------------------------------------
	//! Event called on item when it is removed from the player(Man) inventory, passes the old owner as a parameter
	void OnInventoryExit(Man player)
	{
		PlayerBase nplayer;
		if ( PlayerBase.CastTo(nplayer,player) )
		{		
			//nplayer.OnItemInventoryExit(this);
			nplayer.SetEnableQuickBarEntityShortcut(this,false);

		}
		
		if ( HasEnergyManager() )
		{
			GetCompEM().UpdatePlugState(); // Unplug the el. device if it's necesarry.
		}
	}

	// ADVANCED PLACEMENT EVENTS
	override void OnPlacementStarted( Man player ) 
	{
		super.OnPlacementStarted( player );
		
		SetTakeable(false);
	}
	
	override void OnPlacementComplete( Man player ) 
	{
		if ( m_AdminLog )
		{
			m_AdminLog.OnPlacementComplete( player, this );
		}
		
		super.OnPlacementComplete( player );
	}
		
	//-----------------------------
	// AGENT SYSTEM
	//-----------------------------
	//--------------------------------------------------------------------------
	bool ContainsAgent( int agent_id )
	{
		if( agent_id & m_AttachedAgents )
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	//--------------------------------------------------------------------------
	override void RemoveAgent( int agent_id )
	{
		if( ContainsAgent(agent_id) )
		{
			m_AttachedAgents = ~agent_id & m_AttachedAgents;
		}
	}

	//--------------------------------------------------------------------------
	override void RemoveAllAgents()
	{
		m_AttachedAgents = 0;
	}
	//--------------------------------------------------------------------------
	override void RemoveAllAgentsExcept(int agents_to_keep_mask)
	{
		m_AttachedAgents = m_AttachedAgents & agent_to_keep;
	}
	// -------------------------------------------------------------------------
	override void InsertAgent(int agent, float count = 1)
	{
		if( count < 1 )
			return;
		//Debug.Log("Inserting Agent on item: " + agent.ToString() +" count: " + count.ToString());
		m_AttachedAgents = (agent | m_AttachedAgents);
	}
	
	//!transfer agents from another item
	void TransferAgents(int agents)
	{
		m_AttachedAgents = (m_AttachedAgents | agents);
	}
	
	// -------------------------------------------------------------------------
	override int GetAgents()
	{
		return m_AttachedAgents;
	}
	//----------------------------------------------------------------------
	
	int GetContaminationType()
	{
		int contamination_type;
		
		const int CONTAMINATED_MASK = eAgents.CHOLERA | eAgents.INFLUENZA | eAgents.SALMONELLA | eAgents.BRAIN;
		const int POISONED_MASK = eAgents.FOOD_POISON | eAgents.CHEMICAL_POISON;
		const int NERVE_GAS_MASK = eAgents.NERVE_AGENT;
		const int DIRTY_MASK = eAgents.WOUND_AGENT;
		
		Edible_Base edible = Edible_Base.Cast(this);
		int agents = GetAgents();
		if(edible)
		{
			NutritionalProfile profile = Edible_Base.GetNutritionalProfile(edible);
			if(profile)
			{
				//Print("profile agents:" +profile.GetAgents());
				agents = agents | profile.GetAgents();//merge item's agents with nutritional agents
			}
		}
		if( agents & CONTAMINATED_MASK )
		{
			contamination_type = contamination_type | EContaminationTypes.ITEM_BADGE_CONTAMINATED;
		}
		if( agents & POISONED_MASK )
		{
			contamination_type = contamination_type | EContaminationTypes.ITEM_BADGE_POISONED;
		}
		if( agents & NERVE_GAS_MASK )
		{
			contamination_type = contamination_type | EContaminationTypes.ITEM_BADGE_NERVE_GAS;
		}
		if( agents & DIRTY_MASK )
		{
			contamination_type = contamination_type | EContaminationTypes.ITEM_BADGE_DIRTY;
		}
		
		return agents;
	}
	
	// -------------------------------------------------------------------------
	bool LoadAgents(ParamsReadContext ctx, int version)
	{
		if(!ctx.Read(m_AttachedAgents))
			return false;
		return true;
	}
	// -------------------------------------------------------------------------
	void SaveAgents(ParamsWriteContext ctx)
	{
		
		ctx.Write(m_AttachedAgents);
	}
	// -------------------------------------------------------------------------


	//! Called when entity is being created as new by CE/ Debug
	override void EEOnCECreate()
	{
		//Print("EEOnCECreate");
		if( !IsMagazine() && HasQuantity() ) SetCEBasedQuantity();
		//SetCEBasedQuantity();
	}
	
	
	//-------------------------
	// OPEN/CLOSE USER ACTIONS
	//-------------------------
	//! Implementations only
	void Open() {}
	void Close() {}
	bool IsOpen() {return false;}
	
	
	// ------------------------------------------------------------
	// CONDITIONS
	// ------------------------------------------------------------
	override bool CanPutInCargo( EntityAI parent )
	{
		if ( parent )
		{
			if ( parent.IsInherited(DayZInfected) )
				return true;

			if ( !parent.IsRuined() )
				return true;
		}
		
		return true;
	}	
	
	override bool CanPutAsAttachment( EntityAI parent )
	{
		if ( !IsRuined() )
			return true;
		return false;
	}
	
	// Plays muzzle flash particle effects
	static void PlayFireParticles(ItemBase weapon, int muzzle_index, string ammoType, ItemBase muzzle_owner, ItemBase suppressor, string config_to_search)
	{
		int id = muzzle_owner.GetMuzzleID();
		array<ref WeaponParticlesOnFire> WPOF_array = m_OnFireEffect.Get(id);
		
		if (WPOF_array)
		{
			for (int i = 0; i < WPOF_array.Count(); i++)
			{
				WeaponParticlesOnFire WPOF = WPOF_array.Get(i);
				
				if (WPOF)
				{
					WPOF.OnActivate(weapon, muzzle_index, ammoType, muzzle_owner, suppressor, config_to_search);
				}
			}
		}
	}
	
	// Plays bullet eject particle effects (usually just smoke, the bullet itself is a 3D model and is not part of this function)
	static void PlayBulletCasingEjectParticles(ItemBase weapon, string ammoType, ItemBase muzzle_owner, ItemBase suppressor, string config_to_search)
	{
		int id = muzzle_owner.GetMuzzleID();
		array<ref WeaponParticlesOnBulletCasingEject> WPOBE_array = m_OnBulletCasingEjectEffect.Get(id);
		
		if (WPOBE_array)
		{
			for (int i = 0; i < WPOBE_array.Count(); i++)
			{
				WeaponParticlesOnBulletCasingEject WPOBE = WPOBE_array.Get(i);
				
				if (WPOBE)
				{
					WPOBE.OnActivate(weapon, 0, ammoType, muzzle_owner, suppressor, config_to_search);
				}
			}
		}
	}
	
	// Plays all weapon overheating particles
	static void PlayOverheatingParticles(ItemBase weapon, string ammoType, ItemBase muzzle_owner, ItemBase suppressor, string config_to_search)
	{
		int id = muzzle_owner.GetMuzzleID();
		array<ref WeaponParticlesOnOverheating> WPOOH_array = weapon.m_OnOverheatingEffect.Get(id);
		
		if (WPOOH_array)
		{
			for (int i = 0; i < WPOOH_array.Count(); i++)
			{
				WeaponParticlesOnOverheating WPOOH = WPOOH_array.Get(i);
				
				if (WPOOH)
				{
					WPOOH.OnActivate(weapon, 0, ammoType, muzzle_owner, suppressor, config_to_search);
				}
			}
		}
	}
	
	// Updates all weapon overheating particles
	static void UpdateOverheatingParticles(ItemBase weapon, string ammoType, ItemBase muzzle_owner, ItemBase suppressor, string config_to_search)
	{
		int id = muzzle_owner.GetMuzzleID();
		array<ref WeaponParticlesOnOverheating> WPOOH_array = weapon.m_OnOverheatingEffect.Get(id);
		
		if (WPOOH_array)
		{
			for (int i = 0; i < WPOOH_array.Count(); i++)
			{
				WeaponParticlesOnOverheating WPOOH = WPOOH_array.Get(i);
				
				if (WPOOH)
				{
					WPOOH.OnUpdate(weapon, ammoType, muzzle_owner, suppressor, config_to_search);
				}
			}
		}
	}
	
	// Stops overheating particles
	static void StopOverheatingParticles(ItemBase weapon, string ammoType, ItemBase muzzle_owner, ItemBase suppressor, string config_to_search)
	{
		int id = muzzle_owner.GetMuzzleID();
		array<ref WeaponParticlesOnOverheating> WPOOH_array = weapon.m_OnOverheatingEffect.Get(id);
		
		if (WPOOH_array)
		{
			for (int i = 0; i < WPOOH_array.Count(); i++)
			{
				WeaponParticlesOnOverheating WPOOH = WPOOH_array.Get(i);
				
				if (WPOOH)
				{
					WPOOH.OnDeactivate(weapon, ammoType, muzzle_owner, suppressor, config_to_search);
				}
			}
		}
	}
	
	//----------------------------------------------------------------
	//Item Behaviour
	override bool IsHeavyBehaviour()
	{
		if (m_ItemBehaviour == 0)
		{	
			return true;
		}
		
		return false;
	}
	
	override bool IsOneHandedBehaviour()
	{
		if (m_ItemBehaviour == 1)
		{
			return true;	
		}
		
		return false;
	}
	
	override bool IsTwoHandedBehaviour()
	{
		if (m_ItemBehaviour == 2)
		{
			return true;
		}
			
		return false;
	}
	
	bool IsDeployable()
	{
		return false;
	}

	//----------------------------------------------------------------
	// Item Targeting (User Actions)
	void SetTakeable(bool pState)
	{
		m_IsTakeable = pState;
		SetSynchDirty();
	}

	bool IsTakeable()
	{
		return m_IsTakeable;
	}

	//! Attachment Sound Type getting from config file
	protected void PreLoadSoundAttachmentType()
	{
		string att_type = "None";

		if( ConfigIsExisting("soundAttType") )
		{
			att_type = ConfigGetString("soundAttType");
		}
		
		m_SoundAttType = att_type;
	}
	
	override string GetAttachmentSoundType()
	{	
		return m_SoundAttType;
	}
	
	//----------------------------------------------------------------
	//SOUNDS FOR ADVANCED PLACEMNT
	//----------------------------------------------------------------
	
	void SoundSynchRemoteReset()
	{	
		m_IsSoundSynchRemote = false;
		
		SetSynchDirty();
	}
	
	void SoundSynchRemote()
	{	
		m_IsSoundSynchRemote = true;
				
		SetSynchDirty();
	}
	
	bool IsSoundSynchRemote()
	{	
		return m_IsSoundSynchRemote;
	}
	
	string GetDeploySoundset()
	{
		
	}
	
	string GetPlaceSoundset()
	{
		
	}
	
	string GetLoopDeploySoundset()
	{
		
	}
	
	void SetIsPlaceSound( bool is_place_sound )
	{
		m_IsPlaceSound = is_place_sound;
	}
	
	bool IsPlaceSound()
	{
		return m_IsPlaceSound;
	}
	
	void SetIsDeploySound( bool is_deploy_sound )
	{
		m_IsDeploySound = is_deploy_sound;
	}
	
	bool IsDeploySound()
	{
		return m_IsDeploySound;
	}
	
	void PlayDeploySound()
	{		
		if ( GetGame().IsMultiplayer() && GetGame().IsClient() || !GetGame().IsMultiplayer() )
		{		
			EffectSound sound =	SEffectManager.PlaySound( GetDeploySoundset(), GetPosition() );
			sound.SetSoundAutodestroy( true );
		}
	}
	
	void PlayPlaceSound()
	{		
		if ( GetGame().IsMultiplayer() && GetGame().IsClient() || !GetGame().IsMultiplayer() )
		{		
			EffectSound sound =	SEffectManager.PlaySound( GetPlaceSoundset(), GetPosition() );
			sound.SetSoundAutodestroy( true );
		}
	}
	
	bool CanPlayDeployLoopSound()
	{		
		return IsBeingPlaced() && IsSoundSynchRemote();
	}
	
	void OnApply(PlayerBase player);
	
	float GetBandagingEffectivity()
	{
		return 1.0;
	};
	//returns applicable selection
	array<string> GetHeadHidingSelection()
	{
		return m_HeadHidingSelections;
	}

}

EntityAI SpawnItemOnLocation (string object_name, notnull InventoryLocation loc, bool full_quantity)
{
	EntityAI entity = SpawnEntity(object_name, loc, ECE_IN_INVENTORY, RF_DEFAULT);
	if (entity)
	{
		bool is_item = entity.IsInherited( ItemBase );
		if ( is_item && full_quantity )
		{
			ItemBase item = ItemBase.Cast( entity );
			item.SetQuantity(item.GetQuantityInit());
		}
	}
	else
	{
		Print("Cannot spawn entity: " + object_name);
		return NULL;
	}
	return entity;
}

void SetupSpawnedItem (ItemBase item, float health, float quantity)
{
	if (item)
	{
		if ( quantity == -1 )
		{
			if (item.HasQuantity())
				quantity = item.GetQuantityInit();
		}

		item.SetHealth("", "", health);

		if ( quantity > 0 )
		{
			item.SetQuantity(quantity);
		}
	}
}
