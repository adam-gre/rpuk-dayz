class UndergroundStash extends ItemBase
{
	void UndergroundStash()
	{
		SetEventMask(EntityEvent.INIT);
	}
	
	override void EOnInit(IEntity other, int extra) //!EntityEvent.INIT
	{
		vector pos = GetPosition();
		this.SetOrientation( GetGame().GetSurfaceOrientation(pos[0], pos[2]) );
	}
	
	override bool CanDisplayCargo()
	{
		super.CanDisplayCargo();
		
		return false;
	}
	
	void PlaceOnGround()
	{
		vector pos = GetPosition();
		pos[1] = GetGame().SurfaceY(pos[0], pos[2]);
		pos[1] = pos[1] + 0.22;
		SetPosition(pos);
	}
	
	ItemBase GetStashedItem()
	{
		ItemBase item;		
		
		if ( GetInventory().GetCargo().GetItemCount() > 0 )
		{
			item = ItemBase.Cast( GetInventory().GetCargo().GetItem(0) );
		}

		Print("GetStashedItem()");
		Print(item);
		return item;
	}
	
	override bool CanPutInCargo( EntityAI parent )
	{
		if( !super.CanPutInCargo(parent) ) {return false;}
		return false;
	}
	
	override bool CanReleaseCargo(EntityAI cargo)
	{
		return false;
	}
	
	override bool CanReceiveItemIntoHands (EntityAI item_to_hands)
	{
		return false;
	}
	
	override bool CanSaveItemInHands (EntityAI item_in_hands)
	{
		return false;
	}
	
	override bool CanPutIntoHands (EntityAI parent)
	{
		super.CanPutIntoHands( parent );
		
		return false;
	}
	
	override bool IsInventoryVisible()
	{
		return false;
	}
}