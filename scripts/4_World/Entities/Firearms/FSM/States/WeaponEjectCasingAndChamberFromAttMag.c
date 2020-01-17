
class WeaponEjectCasing extends WeaponStateBase
{
	void WeaponEjectCasing (Weapon_Base w = NULL, WeaponStateBase parent = NULL) { }

	override void OnEntry (WeaponEventBase e)
	{
		super.OnEntry(e);

		wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " ejected fired out casing");
		int mi = m_weapon.GetCurrentMuzzle();
		if(m_weapon.IsChamberFiredOut(mi))
		{
			m_weapon.EjectCasing(mi);
		}
		m_weapon.EffectBulletHide(mi);
		m_weapon.SelectionBulletHide();
	}
};

class WeaponEjectCasingMultiMuzzle extends WeaponStateBase
{
	void WeaponEjectCasingMultiMuzzle (Weapon_Base w = NULL, WeaponStateBase parent = NULL) { }

	override void OnEntry (WeaponEventBase e)
	{
		super.OnEntry(e);

		wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " ejected fired out casing multi muzzle");
		for( int i = 0; i < m_weapon.GetMuzzleCount(); i++ )
		{
			if(m_weapon.IsChamberFiredOut(i))
			{
				m_weapon.EjectCasing(i);
				m_weapon.EffectBulletHide(i);
				m_weapon.HideBullet(i);
			}
		}
	}
};

class WeaponEjectCasing_W4T extends WeaponEjectCasingMultiMuzzle
{
	override bool IsWaitingForActionFinish () { return true; }
};

class WeaponEjectAllMuzzles extends WeaponStateBase
{
	Magazine m_dstMagazine;
	void WeaponEjectAllMuzzles (Weapon_Base w = NULL, WeaponStateBase parent = NULL) { }

	override void OnEntry (WeaponEventBase e)
	{
		super.OnEntry(e);

		wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " ejected bullets from all muzzles");
		for( int i = 0; i < m_weapon.GetMuzzleCount(); i++ )
		{
			if(m_weapon.IsChamberFiredOut(i))
			{
				m_weapon.EjectCasing(i);
				m_weapon.EffectBulletHide(i);
				m_weapon.HideBullet(i);
				continue;
			}
			
			if(!m_weapon.IsChamberEmpty(i))
			{
				DayZPlayer p = e.m_player;
				if(m_dstMagazine && m_dstMagazine.GetAmmoCount() < m_dstMagazine.GetAmmoMax() )
				{
					m_weapon.CreateRound(i);
					ejectBulletAndStoreInMagazine(m_weapon,i,m_dstMagazine,p);
				}
				else
				{
					m_weapon.CreateRound(i);
					ejectBulletAndStoreInMagazine(m_weapon,i,null,p);
				}
				m_weapon.EffectBulletHide(i);
				m_weapon.HideBullet(i);
			}
		}
	
		//m_weapon.SelectionBulletHide();
	}
	
};

// eject casing and load bullet from att mag (no anim)
class WeaponEjectCasingAndChamberFromAttMag extends WeaponEjectCasing
{
	override void OnEntry (WeaponEventBase e)
	{
		super.OnEntry(e);

		int mi = m_weapon.GetCurrentMuzzle();
		pushToChamberFromAttachedMagazine(m_weapon, mi);
	}
	override void OnExit (WeaponEventBase e)
	{
		super.OnExit(e);
	}
};

class WeaponEjectCasingAndChamberFromAttMag_W4T extends WeaponEjectCasingAndChamberFromAttMag
{
	override bool IsWaitingForActionFinish () { return true; }
};

