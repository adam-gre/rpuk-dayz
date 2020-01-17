
//-----------------------------------------------------------------------------
class ItemSuppressor extends InventoryItemSuper
{
};

typedef ItemSuppressor SuppressorBase;

//-----------------------------------------------------------------------------
class ItemWatch extends InventoryItemSuper
{
};

//-----------------------------------------------------------------------------
class ItemTransmitter extends InventoryItemSuper
{
	proto native void SetNextChannel();
	proto native void SetPrevChannel();
	proto native float GetTunedFrequency();
	proto native void EnableBroadcast(bool state);
	proto native void EnableReceive(bool state);
	proto native bool IsBroadcasting();
	proto native bool IsReceiving();
	proto native int GetTunedFrequencyIndex();
	proto native void SetFrequencyByIndex(int index);
};

//-----------------------------------------------------------------------------
class ItemMegaphone extends InventoryItemSuper
{
	proto native bool CanSpeak();
	proto native void SetCanSpeak(bool onOff); 
}

//-----------------------------------------------------------------------------
class ItemRadio extends InventoryItemSuper
{
	proto native void TuneNext();
	proto native void TunePrev();
};

//-----------------------------------------------------------------------------
class ItemBarrel extends InventoryItemSuper
{
};

//-----------------------------------------------------------------------------
class UnderSlugLauncher extends Weapon
{
};

//-----------------------------------------------------------------------------
class ItemGPS extends InventoryItemSuper
{
};

//-----------------------------------------------------------------------------
class ItemCompass extends InventoryItemSuper
{
};

//-----------------------------------------------------------------------------
class CarWheel extends InventoryItemSuper
{
	//! Returns current wheel radius.
	proto native float GetRadius();

	//! Returns wheel width.
	proto native float GetWidth();

/*	Invoke car phys, when wheel was attached
	not working properly, postponed for now
	override void OnWasAttached( EntityAI parent, int slot_name )
	{
		IEntity iePrnt = IEntity.Cast( parent );
		dBodyActive( iePrnt, ActiveState.ACTIVE);
		dBodyApplyImpulse( iePrnt, "0 1 0");
	}
*/

/*
	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos)
	{
		Print("CarWheel>>> EEHitBy");
		Print( dmgZone );
		Print( damageResult );
		Print( source );
		Print( component );
		Print( damageResult.GetDamage(dmgZone, "Health") );
	}
*/
	
	override void EEKilled(Object killer)
	{
		string newWheel = "";
		switch( GetType() )
		{
			case "HatchbackWheel":
				newWheel = "HatchbackWheel_Ruined";
			break;

			case "CivSedanWheel":
				newWheel = "CivSedanWheel_Ruined";
			break;

			case "Hatchback_02_Wheel":
				newWheel = "Hatchback_02_Wheel_Ruined";
			break;
			
			case "Sedan_02_Wheel":
				newWheel = "Sedan_02_Wheel_Ruined";
			break;

			case "V3SWheel":
				newWheel = "V3SWheel_Ruined";
			break;

			case "V3SWheelDouble":
				newWheel = "V3SWheelDouble_Ruined";
			break;


		}

		if ( newWheel != "" )
		{
			ReplaceWheelLambda lambda = new ReplaceWheelLambda ( this, newWheel, NULL);
			lambda.SetTransferParams(true, true, true);
			GetInventory().ReplaceItemWithNew(InventoryMode.SERVER, lambda);
		}
	}

	override int GetMeleeTargetType()
	{
		return EMeleeTargetType.NONALIGNABLE;
	}
	
	override void SetActions()
	{
		super.SetActions();
		AddAction(ActionDetach);
		AddAction(ActionAttachOnSelection);
	}
};

class ReplaceWheelLambda : TurnItemIntoItemLambda
{
	vector m_oldOri;
	void ReplaceWheelLambda (EntityAI old_item, string new_item_type, PlayerBase player) {}

	override void CopyOldPropertiesToNew (notnull EntityAI old_item, EntityAI new_item)
	{
		super.CopyOldPropertiesToNew(old_item, new_item);
		m_oldOri = old_item.GetOrientation();
	}
	
	override protected void OnSuccess (EntityAI new_item)
	{
		super.OnSuccess( new_item );
		if (new_item)
			new_item.SetOrientation( m_oldOri );
	}
}

class Hatchback_02_Wheel extends CarWheel {};
class Hatchback_02_Wheel_Ruined extends CarWheel {};

class Sedan_02_Wheel extends CarWheel {};
class Sedan_02_Wheel_Ruined extends CarWheel {};

class HatchbackWheel extends CarWheel {};
class HatchbackWheel_Ruined extends CarWheel {};

class CivSedanWheel extends CarWheel {};
class CivSedanWheel_Ruined extends CarWheel {};

class CarDoor extends InventoryItemSuper
{

	override bool CanDetachAttachment(EntityAI parent)
	{
		//parent.FindAtt
		InventoryLocation loc = new InventoryLocation();
		
		bool isPresent = GetInventory().GetCurrentInventoryLocation( loc );
		
		if ( !isPresent )
		 return false;
		
		string slotName = InventorySlots.GetSlotName( loc.GetSlot() );

		if ( slotName && CarScript.Cast( parent ).GetCarDoorsState( slotName ) == CarDoorState.DOORS_OPEN )
			return true;
		
		return false;
	}

	override int GetMeleeTargetType()
	{
		return EMeleeTargetType.NONALIGNABLE;
	}
	
	override void EEHealthLevelChanged(int oldLevel, int newLevel, string zone)
	{
		if ( newLevel ==  GameConstants.STATE_RUINED )
		{
			EffectSound sound_plug;
			switch( zone )
			{
				case "Window":
					PlaySoundSet( sound_plug, "offroad_hit_window_small_SoundSet", 0, 0 );
				break;
			}
		}
	}
/*
	override void OnWasAttached( EntityAI parent, int slot_name )
	{
		
		SoundParams soundParams = new SoundParams("offroad_door_close_SoundSet");
		SoundObjectBuilder soundBuilder = new SoundObjectBuilder(soundParams);
		SoundObject soundObject = soundBuilder.BuildSoundObject();
		soundObject.SetPosition(GetPosition());
		GetGame().GetSoundScene().Play3D(soundObject, soundBuilder);
	}
*/
/*
	void CloseDoors()
	{
		SetAnimationPhase();
		
	}
*/
	override void SetActions()
	{
		super.SetActions();

		AddAction(ActionAttachOnSelection);
		AddAction(ActionDetach);
	}
	
};

//-------------------------------------
class Hatchback_02_Door_1_1 extends CarDoor {};
class Hatchback_02_Door_1_2 extends CarDoor {};
class Hatchback_02_Door_2_1 extends CarDoor {};
class Hatchback_02_Door_2_2 extends CarDoor {};
class Hatchback_02_Hood extends CarDoor {};
class Hatchback_02_Trunk extends CarDoor {};

class Hatchback_02_Door_1_1_Black extends Hatchback_02_Door_1_1 {};
class Hatchback_02_Door_1_2_Black extends Hatchback_02_Door_1_2 {};
class Hatchback_02_Door_2_1_Black extends Hatchback_02_Door_2_1 {};
class Hatchback_02_Door_2_2_Black extends Hatchback_02_Door_2_2 {};
class Hatchback_02_Hood_Black extends Hatchback_02_Hood {};
class Hatchback_02_Trunk_Black extends Hatchback_02_Trunk {};

class Hatchback_02_Door_1_1_Blue extends Hatchback_02_Door_1_1 {};
class Hatchback_02_Door_1_2_Blue extends Hatchback_02_Door_1_2 {};
class Hatchback_02_Door_2_1_Blue extends Hatchback_02_Door_2_1 {};
class Hatchback_02_Door_2_2_Blue extends Hatchback_02_Door_2_2 {};
class Hatchback_02_Hood_Blue extends Hatchback_02_Hood {};
class Hatchback_02_Trunk_Blue extends Hatchback_02_Trunk {};

//-------------------------------------
class Sedan_02_Door_1_1 extends CarDoor {};
class Sedan_02_Door_1_2 extends CarDoor {};
class Sedan_02_Door_2_1 extends CarDoor {};
class Sedan_02_Door_2_2 extends CarDoor {};
class Sedan_02_Hood extends CarDoor {};
class Sedan_02_Trunk extends CarDoor {};

class Sedan_02_Door_1_1_YellowRust extends CarDoor {};
class Sedan_02_Door_1_2_YellowRust extends CarDoor {};
class Sedan_02_Door_2_1_YellowRust extends CarDoor {};
class Sedan_02_Door_2_2_YellowRust extends CarDoor {};
class Sedan_02_Hood_YellowRust extends CarDoor {};
class Sedan_02_Trunk_YellowRust extends CarDoor {};

class Sedan_02_Door_1_1_Red extends Sedan_02_Door_1_1 {};
class Sedan_02_Door_1_2_Red extends Sedan_02_Door_1_2 {};
class Sedan_02_Door_2_1_Red extends Sedan_02_Door_2_1 {};
class Sedan_02_Door_2_2_Red extends Sedan_02_Door_2_2 {};
class Sedan_02_Hood_Red extends Sedan_02_Hood {};
class Sedan_02_Trunk_Red extends Sedan_02_Trunk {};

class Sedan_02_Door_1_1_RedRust extends Sedan_02_Door_1_1 {};
class Sedan_02_Door_1_2_RedRust extends Sedan_02_Door_1_2 {};
class Sedan_02_Door_2_1_RedRust extends Sedan_02_Door_2_1 {};
class Sedan_02_Door_2_2_RedRust extends Sedan_02_Door_2_2 {};
class Sedan_02_Hood_RedRust extends Sedan_02_Hood {};
class Sedan_02_Trunk_RedRust extends Sedan_02_Trunk {};

class Sedan_02_Door_1_1_Grey extends Sedan_02_Door_1_1 {};
class Sedan_02_Door_1_2_Grey extends Sedan_02_Door_1_2 {};
class Sedan_02_Door_2_1_Grey extends Sedan_02_Door_2_1 {};
class Sedan_02_Door_2_2_Grey extends Sedan_02_Door_2_2 {};
class Sedan_02_Hood_Grey extends Sedan_02_Hood {};
class Sedan_02_Trunk_Grey extends Sedan_02_Trunk {};

class Sedan_02_Door_1_1_GreyRust extends Sedan_02_Door_1_1 {};
class Sedan_02_Door_1_2_GreyRust extends Sedan_02_Door_1_2 {};
class Sedan_02_Door_2_1_GreyRust extends Sedan_02_Door_2_1 {};
class Sedan_02_Door_2_2_GreyRust extends Sedan_02_Door_2_2 {};
class Sedan_02_Hood_GreyRust extends Sedan_02_Hood {};
class Sedan_02_Trunk_GreyRust extends Sedan_02_Trunk {};

//-------------------------------------
class HatchbackDoors_Driver extends CarDoor {};
class HatchbackDoors_CoDriver extends CarDoor {};
class HatchbackHood extends CarDoor {};
class HatchbackTrunk extends CarDoor {};

class HatchbackDoors_Driver_White extends HatchbackDoors_Driver {};
class HatchbackDoors_CoDriver_White extends HatchbackDoors_CoDriver {};
class HatchbackHood_White extends HatchbackHood {};
class HatchbackTrunk_White extends HatchbackTrunk {};

class HatchbackDoors_Driver_Blue extends HatchbackDoors_Driver {};
class HatchbackDoors_CoDriver_Blue extends HatchbackDoors_CoDriver {};
class HatchbackHood_Blue extends HatchbackHood {};
class HatchbackTrunk_Blue extends HatchbackTrunk {};

class HatchbackDoors_Driver_GreenRust extends HatchbackDoors_Driver {};
class HatchbackDoors_CoDriver_GreenRust extends HatchbackDoors_CoDriver {};
class HatchbackHood_GreenRust extends HatchbackHood {};
class HatchbackTrunk_GreenRust extends HatchbackTrunk {};

class HatchbackDoors_Driver_WhiteRust extends HatchbackDoors_Driver {};
class HatchbackDoors_CoDriver_WhiteRust extends HatchbackDoors_CoDriver {};
class HatchbackHood_WhiteRust extends HatchbackHood {};
class HatchbackTrunk_WhiteRust extends HatchbackTrunk {};

class HatchbackDoors_Driver_BlueRust extends HatchbackDoors_Driver {};
class HatchbackDoors_CoDriver_BlueRust extends HatchbackDoors_CoDriver {};
class HatchbackHood_BlueRust extends HatchbackHood {};
class HatchbackTrunk_BlueRust extends HatchbackTrunk {};

//-------------------------------------
class CivSedanDoors_Driver extends CarDoor {};
class CivSedanDoors_CoDriver extends CarDoor {};
class CivSedanDoors_BackLeft extends CarDoor {};
class CivSedanDoors_BackRight extends CarDoor {};
class CivSedanHood extends CarDoor {};
class CivSedanTrunk extends CarDoor {};

class CivSedanDoors_Driver_Black extends CivSedanDoors_Driver {};
class CivSedanDoors_CoDriver_Black extends CivSedanDoors_CoDriver {};
class CivSedanDoors_BackLeft_Black extends CivSedanDoors_BackLeft {};
class CivSedanDoors_BackRight_Black extends CivSedanDoors_BackRight {};
class CivSedanHood_Black extends CivSedanHood {};
class CivSedanTrunk_Black extends CivSedanTrunk {};

class CivSedanDoors_Driver_Wine extends CivSedanDoors_Driver {};
class CivSedanDoors_CoDriver_Wine extends CivSedanDoors_CoDriver {};
class CivSedanDoors_BackLeft_Wine extends CivSedanDoors_BackLeft {};
class CivSedanDoors_BackRight_Wine extends CivSedanDoors_BackRight {};
class CivSedanHood_Wine extends CivSedanHood {};
class CivSedanTrunk_Wine extends CivSedanTrunk {};

class CarRadiator extends InventoryItemSuper
{

/* MOVED TO CarScript.C
	override void OnWasAttached ( EntityAI parent, int slot_id )
	{	
		if ( GetGame().IsServer() && parent )
		{
			Car car;
		 	Class.CastTo( car, parent );

			if ( car )
				car.SetHealth( "Radiator", "Health", GetHealth() );
		}
	}

	override void OnWasDetached ( EntityAI parent, int slot_id )
	{
		if ( GetGame().IsServer() && parent )
		{
			Car car;
		 	Class.CastTo( car, parent );

			if ( car )
			{
				car.Leak( CarFluid.COOLANT, car.GetFluidFraction(CarFluid.COOLANT)*car.GetFluidCapacity(CarFluid.COOLANT) );
				car.SetHealth( "Radiator", "Health", 0);
			}
		}
	}
*/
	override void EEKilled(Object killer)
	{
		if ( GetGame().IsServer() )
		{
			Car car;
			EntityAI parent = GetHierarchyParent();
			
			Class.CastTo( car, parent );

			if ( car )
				car.LeakAll( CarFluid.COOLANT );
		}
	}

	override void SetActions()
	{
		super.SetActions();
		
		AddAction(ActionAttachOnSelection);
		AddAction(ActionDetach);
	}
};

class TruckRadiator extends CarRadiator {};

class TruckExhaust extends ItemBase
{
	override void SetActions()
	{
		super.SetActions();
		
		AddAction(ActionAttach);
	}
};

class EngineBelt extends ItemBase
{
	override void SetActions()
	{
		super.SetActions();
		
		AddAction(ActionAttach);
	}
};

class BrakeFluid extends ItemBase {};

class EngineOil extends ItemBase
{
	override void SetActions()
	{
		super.SetActions();
		
		AddAction(ActionFillOil);
	}
};

class TireRepairKit extends ItemBase {};

class HeadlightH7 extends ItemBase
{
	override void SetActions()
	{
		super.SetActions();
		
		//AddAction(ActionAttach);
		AddAction(ActionAttachOnSelection);
		AddAction(ActionDetach);
	}
};

class HeadlightH7_Box extends Box_Base {};

class WindscreenBox extends ItemBase {};

class GlowPlug extends ItemBase
{
	override void SetActions()
	{
		super.SetActions();
		
		AddAction(ActionAttach);
	}
};

class SparkPlug extends ItemBase
{
	override void SetActions()
	{
		super.SetActions();
		
		AddAction(ActionAttach);
		AddAction(ActionAttachOnSelection);
		AddAction(ActionInsertSparkplug);
		AddAction(ActionDetach);
	}
};


//-----------------------------------------------------------------------------
class Clothing extends ItemBase
{
	override bool IsClothing()
	{
		return true;
	}

	// Conditions	
	override bool CanPutInCargo( EntityAI parent )
	{
		if( !super.CanPutInCargo( parent ) )
		{
			return false;
		}
		
		bool is_hidden_stash_exception = false;
		
		if ( parent.IsInherited( UndergroundStash ) )
			is_hidden_stash_exception = true;
		
		if ( ( GetNumberOfItems() == 0 || !parent || parent.IsMan() || is_hidden_stash_exception ) && super.CanPutInCargo( parent ) )
		{
			return true;
		}
		
		return false;
	}

	float GetItemVisibility()
	{
		return ConfigGetFloat("visibilityModifier");
	}
	
	void UpdateNVGStatus(PlayerBase player, bool attaching = false, bool force_disable = false)
	{
		NVGoggles NVGAttachment;
		NVGAttachment = NVGoggles.Cast(FindAttachmentBySlotName("NVG"));
		bool has_nvg_slot;
		for (int i = 0; i < GetInventory().GetAttachmentSlotsCount(); i++)
		{
			has_nvg_slot = GetInventory().GetAttachmentSlotId(i) == InventorySlots.GetSlotIdFromString("NVG");
			if (has_nvg_slot)
				break;
		}
		
		if ( player && has_nvg_slot )
		{
			if ( NVGAttachment )
			{
				NVGAttachment.LoweredCheck();
				
				if ( attaching && NVGAttachment.IsWorking() && NVGAttachment.m_Strap && NVGAttachment.m_IsLowered && !player.IsNVGWorking() )
				{
					NVGAttachment.SetPlayer(player);
					player.SetNVGWorking(true);
				}
				else if ( player.IsNVGWorking() )
				{
					NVGAttachment.SetPlayer(null);
					player.SetNVGWorking(false);
				}
			}
			else if ( player.IsNVGWorking() && force_disable )
			{
				player.SetNVGWorking(false);
			}
		}
	}
	
	//! Returns if this entity obsructs player's voice
	bool IsObstructingVoice()
	{
		return false;
	}
	
	//! This items has effect on player's voice
	void MutePlayer(PlayerBase player, bool state)
	{
	}
};

typedef Clothing ClothingBase;

//-----------------------------------------------------------------------------
class Box_Base extends InventoryItemSuper
{
	override void SetActions()
	{
		super.SetActions();
		AddAction(ActionUnpackBox);
	}
};

//-----------------------------------------------------------------------------
class ItemGrenade extends InventoryItemSuper
{
	/**@fn		SetPinned
	 * @brief	sets grenade to state GR_PINNED (if the state transition is possible)
	 * @return	true if state already set or if state transition performed, false otherwise
	 **/
	proto native bool SetPinned ();
	/**@fn		SetUnpinned
	 * @brief	sets grenade to state GR_UNPINNED (if the state transition is possible)
	 * @return	true if state already set or if state transition performed, false otherwise
	 **/
	proto native bool SetUnpinned ();
	/**@fn		SetIgnited
	 * @brief	sets grenade to state GR_TRIGGERED (if the state transition is possible)
	 * @return	true if state already set or if state transition performed, false otherwise
	 **/
	proto native bool SetIgnited ();
	/**@fn		IsPinned
	 * @return	true if grenade is pinned
	 **/
	proto native bool IsPinned ();
	/**@fn		IsActivated
	 * @return	true if grenade is GR_TRIGGERED or GR_EXPLODED
	 **/
	proto native bool IsActivated ();
};

typedef ItemGrenade GrenadeBase;

//-----------------------------------------------------------------------------
class ItemMap extends InventoryItemSuper
{
	string m_TextureClosed;
	string m_TextureOpened;
	string m_TextureLegend;
	string m_DisplayName;
	string m_Description;
	
	void ItemMap()
	{
		InitMapState();
	}
	
	void InitMapState()
	{
		string path = "CfgWorlds " + GetGame().GetWorldName();
		
		GetGame().ConfigGetText(path + " mapDisplayNameKey",m_DisplayName);
		GetGame().ConfigGetText(path + " mapDescriptionKey",m_Description);
		GetGame().ConfigGetText(path + " mapTextureClosed",m_TextureClosed);
		GetGame().ConfigGetText(path + " mapTextureOpened",m_TextureOpened);
		GetGame().ConfigGetText(path + " mapTextureLegend",m_TextureLegend);
		
		/*if (GetMapStateAnimation())
		{
			SetObjectTexture(0,m_TextureClosed);
		}
		else
		{
			SetObjectTexture(1,m_TextureOpened);
			SetObjectTexture(2,m_TextureLegend);
		}*/
		
		SetObjectTexture(0,m_TextureClosed);
		SetObjectTexture(1,m_TextureOpened);
		SetObjectTexture(2,m_TextureLegend);
	}
	
	override void OnItemLocationChanged(EntityAI old_owner, EntityAI new_owner)
	{
		super.OnItemLocationChanged(old_owner,new_owner);
		
		SetMapStateOpen(false, PlayerBase.Cast(old_owner));
	}
	
	//! displays open/closed selections; 1 == opened
	void SetMapStateOpen(bool state, PlayerBase player)
	{
		if (player)
		{
			player.SetMapOpen(state);
			if (state)
				player.OnItemInHandsChanged();
		}
		
		if (state)
		{
			ShowSelection("map_opened");
			HideSelection("map_closed");
		}
		else
		{
			ShowSelection("map_closed");
			HideSelection("map_opened");
		}
	}
	
	bool GetMapStateAnimation()
	{
		if (GetAnimationPhase("map_opened") == 0)
			return true;
		return false;
	}
	
	override bool NameOverride(out string output)
	{
		output = m_DisplayName;
		
		return true;
	}
	
	override bool DescriptionOverride(out string output)
	{
		output = m_Description;
		
		return true;
	}
};
