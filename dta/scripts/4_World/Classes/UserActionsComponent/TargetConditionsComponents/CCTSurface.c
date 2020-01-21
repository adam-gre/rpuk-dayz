class CCTSurface : CCTBase
{
	protected float m_MaximalActionDistance;
	
	void CCTSurface ( float maximal_target_distance ) //distance measured from the center of the object!
	{
		m_MaximalActionDistance = maximal_target_distance;
	}
	
	override bool Can( PlayerBase player, ActionTarget target )
	{	
		if( !target || (target && target.GetObject()) )
		{
			return false;
		}
		
		if( !GetGame().IsMultiplayer() || GetGame().IsClient() ) 
		{
			float distance;
			vector hit_pos = target.GetCursorHitPos();
			if (hit_pos == vector.Zero)
				return false;
			
			//string surface_type;
			//GetGame().SurfaceGetType( hit_pos[0], hit_pos[2], surface_type );
			
			distance = Math.AbsInt(vector.Distance(hit_pos,player.GetPosition()));
			if ( distance <= m_MaximalActionDistance ) 
			{	
				return true;
			}
		}
		else
		{
			return true;
		}
		return false;
	}
};
