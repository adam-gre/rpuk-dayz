// Have a spooky Halloween everyone!

class PumpkinHelmet : Clothing
{
	void PumpkinHelmet()
	{
		SetEventMask(EntityEvent.INIT); // Enable EOnInit event
	}
	
	override void OnMovedInsideCargo(EntityAI container)
	{
		UpdateGlowState();
	}
	
	override void OnMovedWithinCargo(EntityAI container)
	{
		UpdateGlowState();
	}
	
	override void OnRemovedFromCargo(EntityAI container)
	{
		UpdateGlowState();
	}
	
	override void EOnInit( IEntity other, int extra)
	{
		UpdateGlowState();
	}
	
	override void OnItemLocationChanged( EntityAI old_owner, EntityAI new_owner)
	{
		super.OnItemLocationChanged( old_owner, new_owner);
		
		UpdateGlowState();
	}
	
	void UpdateGlowState()
	{
		// Makes sure PumpkinHelmet doesn't glow when it's attached on head, or it's inside cargo.
		
		bool do_glow = true;
		
		int id = GetInventory().GetSlotId(0);
		InventoryLocation IL = new InventoryLocation();
		GetInventory().GetCurrentInventoryLocation( IL );
		
		int id_2 = IL.GetSlot();
		int id_cargo = IL.GetIdx();
		
		if ( id == id_2) // Pumpkin is attached on head
			do_glow = false;
		
		if ( id_cargo != -1 ) // Pumpkin is in cargo
			do_glow = false;
		
		SetPilotLight(do_glow);
	}
};