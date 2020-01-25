// fire
class WeaponDryFire extends WeaponStartAction
{
	float m_dtAccumulator;

	override void OnEntry (WeaponEventBase e)
	{
		super.OnEntry(e);
		m_dtAccumulator = 0;
	}

	override bool IsWaitingForActionFinish () { return true; }

	override void OnUpdate (float dt)
	{
		m_dtAccumulator += dt;
		DayZPlayer p;
		Class.CastTo(p, m_weapon.GetHierarchyParent());
		if (p)
		{
			HumanInputController hic = p.GetInputController();

			int muzzleIndex = m_weapon.GetCurrentMuzzle();
			float reloadTime = m_weapon.GetReloadTime(muzzleIndex);
			if ( hic.IsUseButton() && m_dtAccumulator >= reloadTime)
				if (m_weapon.CanProcessWeaponEvents())
					m_weapon.ProcessWeaponEvent(new WeaponEventDryFireTimeout(p));
		}
	}

	override void OnExit (WeaponEventBase e)
	{
		m_dtAccumulator = 0;
		super.OnExit(e);
	}
	
};

// fire
class WeaponFire extends WeaponStartAction
{
	float m_dtAccumulator;

	override bool IsWaitingForActionFinish () { return true; }

	override void OnEntry (WeaponEventBase e)
	{
		m_dtAccumulator = 0;

		wpnPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " WeaponFire bang!");
		//m_weapon.Fire();
		int mi = m_weapon.GetCurrentMuzzle();
		if (TryFireWeapon(m_weapon, mi))
		{
			DayZPlayerImplement p;
			if (Class.CastTo(p, e.m_player))
				p.GetAimingModel().SetRecoil(m_weapon);
		}
		super.OnEntry(e);
	}

	override void OnUpdate (float dt)
	{
		m_dtAccumulator += dt;

		DayZPlayer p;
		Class.CastTo(p, m_weapon.GetHierarchyParent());

		int muzzleIndex = m_weapon.GetCurrentMuzzle();
		float reloadTime = m_weapon.GetReloadTime(muzzleIndex);
		if (m_dtAccumulator >= reloadTime)
			if (m_weapon.CanProcessWeaponEvents())
				m_weapon.ProcessWeaponEvent(new WeaponEventReloadTimeout(p));
	}

	override void OnExit (WeaponEventBase e)
	{
		m_dtAccumulator = 0;
		super.OnExit(e);
	}
};

// fire
class WeaponFireMultiMuzzle extends WeaponStartAction
{
	float m_dtAccumulator;

	override bool IsWaitingForActionFinish () { return true; }

	override void OnEntry (WeaponEventBase e)
	{
		m_dtAccumulator = 0;

		wpnPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " WeaponFire bang bang!");
		//m_weapon.Fire();
		int mi = m_weapon.GetCurrentMuzzle();
		int b = m_weapon.GetCurrentModeBurstSize(mi);
		if(b > 1 )
		{
			
			for (int i = 0; i < b; i++)
			{
				if (TryFireWeapon(m_weapon, i))
				{
					DayZPlayerImplement p1;
					if (Class.CastTo(p1, e.m_player))
					p1.GetAimingModel().SetRecoil(m_weapon);
				}
			}
		}
		else
		{
			//int mi = m_weapon.GetCurrentMuzzle();
			if (TryFireWeapon(m_weapon, mi))
			{
				DayZPlayerImplement p;
				if (Class.CastTo(p, e.m_player))
					p.GetAimingModel().SetRecoil(m_weapon);
			}
		}
		super.OnEntry(e);
	}

	override void OnUpdate (float dt)
	{
		m_dtAccumulator += dt;

		DayZPlayer p;
		Class.CastTo(p, m_weapon.GetHierarchyParent());

		int muzzleIndex = m_weapon.GetCurrentMuzzle();
		float reloadTime = m_weapon.GetReloadTime(muzzleIndex);
		if (m_dtAccumulator >= reloadTime)
			if (m_weapon.CanProcessWeaponEvents())
				m_weapon.ProcessWeaponEvent(new WeaponEventReloadTimeout(p));
	}

	override void OnExit (WeaponEventBase e)
	{
		m_dtAccumulator = 0;
		super.OnExit(e);
	}
};

// fire to jam
class WeaponFireToJam extends WeaponStartAction
{
	float m_dtAccumulator;

	override bool IsWaitingForActionFinish () { return true; }

	override void OnEntry (WeaponEventBase e)
	{
		m_dtAccumulator = 0;

		wpnPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " WeaponFire bang! and jam?");
		//m_weapon.Fire();
		int mi = m_weapon.GetCurrentMuzzle();
		if (TryFireWeapon(m_weapon, mi))
		{
			m_weapon.SetJammed(true);
			DayZPlayerImplement p;
			if (Class.CastTo(p, e.m_player))
				p.GetAimingModel().SetRecoil(m_weapon);
		}
		super.OnEntry(e);
	}

	override void OnUpdate (float dt)
	{
		m_dtAccumulator += dt;

		DayZPlayer p;
		Class.CastTo(p, m_weapon.GetHierarchyParent());

		int muzzleIndex = m_weapon.GetCurrentMuzzle();
		float reloadTime = m_weapon.GetReloadTime(muzzleIndex);
		if (m_dtAccumulator >= reloadTime)
			if (m_weapon.CanProcessWeaponEvents())
				m_weapon.ProcessWeaponEvent(new WeaponEventReloadTimeout(p));
	}

	override void OnExit (WeaponEventBase e)
	{
		m_dtAccumulator = 0;
		super.OnExit(e);
	}
};


class WeaponFireAndChamber extends WeaponFire
{
	override void OnEntry (WeaponEventBase e)
	{
		super.OnEntry(e);
		
		if (!m_weapon.IsJammed())
		{
			wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " ejected fired out casing");
			int mi = m_weapon.GetCurrentMuzzle();
			m_weapon.EjectCasing(mi);
			m_weapon.EffectBulletHide(mi);
			m_weapon.SelectionBulletHide();

			pushToChamberFromAttachedMagazine(m_weapon, mi);
		}
	}
};

class WeaponFireAndChamberFromInnerMagazine extends WeaponFire
{
	override void OnEntry (WeaponEventBase e)
	{
		super.OnEntry(e);
		
		if (!m_weapon.IsJammed())
		{
			wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " ejected fired out casing");
			int mi = m_weapon.GetCurrentMuzzle();
			m_weapon.EjectCasing(mi);
			m_weapon.EffectBulletHide(mi);
			m_weapon.SelectionBulletHide();
			
			pushToChamberFromInnerMagazine(m_weapon, mi);
		}
	}
};

