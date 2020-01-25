void main()
{
	//INIT WEATHER BEFORE ECONOMY INIT------------------------
	Weather weather = g_Game.GetWeather();

	weather.MissionWeather(false);    // false = use weather controller from Weather.c

	weather.GetOvercast().Set( Math.RandomFloatInclusive(0.4, 0.6), 1, 0);
	weather.GetRain().Set( 0, 0, 1);
	weather.GetFog().Set( Math.RandomFloatInclusive(0.05, 0.1), 1, 0);

	//INIT ECONOMY--------------------------------------
	Hive ce = CreateHive();
	if ( ce )
		ce.InitOffline();

	//DATE RESET AFTER ECONOMY INIT-------------------------
	int year, month, day, hour, minute;
	int reset_month = 9, reset_day = 20;
	GetGame().GetWorld().GetDate(year, month, day, hour, minute);

	if ((month == reset_month) && (day < reset_day))
	{
		GetGame().GetWorld().SetDate(year, reset_month, reset_day, hour, minute);
	}
	else
	{
		if ((month == reset_month + 1) && (day > reset_day))
		{
			GetGame().GetWorld().SetDate(year, reset_month, reset_day, hour, minute);
		}
		else
		{
			if ((month < reset_month) || (month > reset_month + 1))
			{
				GetGame().GetWorld().SetDate(year, reset_month, reset_day, hour, minute);
			}
		}
	}
}

class CustomMission: MissionServer
{
	void SetRandomHealth(EntityAI itemEnt)
	{
		if ( itemEnt )
		{
			int rndHlt = Math.RandomInt(55,100);
			itemEnt.SetHealth("","",rndHlt);
		}
	}

	override PlayerBase CreateCharacter(PlayerIdentity identity, vector pos, ParamsReadContext ctx, string characterName)
	{
		Entity playerEnt;
		playerEnt = GetGame().CreatePlayer(identity, characterName, pos, 0, "NONE");//Creates random player
		Class.CastTo(m_player, playerEnt);

		GetGame().SelectPlayer(identity, m_player);

		return m_player;
	}

	override void StartingEquipSetup(PlayerBase player, bool clothesChosen)
	{
		player.RemoveAllItems();

		int rndHoodie = Math.RandomInt(0, 2);
		if ( rndHoodie == 0 )
		{
			player.GetInventory().CreateInInventory("Hoodie_RPUK_Red");
		}
		else
		{
			player.GetInventory().CreateInInventory("Hoodie_RPUK_Black");
		}
		
		player.GetInventory().CreateInInventory("Jeans_Blue");
		player.GetInventory().CreateInInventory("AthleticShoes_Black");
		
		EntityAI itemTop;
		EntityAI itemEnt;
		ItemBase itemBs;
		float rand;

		itemTop = player.FindAttachmentBySlotName("Body");

		if ( itemTop )
		{
			itemEnt = itemTop.GetInventory().CreateInInventory("Rag");
			if ( Class.CastTo(itemBs, itemEnt ) )
				itemBs.SetQuantity(4);

			SetRandomHealth(itemEnt);

			string chemlightArray[] = { "Chemlight_White", "Chemlight_Yellow", "Chemlight_Green", "Chemlight_Red" };
			int rndIndex = Math.RandomInt(0, 4);
			itemEnt = itemTop.GetInventory().CreateInInventory(chemlightArray[rndIndex]);
			SetRandomHealth(itemEnt);

			rand = Math.RandomFloatInclusive(0.0, 1.0);
			if ( rand < 0.35 )
				itemEnt = player.GetInventory().CreateInInventory("Apple");
			else if ( rand > 0.65 )
				itemEnt = player.GetInventory().CreateInInventory("Pear");
			else
				itemEnt = player.GetInventory().CreateInInventory("Plum");

			SetRandomHealth(itemEnt);
		}

	}
	
	///////////// WORK IN PROGRESS ADMIN PANEL ///////////////


/* 
	bool freecam_active = false;
	bool verify_admins = false; // true=verify presence of BI UID in admin list
	string cmd_prefix = "/"; // Must be special character
	ref TStringArray admins = {76561198206695552,}; // Add your BI UID or SteamID

	bool IsPlayerAnAdmin(PlayerBase player) {
		bool found = false;
		for ( int i = 0; i < admins.Count(); ++i ) {
			if(player.GetIdentity().GetId() == admins[i] || player.GetIdentity().GetPlainId() == admins[i]) { found=true; break; }
		}
		return found;
	}

	void SendMessageToPlayer(PlayerBase player, string message) {
		Param1<string> param = new Param1<string>( message );
		GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, param, true, player.GetIdentity());
	}

	bool IsPlayer(string name) { 
		PlayerBase p;
		array<Man> players = new array<Man>; GetGame().GetPlayers(players);
		for ( int i = 0; i < players.Count(); ++i ) {
			p = players.Get(i);
			if(p.GetIdentity().GetName() == name) return true;
		}
		return false;
	}

	PlayerBase GetPlayer(string name) {
		PlayerBase p;
		array<Man> players = new array<Man>; GetGame().GetPlayers(players);
		for ( int i = 0; i < players.Count(); ++i ) {
			p = players.Get(i);
			if(p.GetIdentity().GetName() == name) return p;
		}
		return NULL;
	}

	override void OnEvent(EventType eventTypeId, Param params)  {
		int i;
		PlayerBase player, temp_player;
		array<Man> players = new array<Man>; GetGame().GetPlayers(players);
		if(eventTypeId != ChatMessageEventTypeID) { super.OnEvent(eventTypeId,params); return; }
		ChatMessageEventParams chat_params = ChatMessageEventParams.Cast( params );
		if(chat_params.param1 != 0 || chat_params.param2 == "") { super.OnEvent(eventTypeId,params); return; }
		player = GetPlayer(chat_params.param2);
		if(player == NULL) { super.OnEvent(eventTypeId,params); return; }
		string message = chat_params.param3, prefix, param0, command;
		TStringArray tokens = new TStringArray;
		message.Replace("` ", "&SPCESC!");
		message.Split(" ", tokens); int count = tokens.Count();
		for ( i = 0; i < count; ++i ) {
			message = tokens[i];
			message.Replace("&SPCESC!", " ");
			tokens[i] = message;
		}

		param0 = tokens.Get(0);
		param0.ParseStringEx(prefix); if(prefix != cmd_prefix) { super.OnEvent(eventTypeId,params); return; };
		param0.ParseStringEx(command);
		if(verify_admins && !IsPlayerAnAdmin(player)) { GetGame().AdminLog("[ADMCMD] (Unauthorized) " + player.GetIdentity().GetName() +" ("+player.GetIdentity().GetPlainId()+", "+player.GetIdentity().GetId()+") tried to execute "+ chat_params.param3); return; }
		GetGame().AdminLog("[ADMCMD] PLAYER: "+ player.GetIdentity().GetName() +" ("+player.GetIdentity().GetPlainId()+", "+player.GetIdentity().GetId()+") CMD: "+ command);
		switch(command) {
			case "spawn": {
				if(count != 2) { SendMessageToPlayer(player, "/spawn [object]"); return; }
				GetGame().CreateObject(tokens[1], player.GetPosition(), false, true );
				SendMessageToPlayer(player, "[ObjectSpawn] Object spawned: " + tokens[1]);
				break;
			}

			case "inv": {
				if(count != 2) { SendMessageToPlayer(player, "/inv [object]"); return; }
				player.GetInventory().CreateInInventory(tokens[1]);
				SendMessageToPlayer(player, "[ObjectSpawn] Object spawned in inventory: " + tokens[1]);
				break;
			}

			case "weapon": {
				if(count != 2) { SendMessageToPlayer(player, "/weapon [weapon]"); return; }
				EntityAI weapon;
				switch(tokens[1]) {
					case "ump": {
						weapon = player.GetHumanInventory().CreateInHands("UMP45");
						player.GetInventory().CreateInInventory("Mag_UMP_25Rnd");
						break;
					}

					case "cz61": {
						weapon = player.GetHumanInventory().CreateInHands("CZ61");
						player.GetInventory().CreateInInventory("Mag_CZ61_20Rnd");
						break;
					}

					case "mp5": {
						weapon = player.GetHumanInventory().CreateInHands("MP5K");
						weapon.GetInventory().CreateAttachment("MP5k_StockBttstck");
						weapon.GetInventory().CreateAttachment("MP5_PRailHndgrd");
						player.GetInventory().CreateInInventory("Mag_MP5_30Rnd");
						break;
					}

					case "svd": {
						weapon = player.GetHumanInventory().CreateInHands("SVD");
						weapon.GetInventory().CreateAttachment("PSO11Optic");
						player.GetInventory().CreateInInventory("Mag_SVD_10Rnd");
						break;
					}

					case "mp133": {
						weapon = player.GetHumanInventory().CreateInHands("Mp133Shotgun");
						player.GetInventory().CreateInInventory("Ammo_12gaPellets");
						break;
					}

					case "mosin": {
						weapon = player.GetHumanInventory().CreateInHands("Mosin9130");
						weapon.GetInventory().CreateAttachment("PUScopeOptic");
						player.GetInventory().CreateInInventory("Ammo_762x54");
						break;
					}

					case "m4": {
						weapon = player.GetHumanInventory().CreateInHands("M4A1_Black");
						weapon.GetInventory().CreateAttachment("M4_RISHndgrd_Black");
						weapon.GetInventory().CreateAttachment("M4_MPBttstck_Black");
						weapon.GetInventory().CreateAttachment("BUISOptic");
						player.GetInventory().CreateInInventory("Mag_STANAGCoupled_30Rnd");
						break;
					}

					case "akm": {
						weapon = player.GetHumanInventory().CreateInHands("AKM");
						weapon.GetInventory().CreateAttachment("AK_RailHndgrd");
						weapon.GetInventory().CreateAttachment("AK_PlasticBttstck");
						player.GetInventory().CreateInInventory("Mag_AKM_30Rnd");
						break;
					}

					case "izh18": {
						weapon = player.GetHumanInventory().CreateInHands("Izh18");
						player.GetInventory().CreateInInventory("Ammo_762x39");
						break;
					}

					case "fnx": {
						weapon = player.GetHumanInventory().CreateInHands("FNX45");
						player.GetInventory().CreateInInventory("Mag_FNX45_15Rnd");
						break;
					}

					default: {
						SendMessageToPlayer(player, "[WeaponSpawner] " + tokens[1] + " not found");
						break;
					}
				}
				break;
			}
				
			case "tp": {
				if(count != 2) { SendMessageToPlayer(player, "/tp [POI]"); return; }
				vector poi;
				switch(tokens[1]) {
					case "nwaf": {
						poi = "4660 0 10330";
						SendMessageToPlayer(player, "[POI-TP] Teleported to NWAF");
						break;
					}

					case "nwaf_tents": {
						poi = "3160 0 11000";
						SendMessageToPlayer(player, "[POI-TP] Teleported to NWAF-Tents");
						break;
					}

					case "tisy": {
						poi = "1600 0 14000";
						SendMessageToPlayer(player, "[POI-TP] Teleported to Tisy");
						break;
					}

					case "vmc": {
						poi = "4600 0 8200";
						SendMessageToPlayer(player, "[POI-TP] Teleported to Vybor Military Compound");
						break;
					}

					case "pmc": {
						poi = "1830 0 3450";
						SendMessageToPlayer(player, "[POI-TP] Teleported to Pavlovo Military Compound");
						break;
					}

					case "tmc": {
						poi = "7850 0 14700";
						SendMessageToPlayer(player, "[POI-TP] Teleported to Troitskoe Military Compound");
						break;
					}

					default: {
						SendMessageToPlayer(player, "[POI-TP] " + tokens[1] + " not found");
						SendMessageToPlayer(player, "Available POI: ");
						return;
					}
				}

				poi[1] = GetGame().SurfaceY(poi[0], poi[2]);
				player.SetPosition(poi);
				break;
			}

			case "strip": {
				if(count != 2) { SendMessageToPlayer(player, "/strip [player]"); return; }
				temp_player = GetPlayer(tokens[1]);
				if(temp_player == NULL) {
					SendMessageToPlayer(player, "[Strip] Can't find player called: '"+tokens[1]+"'");
				} else {
					temp_player.RemoveAllItems();
					SendMessageToPlayer(player, "[Strip] You stripped " + temp_player.GetIdentity().GetName());
					SendMessageToPlayer(temp_player, "You have been stripped by an admin");
				}
				break;
			}

			case "slap": {
				if(count != 2) { SendMessageToPlayer(player, "/slap [player]"); return; }
				temp_player = GetPlayer(tokens[1]);
				if(temp_player == NULL) {
					SendMessageToPlayer(player, "[Slap] Can't find player called: '"+tokens[1]+"'");
				} else {
					temp_player.SetPosition(temp_player.GetPosition() + "0 3 0");
					SendMessageToPlayer(player, "[Slap] You stripped " + temp_player.GetIdentity().GetName());
					SendMessageToPlayer(temp_player, "You have been slapped by an admin");
				}
				break;
			}

			case "topos": {
				if (count < 3) { SendMessageToPlayer(player, "/topos [x] [y] (player)"); return; }
				float ATL_Z = GetGame().SurfaceY(tokens[1].ToFloat(), tokens[2].ToFloat());
				vector reqpos = Vector(tokens[1].ToFloat(), ATL_Z, tokens[2].ToFloat());
				temp_player = player;
				if (count == 4) { 
					temp_player = GetPlayer(tokens[3]); 
					if (temp_player == NULL) {
						SendMessageToPlayer(player, "[Teleport] Can't find player called: '"+tokens[3]+"'"); return;
					} else {
						SendMessageToPlayer(temp_player, "[Teleport] You've been teleported to " + reqpos + " by admin " + player.GetIdentity().GetName());
					}
				}
				temp_player.SetPosition(reqpos);
				SendMessageToPlayer(player, "[Teleport] Target teleported to " + temp_player.GetPosition());
				break;
			}

			case "goto": {
				if(count != 2) { SendMessageToPlayer(player, "/goto [player]"); return; }
				temp_player = GetPlayer(tokens[1]);
				if(temp_player == NULL) {
					SendMessageToPlayer(player, "[Teleport] Can't find player called: '"+tokens[1]+"'");
				} else {
					player.SetPosition(temp_player.GetPosition());
					SendMessageToPlayer(player, "[Teleport] You teleported to " + temp_player.GetIdentity().GetName());
				}
				break;
			}

			case "allgoto": {
				PlayerBase allgoto_target;
				if(count != 2) { SendMessageToPlayer(player, "/allgoto [player]"); return; }
				allgoto_target = GetPlayer(tokens[1]);
				if(allgoto_target == NULL) {
					SendMessageToPlayer(player, "[Teleport] Can't find player called: '"+tokens[1]+"'");
				} else {
					SendMessageToPlayer(player, "[Teleport] You teleported everyone to your location");
					for (i = 0; i < players.Count(); i++) {
						temp_player = players.Get(i);
						SendMessageToPlayer(temp_player, "[Teleport] You have been teleported to player " + allgoto_target.GetIdentity().GetName());
						temp_player.SetPosition(allgoto_target.GetPosition());
					}
				}
				break;
			}

			case "here": {
				if(count != 2) { SendMessageToPlayer(player, "/here [player]"); return; }
				temp_player = GetPlayer(tokens[1]);
				if(temp_player == NULL) {
					SendMessageToPlayer(player, "[Teleport] Can't find player called: '"+tokens[1]+"'");
				} else {
					temp_player.SetPosition(player.GetPosition());
					SendMessageToPlayer(temp_player, "[Teleport] You have been teleported to admin " + player.GetIdentity().GetName());
					SendMessageToPlayer(player, "[Teleport] You teleported " + temp_player.GetIdentity().GetName() + " to your location");
				}
				break;
			}

			case "allhere": {
				SendMessageToPlayer(player, "[Teleport] You teleported everyone to your location");
				for (i = 0; i < players.Count(); i++) {
					temp_player = players.Get(i);
					SendMessageToPlayer(temp_player, "[Teleport] You have been teleported to admin " + player.GetIdentity().GetName());
					temp_player.SetPosition(player.GetPosition());
				}
				break;
			}

			case "time": {
				if(count != 3) { SendMessageToPlayer(player, "/time [hour] [minute]"); return; }
				GetGame().GetWorld().SetDate( 2018, 1, 7, tokens[1].ToInt(), tokens[2].ToInt());
				SendMessageToPlayer(player, "[Servertime] You have set the servertime to " + tokens[1] + ":"+tokens[2]);
				break;
			}

			case "day": {
				GetGame().GetWorld().SetDate( 2018, 1, 7, 12, 0);
				SendMessageToPlayer(player, "[Servertime] You have set the servertime to daytime");
				break;
			}

			case "night": {
				GetGame().GetWorld().SetDate( 2018, 1, 7, 24, 0);
				SendMessageToPlayer(player, "[Servertime] You have set the servertime to daytime");
				break;
			}

			///////////////// 
                /*
			case "rain": {
				if(count != 2) { SendMessageToPlayer(player, "/rain [value 0-100]"); return; }
				float rain = tokens[1].ToFloat() / 100;
				GetGame().GetWeather().GetRain().Set(rain, 0, 600);
				SendMessageToPlayer(player, "[Weather] You have set Rain to " + tokens[1] + "% ["+rain+"]");
				break;
			}
			case "fog": {
				if(count != 2) { SendMessageToPlayer(player, "/rain [value 0-100]"); return; }
				float fog = tokens[1].ToFloat() / 100;
				GetGame().GetWeather().GetFog().Set(fog, 0, 600);
				SendMessageToPlayer(player, "[Weather] You have set Fog to " + tokens[1] + "% ["+fog+"]");
				break;
			}
			case "overcast": {
				if(count != 2) { SendMessageToPlayer(player, "/rain [value 0-100]"); return; }
				float overcast = tokens[1].ToFloat() / 100;
				GetGame().GetWeather().GetOvercast().Set(overcast, 0, 600);
				SendMessageToPlayer(player, "[Weather] You have set Overcast to " + tokens[1] + "% ["+overcast+"]");
				break;
			}
			///////////////////// */

		/*	case "kill": {
				if(count == 2) {
					temp_player = GetPlayer(tokens[1]);
					if(temp_player == NULL) {
						SendMessageToPlayer(player, "[Kill] Can't find player called: '"+tokens[1]+"'");
					} else {
						temp_player.SetHealth(0);
						SendMessageToPlayer(player, "[Kill] You killed " + temp_player.GetIdentity().GetName());
					}
				} else {
					player.SetHealth(0);
					SendMessageToPlayer(player, "[Kill] You killed yourself");
				}
				break;
			}

			case "killall": {
				SendMessageToPlayer(player, "[Kill] You killed everyone");
				for (i = 0; i < players.Count(); i++) {
					temp_player = players.Get(i);
					if(temp_player.GetIdentity().GetId() == player.GetIdentity().GetId()) continue;
					temp_player.SetHealth(0);
				}
				break;
			}

			case "heal": {
				PlayerBase heal_target;
				if(count == 2) {
					heal_target = GetPlayer(tokens[1]);
					if(heal_target == NULL) {
						SendMessageToPlayer(player, "[Heal] Can't find player called: '"+tokens[1]+"'");
					} else {
						SendMessageToPlayer(player, "[Heal] You healed " + heal_target.GetIdentity().GetName());
					}
				} else {
					heal_target = player;
					SendMessageToPlayer(player, "[Heal] You healed yourself");
				}
				if(heal_target != NULL) {
					heal_target.SetHealth(heal_target.GetMaxHealth("", ""));
					heal_target.SetHealth("", "Blood", heal_target.GetMaxHealth("", "Blood"));
					heal_target.GetStatStamina().Set(1000);
					heal_target.GetStatEnergy().Set(1000);
					heal_target.GetStatWater().Set(1000);
				}
				break;
			}

			case "freecam": {
				PlayerBase body = player;
				if(freecam_active) {
					freecam_active = false;
					GetGame().SelectPlayer(player.GetIdentity(), body);
					SendMessageToPlayer(player, "[Freecam] Exited");

				} else {
					freecam_active = true;
					GetGame().SelectPlayer(player.GetIdentity(), NULL);
					GetGame().SelectSpectator(player.GetIdentity(), "freedebugcamera", player.GetPosition());	
					SendMessageToPlayer(player, "[Freecam] Entered");
				}
				break;
			}

			case "offroad": {
				SendMessageToPlayer(player, "[Offroad] Vehicled spawned");
				Car v;
				float playerAngle = MiscGameplayFunctions.GetHeadingAngle(player);
				vector posModifier = Vector(-(3 * Math.Sin(playerAngle)), 0, 3 * Math.Cos(playerAngle));
				v = Car.Cast(GetGame().CreateObject( "OffroadHatchback", player.GetPosition() + posModifier));
				
				v.GetInventory().CreateAttachment("SparkPlug");
				v.GetInventory().CreateAttachment("EngineBelt");
				v.GetInventory().CreateAttachment("CarBattery");
				v.GetInventory().CreateAttachment("HatchbackHood");
				v.GetInventory().CreateAttachment("HatchbackTrunk");
				v.GetInventory().CreateAttachment("HatchbackDoors_CoDriver");
				v.GetInventory().CreateAttachment("HatchbackWheel");
				v.GetInventory().CreateAttachment("HatchbackWheel");
				v.GetInventory().CreateAttachment("HatchbackWheel");
				v.GetInventory().CreateAttachment("HatchbackWheel");
				v.GetInventory().CreateAttachment("HatchbackWheel"); // spare
				break;
			}

			case "refuel": {
				ref array<Object> nearest_objects = new array<Object>;
				ref array<CargoBase> proxy_cargos = new array<CargoBase>;
				Car toBeFilled;
				vector position = player.GetPosition();
				GetGame().GetObjectsAtPosition ( position, 10, nearest_objects, proxy_cargos );
		
				for (i = 0; i < nearest_objects.Count(); i++) {
					if (nearest_objects[i].IsKindOf("CarScript")) {
						SendMessageToPlayer(player, "[Refuel] Found car: '"+nearest_objects[i]+"'");
						toBeFilled = Car.Cast(nearest_objects[i]);
						float fuelReq = toBeFilled.GetFluidCapacity( CarFluid.FUEL ) - (toBeFilled.GetFluidCapacity( CarFluid.FUEL ) * toBeFilled.GetFluidFraction( CarFluid.FUEL ));
						float oilReq = toBeFilled.GetFluidCapacity( CarFluid.OIL ) - (toBeFilled.GetFluidCapacity( CarFluid.OIL ) * toBeFilled.GetFluidFraction( CarFluid.OIL ));
						float coolantReq = toBeFilled.GetFluidCapacity( CarFluid.COOLANT ) - (toBeFilled.GetFluidCapacity( CarFluid.COOLANT ) * toBeFilled.GetFluidFraction( CarFluid.COOLANT ));
						float brakeReq = toBeFilled.GetFluidCapacity( CarFluid.BRAKE ) - (toBeFilled.GetFluidCapacity( CarFluid.BRAKE ) * toBeFilled.GetFluidFraction( CarFluid.BRAKE ));
						toBeFilled.Fill( CarFluid.FUEL, fuelReq );
						toBeFilled.Fill( CarFluid.OIL, oilReq );
						toBeFilled.Fill( CarFluid.COOLANT, coolantReq );
						toBeFilled.Fill( CarFluid.BRAKE, brakeReq );
						SendMessageToPlayer(player, "[Refuel] "+fuelReq+"L added, all fluids maxed");
					}
				}
				
				break;
			}

			default: {
				SendMessageToPlayer(player, "Unknown command: " + command);
				break;
			}
		}
	}
	*/

};

Mission CreateCustomMission(string path)
{
	return new CustomMission();
}