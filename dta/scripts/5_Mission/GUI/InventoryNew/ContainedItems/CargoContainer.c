class CargoContainer extends Container
{
	protected const int ROWS_NUMBER_XBOX = 5;
	
	protected EntityAI												m_Entity;
	protected CargoBase												m_Cargo;
	protected int													m_CargoIndex = -1;
	
	protected int													m_FocusedItemPosition = -1;
	protected ref array<ref CargoContainerRow>						m_Rows;
	protected ref array<ref Icon>									m_Icons;
	protected ref map<EntityAI, ref Param3<ref Icon, int, int>>		m_ShowedItemPositions;
	protected ref map<EntityAI, ref Param3<ref Icon, int, int>>		m_ShowedLockPositions;
	
	protected float													m_IconSize;
	protected float													m_SpaceSize;
	
	protected bool													m_IsAttachment;
	protected Widget												m_CargoHeader;
	protected Widget												m_CargoContainer;
	protected Widget												m_ItemsContainer;
	protected ref SizeToChild										m_Resizer1;
	protected ref SizeToChild										m_Resizer2;
	protected ref Timer												m_ResizeTimer;
	
	void CargoContainer( LayoutHolder parent, bool is_attachment = false )
	{
		m_IsAttachment			= is_attachment;
		m_Rows					= new array<ref CargoContainerRow>;
		m_Icons					= new array<ref Icon>;
		m_ShowedItemPositions	= new map<EntityAI, ref Param3<ref Icon, int, int>>;
		m_ShowedLockPositions	= new map<EntityAI, ref Param3<ref Icon, int, int>>;
		
		m_CargoContainer		= m_RootWidget.FindAnyWidget( "grid_background" );
		m_ItemsContainer		= m_RootWidget.FindAnyWidget( "grid_overlay" );
		m_CargoHeader			= m_RootWidget.FindAnyWidget( "grid_container_header_spacer" );
		
		m_RootWidget.GetScript( m_Resizer1 );
		
		m_RootWidget.FindAnyWidget( "grid_container" ).GetScript( m_Resizer2 );
		m_CargoHeader.Show( is_attachment );
		
		m_MainWidget			= m_CargoContainer;
		m_FocusedItemPosition	= -1;
	}
	
	void ~CargoContainer()
	{
		if( m_Entity )
		{
			m_Entity.GetOnItemAddedIntoCargo().Remove( AddedToCargo );
			m_Entity.GetOnItemRemovedFromCargo().Remove( RemovedFromCargo );
			m_Entity.GetOnItemMovedInCargo().Remove( MovedInCargo );
			m_Entity.GetOnSetLock().Remove( SetLock );
			m_Entity.GetOnReleaseLock().Remove( ReleaseLock );
		}
	}
	
	int GetCargoIndex() { return m_CargoIndex; }
	
	void AddedToCargo( EntityAI item )
	{
		InventoryLocation il = new InventoryLocation;
		item.GetInventory().GetCurrentInventoryLocation( il );
		int x = il.GetCol();
		int y = il.GetRow();
		
		if( m_ShowedItemPositions.Contains( item ) )
		{
			Param3<ref Icon, int, int> item_pos = m_ShowedItemPositions.Get( item );
			InitIcon( item_pos.param1, item, x, y );
			item_pos.param2 = x;
			item_pos.param3 = y;
		}
		else
		{
			ref Icon icon = new Icon( this );
			m_Icons.Insert( icon );
			InitIcon( icon, item, x, y );
			m_ShowedItemPositions.Insert( item, new Param3<ref Icon, int, int>( icon, x, y ) );
		}
		UpdateHeaderText();
		
		#ifdef PLATFORM_CONSOLE
		for( int i = 0; i < m_Cargo.GetItemCount(); i++ )
		{
			EntityAI item2 = m_Cargo.GetItem( i );
			Param3<ref Icon, int, int> data = m_ShowedItemPositions.Get( item2 );
			if( data )
			{
				data.param1.SetCargoPos( i );
				data.param1.SetPos();
			}
		}
		
		m_FocusedItemPosition = Math.Min( m_ShowedItemPositions.Count() - 1, m_FocusedItemPosition );
		
		Refresh();
		#endif
	}
	
	void RemovedFromCargo( EntityAI item )
	{
		Param3<ref Icon, int, int> data = m_ShowedItemPositions.Get( item );
		if( data )
		{
			m_Icons.RemoveItem( data.param1 );
			m_ShowedItemPositions.Remove( item );
		}
		
		UpdateHeaderText();
		
		#ifdef PLATFORM_CONSOLE
		for( int i = 0; i < m_Cargo.GetItemCount(); i++ )
		{
			EntityAI item2 = m_Cargo.GetItem( i );
			data = m_ShowedItemPositions.Get( item2 );
			if( data && data.param1 )
			{
				data.param1.SetCargoPos( i );
				data.param1.SetPos();
			}
		}
		
		m_FocusedItemPosition = Math.Min( m_ShowedItemPositions.Count() - 1, m_FocusedItemPosition );
		
		Refresh();
		#endif
	}
	
	void MovedInCargo( EntityAI item )
	{
		InventoryLocation il = new InventoryLocation;
		item.GetInventory().GetCurrentInventoryLocation( il );
		
		int x = il.GetCol();
		#ifdef PLATFORM_CONSOLE
		int y = il.GetRow() - 1;
		#else
		int y = il.GetRow();
		#endif
		if( m_ShowedItemPositions.Contains( item ) )
		{
			ref Param3<ref Icon, int, int> data = m_ShowedItemPositions.Get( item );
			if( data.param1 )
			{
				if( data.param2 != x || data.param3 != y )
				{
					data.param2 = x;
					data.param3 = y;
					#ifdef PLATFORM_CONSOLE
						data.param1.SetCargoPos( data.param3 );
					#endif
					#ifdef PLATFORM_WINDOWS
					data.param1.SetPosX( data.param2 );
					data.param1.SetPosY( data.param3 );
					#endif
				}
				data.param1.UpdateInterval();
			}
		}
		UpdateSelection();
	}
	
	void SetLock( EntityAI item )
	{
		#ifndef PLATFORM_CONSOLE
		if( GetGame().GetPlayer() )
		{
			InventoryLocation il = new InventoryLocation;
			int index = GetGame().GetPlayer().GetHumanInventory().FindUserReservedLocationIndex( item );
			if( index >= 0 )
			{
				GetGame().GetPlayer().GetHumanInventory().GetUserReservedLocation( index, il );
				
				ref Icon icon = new Icon( this );
				m_Icons.Insert( icon );
				icon.InitLock( m_Entity, item, il.GetCol(), il.GetRow(), il.GetFlip() );
				m_ShowedLockPositions.Insert( item, new Param3<ref Icon, int, int>( icon, 1, 1 ) );
				item.GetOnReleaseLock().Insert( ReleaseLock );
			}
		}
		#endif
	}
	
	void ReleaseLock( EntityAI item )
	{
		#ifndef PLATFORM_CONSOLE
		if( m_ShowedLockPositions.Contains( item ) )
		{
			Icon ic	= m_ShowedLockPositions.Get( item ).param1;
			m_Icons.RemoveItem( ic );
			m_ShowedLockPositions.Remove( item );
			item.GetOnReleaseLock().Remove( ReleaseLock );
		}
		#endif
	}
	
	override void SetLayoutName()
	{
		#ifdef PLATFORM_CONSOLE
			m_LayoutName = WidgetLayoutName.CargoContainerXbox;
		#else
			switch( InventoryMenu.GetWidthType() )
			{
				case ScreenWidthType.NARROW:
				{
					m_LayoutName = WidgetLayoutName.CargoContainerNarrow;
					break;
				}
				case ScreenWidthType.MEDIUM:
				{
					m_LayoutName = WidgetLayoutName.CargoContainerMedium;
					break;
				}
				case ScreenWidthType.WIDE:
				{
					m_LayoutName = WidgetLayoutName.CargoContainerWide;
					break;
				}
			}
		#endif
			
	}
	
	void SetEntity( EntityAI item, int cargo_index = 0 )
	{
		if( item )
		{
			m_Entity		= item;
			m_Cargo			= item.GetInventory().GetCargoFromIndex(cargo_index);
			m_CargoIndex	= cargo_index;
			
			m_Entity.GetOnItemAddedIntoCargo().Insert( AddedToCargo );
			m_Entity.GetOnItemRemovedFromCargo().Insert( RemovedFromCargo );
			m_Entity.GetOnItemMovedInCargo().Insert( MovedInCargo );
			m_Entity.GetOnSetLock().Insert( SetLock );
			m_Entity.GetOnReleaseLock().Insert( ReleaseLock );
			UpdateHeaderText();

			InitGridHeight();
			m_MainWidget	= m_ItemsContainer;
			
			if( m_Cargo )
			{
				int i;
				int prev_count = m_ShowedItemPositions.Count();
				
				//START - Add new item Icons
				for( i = 0; i < m_Cargo.GetItemCount(); i++ )
				{
					EntityAI cargo_item = m_Cargo.GetItem( i );
					if( cargo_item )
					{
						AddedToCargo( cargo_item );
					}
				}
				
				#ifdef PLATFORM_CONSOLE
				Refresh();
				#endif
			}
		}
	}
	
	void UpdateHeaderText()
	{
		Widget header;
		string name = m_Entity.GetDisplayName();
		name.ToUpper();
		
		if( m_Entity.GetInventory().GetCargoFromIndex(m_CargoIndex) )
		{
			name = name + " (" + GetCargoCapacity().ToString() + "/" + GetMaxCargoCapacity() + ")";
			if( m_IsAttachment && m_CargoHeader )
			{
				TextWidget.Cast( m_CargoHeader.FindAnyWidget( "TextWidget0" ) ).SetText( name );
				float x, y;
				m_CargoHeader.FindAnyWidget( "TextWidget0" ).Update();
				m_CargoHeader.FindAnyWidget( "TextWidget0" ).GetScreenSize( x, y );
				m_CargoHeader.FindAnyWidget( "grid_container_header" ).SetSize( 1, y + InventoryMenu.GetHeightMultiplied( 10 ) );
				m_CargoHeader.Update();
				return;
			}
		}
		
		if( Container.Cast( GetParent() ) && Container.Cast( GetParent() ).GetHeader() )
			Container.Cast( GetParent() ).GetHeader().SetName( name );
	}
	
	void InitGridHeight()
	{
		m_Rows.Clear();
		m_ShowedItemPositions.Clear();
		
		ref CargoContainerRow row;
		
		#ifdef PLATFORM_CONSOLE
		int cargo_height = 	1;
		#else
		int cargo_height = 	m_Entity.GetInventory().GetCargoFromIndex(m_CargoIndex).GetHeight();
		#endif
		
		for ( int j = 0; j < cargo_height; j++ )
		{
			row = new CargoContainerRow( this );
			
			row.SetNumber( j );
			row.SetEntity( m_Entity );
			
			#ifdef PLATFORM_WINDOWS
			#ifndef PLATFORM_CONSOLE
			row.SetWidth( m_Entity.GetInventory().GetCargoFromIndex(m_CargoIndex).GetWidth() );
			#endif
			#endif
			
			row.GetRootWidget().SetSort( j );
			m_Rows.Insert( row );
		}
		
		float y;
		row.GetRootWidget().FindAnyWidget( "Icon0" ).GetScreenSize( y, m_IconSize );
		#ifdef PLATFORM_WINDOWS
		#ifndef PLATFORM_CONSOLE
			row.GetRootWidget().FindAnyWidget( "Spacer0" ).GetScreenSize( m_SpaceSize, y );
		#endif
		#endif
		
		m_Resizer2.ResizeParentToChild();
		m_Resizer1.ResizeParentToChild();
	}
	
	float GetIconSize()
	{
		return m_IconSize;
	}
	
	float GetSpaceSize()
	{
		return m_SpaceSize;
	}
	
	int GetCargoCapacity()
	{
		#ifdef PLATFORM_CONSOLE
		#ifndef PLATFORM_WINDOWS
			return CargoList.Cast( m_Cargo ).GetTotalWeight( null );
		#endif
		#endif
		int total_size = 0;
		for( int i = 0; i < m_Cargo.GetItemCount(); i++ )
		{
			int x, y;
			m_Cargo.GetItemSize( i, x, y );
			total_size += x * y;
		}
		return total_size;
	}
	
	int GetMaxCargoCapacity()
	{
		#ifdef PLATFORM_CONSOLE
		#ifndef PLATFORM_WINDOWS
			return CargoList.Cast( m_Cargo ).GetMaxWeight();
		#endif
		#endif
		return m_Cargo.GetWidth() * m_Cargo.GetHeight();
	}
	
	Icon GetIcon( EntityAI item )
	{
		if( item && m_ShowedItemPositions.Contains( item ) )
		{
			Param3<ref Icon, int, int> data = m_ShowedItemPositions.Get( item );
			return data.param1;
		}
		return null;
	}
	
	Icon GetIcon( int index )
	{
		if( index >= 0 && m_Cargo.GetItemCount() > index )
			return GetIcon( m_Cargo.GetItem( index ) );
		return null;
	}
	
	Icon GetFocusedItem()
	{
		return GetIcon( m_FocusedItemPosition );
	}
	
	override float GetFocusedContainerHeight( bool contents = false )
	{
		float x, y;
		if( GetFocusedItem() && contents )
			GetFocusedItem().GetRootWidget().GetScreenSize( x, y );
		else
			GetRootWidget().GetScreenSize( x, y );
		return y;
	}
	
	override float GetFocusedContainerYPos( bool contents = false )
	{
		float x, y;
		if( GetFocusedItem() && contents )
			GetFocusedItem().GetRootWidget().GetPos( x, y );
		else
			GetRootWidget().GetPos( x, y );
		return y;
	}
	
	override float GetFocusedContainerYScreenPos( bool contents = false )
	{
		float x, y;
		if( GetFocusedItem() && contents )
			GetFocusedItem().GetRootWidget().GetScreenPos( x, y );
		else
			GetRootWidget().GetScreenPos( x, y );
		return y;
	}
	
	void UpdateSelection()
	{
		if( m_IsActive )
		{
			if( m_FocusedItemPosition >= m_Icons.Count() )
				m_FocusedItemPosition = m_Icons.Count() - 1;
			Icon icon = GetIcon( m_FocusedItemPosition );
			if( icon && !icon.IsActive() )
			{
				icon.SetActive( true );
				Inventory.GetInstance().UpdateConsoleToolbar();
			}
		}
	}
	
	void UpdateRowVisibility( int count )
	{
		int i;
		int rows = Math.Max( 1, Math.Ceil( (count + 1) / ROWS_NUMBER_XBOX ) );
		int diff = rows - m_Rows.Count();
		
		if( diff < 0 )
		{
			for( i = m_Rows.Count() - 1; i >= rows; i-- )
			{
				m_Rows.Remove( i );
			}
		}
		else if( diff > 0 )
		{
			m_MainWidget	= m_CargoContainer;
			for( i = m_Rows.Count(); i < rows; i++ )
			{
				ref CargoContainerRow row = new CargoContainerRow( this );
				
				row.SetNumber( i );
				row.SetEntity( m_Entity );
				row.GetRootWidget().SetSort( i );
				m_Rows.Insert( row );
			}
			m_MainWidget	= m_ItemsContainer;
		}
		
		m_Resizer2.ResizeParentToChild();
		m_Resizer1.ResizeParentToChild();
	}
	
	override void Refresh()
	{
		#ifdef PLATFORM_CONSOLE
		if( !m_ResizeTimer )
			m_ResizeTimer = new Timer();
		if( m_ResizeTimer.IsRunning() )
			m_ResizeTimer.Stop();
		m_ResizeTimer.Run( 0.05, this, "RefreshImpl" );
		#endif
	}
	
	void RefreshImpl()
	{
		UpdateRowVisibility( m_ShowedItemPositions.Count() );
		UpdateSelection();
	}
	
	override void UpdateInterval()
	{
		foreach( Param3<ref Icon, int, int> data : m_ShowedItemPositions )
		{
			if( data.param1 )
			{
				data.param1.UpdateInterval();
			}
		}
	}

	Icon InitIcon( Icon icon, EntityAI item, int pos_x, int pos_y )
	{
		#ifdef PLATFORM_CONSOLE
			icon.SetSize( 1, 1 );
			#ifdef PLATFORM_WINDOWS
				pos_y = pos_y * 5 + pos_x;
			#endif
			icon.SetCargoPos( pos_y );
			icon.SetPosY( pos_y );
			icon.SetPos();
		#else
			int size_x, size_y;
			GetGame().GetInventoryItemSize( InventoryItem.Cast( item ), size_x, size_y );
			if( item.GetInventory().GetFlipCargo() )
			{
				icon.SetSize( size_y, size_x );
			}
			else
			{
				icon.SetSize( size_x, size_y );
			}
			icon.SetPosX( pos_x );
			icon.SetPosY( pos_y );
			icon.SetPos();
		#endif
		icon.Init( item );
		return icon;
	}
	
	bool HasItem( EntityAI item )
	{
		return m_ShowedItemPositions.Contains( item );
	}
	
	override bool TransferItemToVicinity()
	{
		Man player = GetGame().GetPlayer();
		if( GetFocusedItem() )
		{
			EntityAI entity = EntityAI.Cast( GetFocusedItem().GetObject() );
			if( entity && player.CanDropEntity( entity ) )
			{
				player.PredictiveDropEntity( entity );
				return true;
			}
		}
		return false;
	}
	
	void SetDefaultFocus( bool while_micromanagment_mode = false )
	{
		if( !while_micromanagment_mode && ItemManager.GetInstance().IsMicromanagmentMode() )
		{
			return;
		}
		
		Unfocus();
		UpdateSelection();
	}
	
	void Unfocus()
	{
		if( GetFocusedItem() )
		{
			GetFocusedItem().SetActive( false );
		}
		m_FocusedItemPosition = 0;
	}
	
	override void UnfocusAll()
	{
		if( m_Icons )
		{
			foreach( Icon icon : m_Icons )
			{
				icon.SetActive( false );
			}
		}
		m_FocusedItemPosition = 0;
	}
	
	override EntityAI GetFocusedEntity()
	{
		Icon icon = GetFocusedItem();
		if( icon )
		{
			return EntityAI.Cast( icon.GetObject() );
		}
			
		return null;
	}
	
	override void SetLastActive()
	{
		if( GetFocusedItem() )
		{
			GetFocusedItem().SetActive( false );
		}
		m_FocusedItemPosition = ( ( m_Rows.Count() - 1 ) * ROWS_NUMBER_XBOX );
		UpdateSelection();
	}
	
	override void SetActive( bool active )
	{
		super.SetActive( active );
		if( active )
		{
			SetDefaultFocus();
		}
		else
		{
			UnfocusAll();
		}
	}
	
	override bool IsItemActive()
	{
		if( GetFocusedItem() )
		{
			ItemBase item = ItemBase.Cast( GetFocusedItem().GetObject() );
			return ( item != null );
		}
		return false;
	}
	
	override bool IsItemWithQuantityActive()
	{
		if( GetFocusedItem()  )
		{
			ItemBase item = ItemBase.Cast( GetFocusedItem().GetObject() );
			return ( !IsEmpty() && QuantityConversions.HasItemQuantity( item ) && item.CanBeSplit() );
		}
		return false;
	}
	
	override bool IsEmpty()
	{
		return m_Icons.Count() == 0;
	}
	
	int GetRecipeCount( bool recipe_anywhere, ItemBase entity1, ItemBase entity2, PlayerBase player )
	{
		PluginRecipesManager recipes_manager = PluginRecipesManager.Cast( GetPlugin( PluginRecipesManager ) );
		return recipes_manager.GetValidRecipes( entity1, entity2, null, player ) );
	}
	
	override bool CanEquip()
	{
		if( GetFocusedItem() )
		{
			EntityAI entity = EntityAI.Cast( GetFocusedItem().GetObject() );
			InventoryLocation il = new InventoryLocation;
			if( !entity.IsInherited( Magazine ) )
			{
				return GetGame().GetPlayer().GetInventory().FindFreeLocationFor( entity, FindInventoryLocationType.ATTACHMENT, il );
			}
		}
		return false;
	}
	
	override bool CanCombine()
	{
		if( GetFocusedItem() )
		{
			ItemBase entity			= ItemBase.Cast( GetFocusedItem().GetObject() );
			ItemBase item_in_hands	= ItemBase.Cast( GetGame().GetPlayer().GetHumanInventory().GetEntityInHands() );
			
			return ( ItemManager.GetCombinationFlags( entity, item_in_hands ) != 0 );
		}
		return false;
	}
	
	override bool CanCombineAmmo()
	{
		if( GetFocusedItem() )
		{
			ActionManagerClient amc	= ActionManagerClient.Cast( PlayerBase.Cast( GetGame().GetPlayer() ).GetActionManager() );
			
			ItemBase entity			= ItemBase.Cast( GetFocusedItem().GetObject() );
			ItemBase item_in_hands	= ItemBase.Cast( GetGame().GetPlayer().GetHumanInventory().GetEntityInHands() );
			
			return ( amc.CanPerformActionFromInventory( item_in_hands, entity ) || amc.CanSetActionFromInventory( item_in_hands, entity ) );
		}
		return false;
	}
	
	override bool TransferItem()
	{
		if( GetFocusedItem() )
		{
			EntityAI entity = EntityAI.Cast( GetFocusedItem().GetObject() );
			if( entity )
			{
				GetGame().GetPlayer().PredictiveTakeEntityToInventory( FindInventoryLocationType.CARGO, entity );
				return true;
			}
		}
		return false;
	}
	
	override bool EquipItem()
	{
		if( GetFocusedItem() )
		{
			ItemBase entity = ItemBase.Cast( GetFocusedItem().GetObject() );
			if( entity )
			{
				if( entity.HasQuantity() && entity.CanBeSplit() )
				{
					entity.OnRightClick();
					Icon icon = m_ShowedItemPositions.Get( entity ).param1;
					
					if ( icon )
					{
						icon.SetQuantity();
					}
				}
				else
				{
					GetGame().GetPlayer().PredictiveTakeEntityToInventory( FindInventoryLocationType.ATTACHMENT, entity );
					return true;
				}
			}
		}
		return false;
	}
	
	override bool SelectItem()
	{
		Icon focused_item = GetFocusedItem();
		if( focused_item )
		{
			ItemManager.GetInstance().SetSelectedItem( ItemBase.Cast( focused_item.GetObject() ), this, null );
			return true;
		}
		return false;
	}

	override bool Select()
	{
		EntityAI prev_item;
		if( GetFocusedItem() )
		{
			prev_item = EntityAI.Cast( GetFocusedEntity() );
		}
		Man player = GetGame().GetPlayer();
		
		if( ItemManager.GetInstance().IsItemMoving() )
		{
			EntityAI selected_item = ItemManager.GetInstance().GetSelectedItem();
			if( selected_item && m_Entity )
			{
				bool can_add = m_Entity.GetInventory().CanAddEntityInCargo( selected_item, selected_item.GetInventory().GetFlipCargo());
				bool in_cargo = !player.GetInventory().HasEntityInInventory( selected_item ) || !m_Entity.GetInventory().HasEntityInCargo( selected_item );
				if( can_add && in_cargo )
				{
					player.PredictiveTakeEntityToTargetCargo( m_Entity, selected_item );
					Container selected_cont2 = ItemManager.GetInstance().GetSelectedContainer();
					if( selected_cont2 )
					{
						selected_cont2.SetActive( false );
					}
					
					SetActive( true );
					m_FocusedItemPosition = 0;
					return true;
				}
				else
				{
					Container selected_cont = ItemManager.GetInstance().GetSelectedContainer();
					if( selected_cont )
					{
						selected_cont.SetActive( false );
					}
					
					SetActive( true );
					SetDefaultFocus( true );
				}
			}
		}
		else if ( prev_item && prev_item.GetInventory().CanRemoveEntity() )
		{
			EntityAI item_in_hands = GetGame().GetPlayer().GetHumanInventory().GetEntityInHands();
			if( item_in_hands )
			{
				if( GameInventory.CanSwapEntities( item_in_hands, prev_item ) )
				{
					player.PredictiveSwapEntities( item_in_hands, prev_item );
					return true;
				}
			}
			else
			{
				if( player.GetHumanInventory().CanAddEntityInHands( prev_item ) )
				{
					player.PredictiveTakeEntityToHands( prev_item );
					return true;
				}
			}
		}
		return false;
	}
	
	override bool Combine()
	{
		if( GetFocusedItem() )
		{
			Icon icon = GetFocusedItem();
			if( icon )
			{
				EntityAI item_in_hands	= GetGame().GetPlayer().GetHumanInventory().GetEntityInHands();
				EntityAI prev_item		= EntityAI.Cast( icon.GetObject() );
				if( item_in_hands && prev_item )
				{
					return icon.CombineItems( item_in_hands, prev_item );
				}
			}
		}
		return true;
	}
	
	override void MoveGridCursor( int direction )
	{
		if( !ItemManager.GetInstance().IsMicromanagmentMode() )
		{
			Container cnt;
			Icon icon = GetIcon( m_FocusedItemPosition );
			if( icon )
			{
				icon.SetActive( false );
				ItemManager.GetInstance().HideTooltip( );
			}
			
			int focused_row = m_FocusedItemPosition / ROWS_NUMBER_XBOX;
			int row_min = focused_row * ROWS_NUMBER_XBOX;
			int row_max = row_min + ROWS_NUMBER_XBOX - 1;
			
			if(row_max >= m_Icons.Count() )
			{
				row_max = m_Icons.Count() - 1;
			}
			
			if( direction == Direction.UP )
			{
				m_FocusedItemPosition -= ROWS_NUMBER_XBOX;
				focused_row = m_FocusedItemPosition / ROWS_NUMBER_XBOX;
				if( m_FocusedItemPosition < 0 || m_Icons.Count() == 1 )
				{
					m_FocusedItemPosition = 0;
					cnt = Container.Cast( GetParent() );
					if( cnt )
					{
						cnt.SetPreviousActive();
					}
					return;
				}
			}
			else if( direction == Direction.DOWN )
			{
				m_FocusedItemPosition += ROWS_NUMBER_XBOX;
				focused_row = m_FocusedItemPosition / ROWS_NUMBER_XBOX;
				if( ( m_FocusedItemPosition > m_Icons.Count() - 1 && focused_row >= m_Icons.Count() / ROWS_NUMBER_XBOX ) || m_Icons.Count() == 0 )
				{
					cnt = Container.Cast( GetParent() );
					m_FocusedItemPosition = 0;
					if( cnt )
					{
						cnt.SetNextActive();
					}
					return;
				}
				else if( m_FocusedItemPosition > m_Icons.Count() - 1 )
				{
					m_FocusedItemPosition = m_Icons.Count() - 1;
				}
			}
			else if( direction == Direction.RIGHT && m_Icons.Count() > 1 )
			{
				m_FocusedItemPosition++;
				if( m_FocusedItemPosition > row_max  )
				{
					m_FocusedItemPosition = row_min;
				}
			}
			else if( direction == Direction.LEFT && m_Icons.Count() > 1 )
			{
				m_FocusedItemPosition--;
				if( m_FocusedItemPosition < row_min  )
				{
					m_FocusedItemPosition = row_max;
				}
			}
		}
		else
		{
			if( direction == Direction.UP )
			{
				cnt = Container.Cast( GetParent() );
				if( cnt )
				{
					cnt.SetPreviousActive();
				}
				m_FocusedItemPosition = 0;
				return;
			}
			else if( direction == Direction.DOWN )
			{
				cnt = Container.Cast( GetParent() );
				if( cnt )
				{
					cnt.SetNextActive();
				}
				m_FocusedItemPosition = 0;
				return;
			}
		}
		UpdateSelection();
	}
}