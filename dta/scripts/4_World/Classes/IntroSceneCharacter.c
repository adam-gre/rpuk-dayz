class IntroSceneCharacter extends Managed
{
	protected int			m_CharacterId;
	protected string		m_CharacterNam;
	protected MenuData		m_CharacterDta;
	protected PlayerBase	m_CharacterObj;
	protected vector		m_CharacterPos;
	protected vector		m_CharacterRot;
		
	protected ref TStringArray 	m_CharGenderList				= new TStringArray;
	protected ref TStringArray	m_CharShirtList					= new TStringArray;
	protected ref TStringArray 	m_CharPantsList					= new TStringArray;
	protected ref TStringArray	m_CharShoesList					= new TStringArray;
	
	protected ref map<ECharGender, ref array<string>> m_Characters = new map<ECharGender, ref array<string>>;
	
	protected ECharGender		m_CharGender;
	protected int 				m_CharShirtIndex;
	protected int 				m_CharPantsIndex;
	protected int 				m_CharShoesIndex;
	
	void IntroSceneCharacter()
	{
		m_CharacterId = GameConstants.DEFAULT_CHARACTER_MENU_ID;
	}
	
	void ~IntroSceneCharacter()
	{
		CharacterUnload();
	}
	
	bool IsDefaultCharacter()
	{
		return m_CharacterId == GameConstants.DEFAULT_CHARACTER_MENU_ID;
	}
	
	void SetToDefaultCharacter()
	{
		m_CharacterId = GameConstants.DEFAULT_CHARACTER_MENU_ID;
	}
	//==============================================
	// SetcharacterID
	//==============================================
	void SetCharacterID(int char_id)
	{
		m_CharacterId = char_id;
	}
	
	//==============================================
	// GetCharacterID
	//==============================================
	int GetCharacterID()
	{
		return m_CharacterId;
	}
	
	//==============================================
	// GetCharacterObj
	//==============================================
	PlayerBase GetCharacterObj()
	{
		return m_CharacterObj;
	}
	
	//==============================================
	// GetCharGenderList	
	//==============================================
	TStringArray GetCharGenderList()
	{
		return m_CharGenderList;
	}
	
	//==============================================
	// GetCharGenderList	
	//==============================================
	TStringArray GetCharList(ECharGender gender)
	{
		return m_Characters[gender];
	}
	
	//==============================================
	// GetCharShirtsList
	//==============================================
	TStringArray GetCharShirtsList()
	{
		return m_CharShirtList;
	}
	
	//==============================================
	// GetCharPantsList
	//==============================================
	TStringArray GetCharPantsList()
	{
		return m_CharPantsList;
	}
	
	//==============================================
	// GetCharShoesList
	//==============================================
	TStringArray GetCharShoesList()
	{
		return m_CharShoesList;
	}
	
	//==============================================
	// SetCharacterGender
	//==============================================
	void SetCharacterGender(ECharGender gender)
	{
		m_CharGender = gender;
	}
	
	//==============================================
	// IsCharacterFemale
	//==============================================
	bool IsCharacterFemale()
	{
		return ( m_CharGender == ECharGender.Female );
	}
	
	//==============================================
	// SetCharacterGender
	//==============================================
	ECharGender GetCharacterGender()
	{
		return m_CharGender;
	}
	
	//==============================================
	// GetPosition
	//==============================================
	vector GetPosition()
	{
		return m_CharacterObj.GetPosition();
	}
	
	//==============================================
	// GetNextCharacterID
	//==============================================
	int GetNextCharacterID()
	{
		int count = m_CharacterDta.GetCharactersCount();
		
		if ( count == 0 )
		{	
			return -1;
		}
		
		if ( m_CharacterId + 1 < count )
		{
			return m_CharacterId + 1;
		}
		else
		{
			return -1;
		}
	}
	
	//==============================================
	// GetPrevCharacterID
	//==============================================
	int GetPrevCharacterID()
	{
		int count = m_CharacterDta.GetCharactersCount();
		
		if ( count == 0 )
		{
			return -1;
		}
		
		if ( m_CharacterId > -1 )
		{
			return m_CharacterId - 1;
		}
		
		return count - 1;
	}
	
	//==============================================
	// CreateNewCharacterRandom
	//==============================================
	void CreateNewCharacterRandom()
	{
		// Select random gender
		SetCharacterGender( Math.RandomInt(0, 2) );
		
		// Select random chloths ( attachments )
		m_CharShirtIndex = m_CharShirtList.GetRandomIndex();
		m_CharPantsIndex = m_CharPantsList.GetRandomIndex();
		m_CharShoesIndex = m_CharShoesList.GetRandomIndex();
		
		// Select random character skin (class name)
		string char_name_random = m_Characters[GetCharacterGender()].GetRandomElement();
		
		// Create new character
		CreateNewCharacterByName( char_name_random );
	}
	
	//==============================================
	// CreateNewCharacterByName
	//==============================================
	void CreateNewCharacterById( int character_id )
	{
		if ( character_id == GameConstants.DEFAULT_CHARACTER_MENU_ID )
		{
			CreateDefaultCharacter();
		}
		else
		{
			CharacterLoad( character_id, m_CharacterPos, m_CharacterRot );
		}
	}
	
	//==============================================
	// CreateNewCharacterByName
	//==============================================
	void CreateNewCharacterByName( string character_name )
	{
		m_CharacterNam = character_name;
		
		CreateNewCharacter();
	}
	
	void CreateDefaultCharacter()
	{
		CharacterUnload();
		Man man = m_CharacterDta.CreateCharacterPerson( -1 );
		m_CharacterObj = PlayerBase.Cast( man );
		if(m_CharacterObj)
		{
			m_CharacterObj.PlaceOnSurface();
			m_CharacterObj.SetPosition(m_CharacterPos);
			m_CharacterObj.SetOrientation(m_CharacterRot);
		}
		else
		{
			string default_name = Widget.TranslateString( GameConstants.DEFAULT_CHARACTER_NAME );
			CreateNewCharacterRandom();
			m_CharacterDta.SaveDefaultCharacter( m_CharacterObj );
			m_CharacterDta.SetCharacterName(GameConstants.DEFAULT_CHARACTER_MENU_ID, default_name);
		}
	}
	
	void GetLastPlayedServer(int characterID, out string address, out int port)
	{
		m_CharacterDta.GetLastServerAddress(characterID,address);
		port = m_CharacterDta.GetLastServerPort(characterID);
	}
	//==============================================
	// CreateNewCharacter
	//==============================================
	void CreateNewCharacter()
	{
		// Unload old character (if exist)
		CharacterUnload();
				
		// Create Character
		m_CharacterObj = PlayerBase.Cast(g_Game.CreateObject(m_CharacterNam, m_CharacterPos, true));
		
		if (m_CharacterObj)
		{
			m_CharacterObj.PlaceOnSurface();
			m_CharacterObj.SetOrientation(m_CharacterRot);
			
			// Select Random Cloths
			SetAttachment(m_CharShirtList[m_CharShirtIndex], InventorySlots.BODY);
			SetAttachment(m_CharPantsList[m_CharPantsIndex], InventorySlots.LEGS);
			SetAttachment(m_CharShoesList[m_CharShoesIndex], InventorySlots.FEET);
		}
		
		//Create New Random Character
		SetupPlayerName( true );
	}
	
	//==============================================
	// LoadCharacterData
	//==============================================
	void LoadCharacterData(vector char_pos, vector char_rot, bool default_char = false)
	{
		m_CharacterDta = g_Game.GetMenuData();
		m_CharacterPos = char_pos;
		m_CharacterRot = char_rot;
		
		if (!default_char && m_CharacterDta.GetLastPlayedCharacter() > -1 )
		{
			m_CharacterId = m_CharacterDta.GetLastPlayedCharacter();
			m_CharacterDta.GetCharacterName(m_CharacterId, g_Game.GetPlayerGameName());
		}
		
		// Load all avalible options for character creation
		g_Game.ConfigGetTextArray("cfgCharacterCreation" + " gender",	m_CharGenderList);
		g_Game.ConfigGetTextArray("cfgCharacterCreation" + " top",		m_CharShirtList);
		g_Game.ConfigGetTextArray("cfgCharacterCreation" + " bottom",	m_CharPantsList);
		g_Game.ConfigGetTextArray("cfgCharacterCreation" + " shoe",		m_CharShoesList);

		// Init character table
		m_Characters.Clear();
		m_Characters.Insert( ECharGender.Male,		new array<string> );
		m_Characters.Insert( ECharGender.Female,	new array<string> );
		
		// Sort character by Gender
		TStringArray characters = GetGame().ListAvailableCharacters();
		for (int i = 0; i < characters.Count(); i++)
		{
			string char_cfg_name = characters.Get(i);
			if ( GetGame().IsKindOf(char_cfg_name, "SurvivorMale_Base") )
			{
				m_Characters[ECharGender.Male].Insert( char_cfg_name );
			}
			else
			{
				m_Characters[ECharGender.Female].Insert( char_cfg_name );
			}
		}
		
		CreateNewCharacterById(m_CharacterId/*, m_CharacterPos, m_CharacterRot*/);
		
		if (GetCharacterObj() )
		{
			if ( GetCharacterObj().IsMale() )
				SetCharacterGender(ECharGender.Male);
			else
				SetCharacterGender(ECharGender.Female);
		}
	}
	
	//-------------------------
	// CharacterUnload
	//-------------------------
	protected void CharacterUnload()
	{
		if ( m_CharacterObj )
		{
			g_Game.ObjectDelete(m_CharacterObj);
			m_CharacterObj = NULL;
		}
	}

	//-------------------------
	// CharacterLoad
	//-------------------------
	protected void CharacterLoad( int character_id, vector char_pos, vector char_rot )
	{
		if ( character_id == -1 )
		{
			Error("IntroSceneCharacter->CharacterLoad: character_id = "+ character_id +" Cant Load Character!!!");
			return;
		}
		
		CharacterUnload();
		
		SetCharacterID( character_id );
				
		m_CharacterObj = PlayerBase.Cast( m_CharacterDta.CreateCharacterPerson( character_id ) );
		
		if ( m_CharacterObj )
		{
			
			m_CharacterObj.PlaceOnSurface();
			m_CharacterObj.SetPosition(char_pos);
			m_CharacterObj.SetOrientation(char_rot);	
		}	
	}
	
	//-------------------------
	// SetupPlayerName
	//-------------------------
	protected void SetupPlayerName( bool new_name )
	{
		string name = GameConstants.DEFAULT_CHARACTER_NAME;		
		
#ifdef PLATFORM_CONSOLE
		BiosUserManager user_manager = GetGame().GetUserManager();
		if( user_manager )
		{
			BiosUser user = user_manager.GetSelectedUser();
			if( user )
			{
				name = user.GetName();
			}
		}
#else
		if ( !new_name )
		{
			m_CharacterDta.GetCharacterName(m_CharacterId, name);
		}
#endif
		
		g_Game.SetPlayerGameName(name);
	}
	
	//==============================================
	// SetAttachment
	//==============================================
	void SetAttachment(string type, int slot)
	{
		if ( !m_CharacterObj )
		{
			return;
		}
		
		g_Game.ObjectDelete(m_CharacterObj.GetInventory().FindAttachment(slot));
		
		EntityAI entity = EntityAI.Cast( g_Game.CreateObject(type, m_CharacterObj.GetPosition(), true) );
		
		if (entity)
		{
			m_CharacterObj.LocalTakeEntityAsAttachmentEx(entity, slot);
		}
	}
	
	string GetCharacterNameById(int char_id )
	{
		string character_name;
		 m_CharacterDta.GetCharacterName(char_id, character_name);
		return character_name:
	}
	
	string GetCharacterName()
	{
		string character_name;
		 m_CharacterDta.GetCharacterName(m_CharacterId, character_name);
		return character_name:
	}
	
	//==============================================
	// SaveCharName
	//==============================================
	void SaveCharName( string name )
	{
		m_CharacterDta.SetCharacterName(m_CharacterId, name);
		m_CharacterDta.SaveCharactersLocal();
	}
	
	//==============================================
	// SaveCharacterSetup
	//==============================================
	void SaveCharacterSetup()
	{
		int index_top; 
		int index_bottom;
		int index_shoes;
		int index_character;
		
		if( m_CharacterObj )
		{
			Object obj = m_CharacterObj.GetInventory().FindAttachment(InventorySlots.BODY);
			if( obj )
			{
				index_top = m_CharShirtList.Find( obj.GetType() );
			}
			
			obj = m_CharacterObj.GetInventory().FindAttachment(InventorySlots.LEGS);
			if( obj )
			{
				index_bottom = m_CharPantsList.Find( obj.GetType() );
			}
			
			obj = m_CharacterObj.GetInventory().FindAttachment(InventorySlots.FEET);
			if( obj )
			{
				index_shoes = m_CharShoesList.Find( obj.GetType() );
			}
			
			index_character = GetGame().ListAvailableCharacters().Find( m_CharacterObj.GetType() );
		}		
		
		//saves player' type and clothes to g_Game to sync with server
		GetGame().SetCharacterInfo(index_top, index_bottom, index_shoes, index_character);
	}
	
	//==============================================
	// SaveDefaultCharacter
	//==============================================
	void SaveDefaultCharacter()
	{
		if (m_CharacterObj)
		{
			m_CharacterDta.SaveDefaultCharacter(m_CharacterObj);
			
			/*g_Game.SetProfileString("defaultCharacter", m_CharacterObj.GetType());
			
			InventoryItem item = NULL;
			TStringArray inventory = new TStringArray;
			
			for (int i = 0; i < InventorySlots.COUNT; i++)
			{
				Class.CastTo(item, m_CharacterObj.GetInventory().FindAttachment(i));
				if (item)
				{
					inventory.Insert(item.GetType());
				}
			}
			
			g_Game.SetProfileStringList("defaultInventory", inventory);
			g_Game.SaveProfile();*/
		}
	}
	
	/* Played Time
		string cached_playtime_str = "";
		g_Game.GetProfileString("cachedPlaytime", cached_playtime_str);
		if ( cached_playtime_str != "" )
		{
			m_CachedPlaytime = cached_playtime_str.ToInt();
		}
	*/
	
	//g_Game.GetPlayerName(m_player_name);
}
