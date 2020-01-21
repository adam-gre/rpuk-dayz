/**@class	WeaponGuardBase
 * @brief	represents guard on a transition from state to state
 **/
class WeaponGuardBase
{
	/**@fn		GuardCondition
	 * @brief enable or disable transition based on condition
	 * the guard is a boolean operation executed first and which can prevent the transition from firing by returning false
	 * @return	true if transition is allowed
	 **/
	bool GuardCondition (WeaponEventBase e) { return true; }
};

class GuardAnd extends WeaponGuardBase
{
	ref WeaponGuardBase m_arg0;
	ref WeaponGuardBase m_arg1;

	void GuardAnd (WeaponGuardBase arg0 = NULL, WeaponGuardBase arg1 = NULL) { m_arg0 = arg0; m_arg1 = arg1; }

	override bool GuardCondition (WeaponEventBase e)
	{
		bool result = m_arg0.GuardCondition(e) && m_arg1.GuardCondition(e);
		wpnDebugPrint("[wpnfsm] guard - " + m_arg0.Type() + " && " + m_arg1.Type() + " = " + result);
		return result;
	}
};

class GuardNot extends WeaponGuardBase
{
	ref WeaponGuardBase m_arg0;

	void GuardNot (WeaponGuardBase arg0 = NULL) { m_arg0 = arg0; }

	override bool GuardCondition (WeaponEventBase e)
	{
		bool result = !m_arg0.GuardCondition(e);
		wpnDebugPrint("[wpnfsm] guard - ! " + m_arg0.Type() + " = " + result);
		return result;
	}
};

class GuardOr extends WeaponGuardBase
{
	ref WeaponGuardBase m_arg0;
	ref WeaponGuardBase m_arg1;

	void GuardOr (WeaponGuardBase arg0 = NULL, WeaponGuardBase arg1 = NULL) { m_arg0 = arg0; m_arg1 = arg1; }

	override bool GuardCondition (WeaponEventBase e)
	{
		bool result = m_arg0.GuardCondition(e) || m_arg1.GuardCondition(e);
		wpnDebugPrint("[wpnfsm] guard - " + m_arg0.Type() + " || " + m_arg1.Type() + " = " + result);
		return result;
	}
};

// guards /////////////////////////////////////////////////////////////////////////////////////////////////////////////

class WeaponGuardJammed extends WeaponGuardBase
{
	protected Weapon_Base m_weapon;
	void WeaponGuardJammed (Weapon_Base w = NULL) { m_weapon = w; }

	override bool GuardCondition (WeaponEventBase e)
	{
		/*int mi = m_weapon.GetCurrentMuzzle();
		if (m_weapon.IsChamberJammed(mi))*/
		if(m_weapon.IsJammed())
		{
			wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - jammed");
			return true;
		}
		wpnDebugPrint("[wpnfsm] guard - not jammed");
		return false;
	}
};

class WeaponGuardIsDestroyed extends WeaponGuardBase
{
	protected Weapon_Base m_weapon;
	void WeaponGuardIsDestroyed (Weapon_Base w = NULL) { m_weapon = w; }

	override bool GuardCondition (WeaponEventBase e)
	{
		if (m_weapon.IsDamageDestroyed())
		{
			wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - weapon destroyed");
			return true;
		}
		wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - weapon not destroyed");
		return false;
	}
}

class WeaponGuardHasAmmo extends WeaponGuardBase
{
	protected Weapon_Base m_weapon;
	void WeaponGuardHasAmmo (Weapon_Base w = NULL) { m_weapon = w; }

	override bool GuardCondition (WeaponEventBase e)
	{
		int mi = m_weapon.GetCurrentMuzzle();
		Magazine mag = m_weapon.GetMagazine(mi);
		if (mag != NULL && mag.GetAmmoCount() > 0)
		{
			wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - has ammo");
			return true;
		}
		wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - no ammo");
		return false;
	}
};

class WeaponGuardHasAmmoInnerMagazine extends WeaponGuardBase
{
	protected Weapon_Base m_weapon;
	void WeaponGuardHasAmmoInnerMagazine (Weapon_Base w = NULL) { m_weapon = w; }

	override bool GuardCondition (WeaponEventBase e)
	{
		int mi = m_weapon.GetCurrentMuzzle();
		if (m_weapon.GetInternalMagazineCartridgeCount(mi) >= 1)
		{
			wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - has ammo in inner magazine");
			return true;
		}
		wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - no ammo in inner magazine");
		return false;
	}
};

class WeaponGuardHasAmmoInEvent extends WeaponGuardBase
{
	protected Weapon_Base m_weapon;
	void WeaponGuardHasAmmoInEvent (Weapon_Base w = NULL) { m_weapon = w; }

	override bool GuardCondition (WeaponEventBase e)
	{
		Magazine mag = e.m_magazine;
		if (mag != NULL && mag.GetAmmoCount() > 0)
		{
			wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - has ammo in event");
			return true;
		}
		wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - no ammo in event");
		return false;
	}
};


class WeaponGuardHasMag extends WeaponGuardBase
{
	protected Weapon_Base m_weapon;
	void WeaponGuardHasMag (Weapon_Base w = NULL) { m_weapon = w; }

	override bool GuardCondition (WeaponEventBase e)
	{
		int mi = m_weapon.GetCurrentMuzzle();
		Magazine mag = m_weapon.GetMagazine(mi);
		if (mag != NULL)
		{
			wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - has magazine");
			return true;
		}
		wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - no magazine");
		return false;
	}
};

class WeaponGuardChamberEmpty extends WeaponGuardBase
{
	protected Weapon_Base m_weapon;
	protected int m_muzzle;
	void WeaponGuardChamberEmpty (Weapon_Base w = NULL, int muzzle_index = 0 ) { m_weapon = w; m_muzzle = muzzle_index; }

	override bool GuardCondition (WeaponEventBase e)
	{
		if (m_weapon.IsChamberEmpty(m_muzzle))
		{
			wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - chamber (" + m_muzzle + ") empty");
			return true;
		}
		wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - chamber (" + m_muzzle + ") not empty");
		return false;
	}
};

class WeaponGuardCurrentChamberEmpty extends WeaponGuardBase
{
	protected Weapon_Base m_weapon;
	void WeaponGuardCurrentChamberEmpty (Weapon_Base w = NULL) { m_weapon = w; }

	override bool GuardCondition (WeaponEventBase e)
	{
		int mi = m_weapon.GetCurrentMuzzle();
		if (m_weapon.IsChamberEmpty(mi))
		{
			wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - chamber empty");
			return true;
		}
		wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - chamber not empty");
		return false;
	}
};

class WeaponGuardChamberFull extends WeaponGuardBase
{
	protected Weapon_Base m_weapon;
	protected int m_muzzle;
	void WeaponGuardChamberFull (Weapon_Base w = NULL, int muzzle_index = 0 ) { m_weapon = w; m_muzzle = muzzle_index; }

	override bool GuardCondition (WeaponEventBase e)
	{
		if (m_weapon.IsChamberFull(m_muzzle))
		{
			wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - chamber (" + m_muzzle + ") full");
			return true;
		}
		wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - chamber (" + m_muzzle + ") not full");
		return false;
	}
};

class WeaponGuardCurrentChamberFull extends WeaponGuardBase
{
	protected Weapon_Base m_weapon;
	void WeaponGuardCurrentChamberFull (Weapon_Base w = NULL) { m_weapon = w; }

	override bool GuardCondition (WeaponEventBase e)
	{
		int mi = m_weapon.GetCurrentMuzzle();
		if (m_weapon.IsChamberFull(mi))
		{
			wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - chamber full");
			return true;
		}
		wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - chamber not full");
		return false;
	}
};


class WeaponGuardInnerMagazineFull extends WeaponGuardBase
{
	protected Weapon_Base m_weapon;
	void WeaponGuardInnerMagazineFull (Weapon_Base w = NULL) { m_weapon = w; }

	override bool GuardCondition (WeaponEventBase e)
	{
		int mi = m_weapon.GetCurrentMuzzle();
		if (m_weapon.IsInternalMagazineFull(mi))
		{
			wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - internal magazine full");
			return true;
		}
		wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - internal magazine not full");
		return false;
	}
};

class WeaponGuardInnerMagazineFullShareChamber extends WeaponGuardBase
{
	protected Weapon_Base m_weapon;
	void WeaponGuardInnerMagazineFullShareChamber (Weapon_Base w = NULL) { m_weapon = w; }

	override bool GuardCondition (WeaponEventBase e)
	{
		int mi = m_weapon.GetCurrentMuzzle();
		
		if ( m_weapon.IsChamberFull(mi) && m_weapon.IsInternalMagazineFull(mi))
		{
			wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - internal magazine with share chamber is full");
			return true;
		}
		wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - internal magazine with share chamber is not full");
		return false;
	}
}; 

class WeaponGuardChamberFiredOut extends WeaponGuardBase
{
	protected Weapon_Base m_weapon;
	protected int m_muzzle;
	void WeaponGuardChamberFiredOut (Weapon_Base w = NULL, int muzzle_index = 0 ) { m_weapon = w; m_muzzle = muzzle_index; }

	override bool GuardCondition (WeaponEventBase e)
	{
		if (m_weapon.IsChamberFiredOut(m_muzzle))
		{
			wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - chamber (" + m_muzzle + ") fireout");
			return true;
		}
		wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - chamber (" + m_muzzle + ") not fireout");
		return false;
	}
};

class WeaponGuardCurrentChamberFiredOut extends WeaponGuardBase
{
	protected Weapon_Base m_weapon;
	void WeaponGuardCurrentChamberFiredOut (Weapon_Base w = NULL) { m_weapon = w; }

	override bool GuardCondition (WeaponEventBase e)
	{
		int mi = m_weapon.GetCurrentMuzzle();
		if (m_weapon.IsChamberFiredOut(mi))
		{
			wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - chamber fired out");
			return true;
		}
		wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - chamber not fired out");
		return false;
	}
};

class WeaponGuardCanAttachMag extends WeaponGuardBase
{
	protected Weapon_Base m_weapon;
	void WeaponGuardCanAttachMag (Weapon_Base w = NULL) { m_weapon = w; }

	override bool GuardCondition (WeaponEventBase e)
	{
		int mi = m_weapon.GetCurrentMuzzle();
		if (m_weapon && e.m_magazine && m_weapon.CanAttachMagazine(mi, e.m_magazine))
		{
			wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - can attach magazine");
			return true;
		}
		wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - cannot attach magazine");
		return false;
	}
};

class WeaponGuardCanSwapMag extends WeaponGuardBase
{
	protected Weapon_Base m_weapon;
	void WeaponGuardCanSwapMag (Weapon_Base w = NULL) { m_weapon = w; }

	override bool GuardCondition (WeaponEventBase e)
	{
		int mi = m_weapon.GetCurrentMuzzle();
		Magazine attached_mag = m_weapon.GetMagazine(mi);
		if (m_weapon && e.m_magazine && e.m_magazine != attached_mag /*&& m_weapon.CanSwapMagazine(mi, e.m_magazine)*/)
		{
			wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - can swap magazine");
			return true;
		}
		wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - cannot swap magazine");
		return false;
	}
};

class WeaponGuardCanDetachMag extends WeaponGuardBase
{
	protected Weapon_Base m_weapon;
	void WeaponGuardCanDetachMag (Weapon_Base w = NULL) { m_weapon = w; }

	override bool GuardCondition (WeaponEventBase e)
	{
		int mi = m_weapon.GetCurrentMuzzle();
		if (m_weapon && e.m_magazine && m_weapon.GetMagazine(mi)/* && m_weapon.CanDetachMagazine(mi, e.m_magazine)*/)
		{
			wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - can detach magazine");
			return true;
		}
		wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - cannot detach magazine");
		return false;
	}
};

class WeaponGuardChamberHasRoomForMoreThanOne extends WeaponGuardBase
{
	protected Weapon_Base m_weapon;
	void WeaponGuardChamberHasRoomForMoreThanOne (Weapon_Base w = NULL) { m_weapon = w; }

	override bool GuardCondition (WeaponEventBase e)
	{
		int mi = m_weapon.GetCurrentMuzzle();
		if (m_weapon.GetInternalMagazineMaxCartridgeCount(mi) - m_weapon.GetInternalMagazineCartridgeCount(mi) > 1)
		{
			wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - chamber has room for more than 1b");
			return true;
		}
		wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - chamber has no room for more than 1b");
		return false;
	}
};

class WeaponGuardChamberHasRoomForOne extends WeaponGuardBase
{
	protected Weapon_Base m_weapon;
	void WeaponGuardChamberHasRoomForOne (Weapon_Base w = NULL) { m_weapon = w; }

	override bool GuardCondition (WeaponEventBase e)
	{
		int mi = m_weapon.GetCurrentMuzzle();
		if (m_weapon.GetTotalMaxCartridgeCount(mi) - m_weapon.GetTotalCartridgeCount(mi) == 1)
		{
			wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - chamber has room for 1b");
			return true;
		}
		wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - chamber has no room for 1b");
		return false;
	}
};

class WeaponGuardChamberMultiHasRoomBulltet extends WeaponGuardBase
{
	protected Weapon_Base m_weapon;
	void WeaponGuardChamberMultiHasRoomBulltet (Weapon_Base w = NULL) { m_weapon = w; }

	override bool GuardCondition (WeaponEventBase e)
	{
		int i = m_weapon.GetMuzzleCount() - 1;
		for ( ; i >= 0; i--)
		{
			if (m_weapon.GetTotalMaxCartridgeCount(i) - m_weapon.GetTotalCartridgeCount(i) >= 1)
			{
				wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - chamber has room for 1b");
				return true;
			}
			wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - chamber has no room for 1b");
		}
		return false;
	}
};

class WeaponGuardHasAmmoInLoopedState extends WeaponGuardBase
{
	WeaponChambering_Base m_state;
	void WeaponGuardHasAmmoInLoopedState (WeaponChambering_Base state) { m_state = state; }

	override bool GuardCondition (WeaponEventBase e)
	{
		Magazine mag = m_state.m_srcMagazine;
		if (mag != NULL && mag.GetAmmoCount() > 0)
		{
			wpnDebugPrint("[wpnfsm] guard - has ammo in looped state");
			return true;
		}
		wpnDebugPrint("[wpnfsm] guard - no ammo in looped state");
		return false;
	}
};

class WeaponGuardMagazinesHaveEqualSizes extends WeaponGuardBase
{
	protected Weapon_Base m_weapon;
	void WeaponGuardMagazinesHaveEqualSizes (Weapon_Base w = NULL) { m_weapon = w; }

	override bool GuardCondition (WeaponEventBase e)
	{
		int mi = m_weapon.GetCurrentMuzzle();
		Magazine mag = m_weapon.GetMagazine(mi);
		Magazine mag2 = e.m_magazine;
		if (mag != NULL && mag2 != NULL)
		{
			bool eq = magazinesHaveEqualSizes(mag, mag2);
			if (eq)
			{
				wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - same inventory sizes");
				return true;
			}

			wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - different inventory sizes");
			return false;
		}
		Error("[wpnfsm] " + Object.GetDebugName(m_weapon) + " guard - mag == NULL or mag2 == NULL, cannot perform comparison");
		return false;
	}
};

class WeaponGuardWeaponManagerWantContinue extends WeaponGuardBase
{
	override bool GuardCondition (WeaponEventBase e)
	{
		PlayerBase player = PlayerBase.Cast(e.m_player);
		return player.GetWeaponManager().WantContinue();
	}
};

