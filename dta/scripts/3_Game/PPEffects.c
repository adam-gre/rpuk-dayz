class PPEffects
{
	// COLORIZE IDs
	static const int COLORIZE_NV = 100;
	
	//-------------------------------------------------------
	// BLUR START
	//-------------------------------------------------------
	static int 		m_BlurInventory;
	static int 		m_BlurDrunk;
	static int		m_BlurFever;
	static int 		m_BlurMenu;
	static int 		m_BlurOptics;
	static int 		m_BlurFlashbang;
	
	static int	 	m_BurlapBlindness;
	static int 		m_DyingEffect;
	
	static float	m_BloodSaturation;
	
	static ref array<float> m_BlurValues;
	static ref map<int, ref array<float>> m_ColorValues;
	static ref array<float> m_ColorEffect;
	static ref map<int, ref array<float>> m_ColorizeEffects;
	
	static float m_UnconsciousVignetteColor[4];
	static float m_UnconsciousVignetteIntesity;
	
	static float m_ColorValueTotal[4] = {0,0,0,0};
	static float m_ColorOverlayTotal;

	static Material m_MatColors;
	
	static void Init()
	{
		//Print("---Init PPEffects---");
		if ( m_BlurValues )
		{
			delete m_BlurValues;
		}
		
		m_MatColors = GetGame().GetWorld().GetMaterial("graphics/materials/postprocess/glow");
		m_BlurValues = new array<float>;
		
		// add new blur effects here
		m_BlurInventory		= RegisterBlurEffect();
		m_BlurDrunk			= RegisterBlurEffect();
		m_BlurFever			= RegisterBlurEffect();
		m_BlurMenu			= RegisterBlurEffect();
		m_BlurOptics 		= RegisterBlurEffect();
		m_BlurFlashbang		= RegisterBlurEffect();
		
		if ( m_ColorEffect )
		{
			delete m_ColorEffect;
		}
		
		m_ColorEffect = new array<float>;
		m_ColorValues = new map<int, ref array<float>>;
		
		// add new color effects here
		m_BurlapBlindness 	= RegisterColorEffect();
		m_DyingEffect 		= RegisterColorEffect();
		
		// ------------------------NV-related stuff below------------------------
		ref array<float> colorizeDefault = {0.0, 0.0, 0.0};
		m_ColorizeEffects = new map<int, ref array<float>>;
		
		// colorize: r, g, b
		// colorize effects registration
		m_ColorizeEffects.Set(PPEffects.COLORIZE_NV, colorizeDefault);
		SetNVParams(1.0, 0.0, 2.35, 2.75); //default values
		// ------------------------End of NV-related stuff------------------------
	}
	
	static void ResetBlurEffects()
	{	
		if( m_BlurValues )
		{
			for ( int i = 0; i < m_BlurValues.Count(); ++i )
			{
				m_BlurValues[i] = 0;
			}
			UpdateBlur();
		}	
	}
		
	static void SetBlurValue(int index, float value)
	{
		if ( m_BlurValues && index < m_BlurValues.Count() )
		{
			m_BlurValues[index] = value;
		}
		else
		{
			Print("Error: PPEffects: m_BlurValues with index: "+ index +" is not registered.");
		}
	}
	
	static int RegisterBlurEffect()
	{
		return m_BlurValues.Insert(0);
	}
	
	//-------------------------------------------------------
	//! sets blur effect to an absolute value between 0..1
	static void SetBlur(float value)
	{
		Material mat_blur = GetGame().GetWorld().GetMaterial("graphics/materials/postprocess/gauss");
		
		if (mat_blur)
		{
			mat_blur.SetParam("Intensity", value);
		}
	}
	//-------------------------------------------------------
	//! updates the blur post process effect where the resulting blur is an aggregate of all individual blur effect values
	static void UpdateBlur()
	{		
		float blur_value_total = 0;
		if( m_BlurValues )
		{
			for ( int i = 0; i < m_BlurValues.Count(); ++i )
			{
				blur_value_total += m_BlurValues[i];
			}
		}
		
		SetBlur( blur_value_total );
	}
	//-------------------------------------------------------
	//! Set blur inventory effect to a specified 'value' between 0..1
	static void SetBlurInventory(float value)
	{
		SetBlurValue(m_BlurInventory, value);
		UpdateBlur();
	}
	//-------------------------------------------------------
	//! Set blur drunk effect to a specified 'value' between 0..1
	static void SetBlurDrunk(float value)
	{
		SetBlurValue(m_BlurDrunk, value);
		UpdateBlur();
	}

	//-------------------------------------------------------
	//! Set blur drunk effect to a specified 'value' between 0..1
	static void SetBlurFever(float value)
	{
		SetBlurValue(m_BlurFever, value);
		UpdateBlur();
	}
	
	//-------------------------------------------------------
	//! Set menu blur to a specified 'value' between 0..1
	static void SetBlurMenu(float value)
	{
		SetBlurValue(m_BlurMenu, value);
		UpdateBlur();
	}
	
	//-------------------------------------------------------
	//! Set optics and ironsights blur to a specified 'value' between 0..1
	static void SetBlurOptics(float value)
	{
		SetBlurValue(m_BlurOptics, value);
		UpdateBlur();
	}
	
	//-------------------------------------------------------
	//! Set blur flashbang hit effect to a specified 'value' between 0..1
	static void SetBlurFlashbang(float value)
	{
		SetBlurValue(m_BlurFlashbang, value);
		UpdateBlur();
	}
	
	//-------------------------------------------------------
	// BLUR END
	//-------------------------------------------------------
	/*
	static void SetOverlayColor(float r, float g, float b, float a)
	{
		Material matColors = GetGame().GetWorld().GetMaterial("graphics/materials/postprocess/glow");
		
		m_Color[0] = r;
		m_Color[1] = g;
		m_Color[2] = b;
		m_Color[3] = a;
		
		matColors.SetParam("OverlayColor", m_Color);
		matColors.SetParam("OverlayFactor", a);

	}*/
	
	static void ResetColorEffects()
	{	
		if( m_ColorEffect )
		{
			for ( int i = 0; i < m_ColorEffect.Count(); ++i )
			{
				m_ColorEffect[i] = 0;
			}
			UpdateColor();
		}
	}
	
	static void SetColorValue(int index, float r, float g, float b, float a, float overlay)
	{
		if ( index < m_ColorEffect.Count() )
		{		
			ref array<float> values = {r,g,b,a,overlay};
			
			m_ColorValues.Set(index, values);
		}
		else
		{
			Print("Error: PPEffects: m_ColorValues with index: "+ index +" is not registered.");
		}
	}
	
	static int RegisterColorEffect()
	{
		return m_ColorEffect.Insert(0);
	}	
	
	static void UpdateColor()
	{
		float color_value_total[4] = {0,0,0,0};
		float color_overlay;
		/*
		m_MatColors.ResetParam("OverlayColor");
		m_MatColors.ResetParam("OverlayFactor");
		*/
		if( !GetGame() || !GetGame().GetWorld() )
		{
			return;
		}

		for ( int i = 0; i < m_ColorValues.Count(); ++i )
		{
			int key = m_ColorValues.GetKey(i);
			ref array<float> value = m_ColorValues.Get(key);
			
			color_value_total[0] 	= color_value_total[0] + value[0];
			color_value_total[1] 	= color_value_total[1] + value[1];
			color_value_total[2] 	= color_value_total[2] + value[2];
			color_value_total[3] 	= color_value_total[3] + value[3];
			color_overlay  			+= value[4];
		}
		
		m_ColorValueTotal = color_value_total;
		m_ColorOverlayTotal = color_overlay;
		m_MatColors.SetParam("OverlayColor", color_value_total);
		m_MatColors.SetParam("OverlayFactor", color_overlay);	
	}

	/*!
	set lens effect 
	\param lens	<-5, 5>, 0 = disable (performance plus), > 0 = outside effect, < 0 inside effect
	\param chromAbb	<0, 1>, chromaticity, 1 = max, 0 disable (performance plus)
	\param centerX	<-1, 1>, center of effect, 0 = screen center in X
	\param centerY	<-1, 1>, center of effect, 0 = screen center in Y
	*/
	static void SetLensEffect(float lens, float chromAbb, float centerX, float centerY)
	{
		PerformSetLensEffect(lens, chromAbb, centerX, centerY);
		/*Material matHDR = GetGame().GetWorld().GetMaterial("Graphics/Materials/postprocess/glow");
                matHDR.SetParam("LensDistort", lens);
                matHDR.SetParam("MaxChromAbberation", chromAbb);
                matHDR.SetParam("LensCenterX", centerX);
                matHDR.SetParam("LensCenterY", centerY);*/
	}

	//!added for convenience
	static void PerformSetLensEffect(float lens, float chromAbb, float centerX, float centerY)
	{
		//Material matHDR = GetGame().GetWorld().GetMaterial("Graphics/Materials/postprocess/glow");
        m_MatColors.SetParam("LensDistort", lens);
        m_MatColors.SetParam("MaxChromAbberation", chromAbb);
        m_MatColors.SetParam("LensCenterX", centerX);
        m_MatColors.SetParam("LensCenterY", centerY);
	}
	
	/*!
	set vignette
	\param intensity <0, 1>, intensity of effect, 0 = disable
	\param R	
	\param G	
	\param B	
	*/
	static void SetVignette(float intensity, float R, float G, float B)
	{
		//Material matHDR = GetGame().GetWorld().GetMaterial("Graphics/Materials/postprocess/glow");

		float color[4];
		color[0] = R;
		color[1] = G;
		color[2] = B;
		color[3] = 0;

		m_MatColors.SetParam("Vignette", intensity);
		m_MatColors.SetParam("VignetteColor", color);
	}


	static void ResetVignette()
	{
		SetVignette(0,0,0,0);
	}

	static void OverrideDOF(bool enable, float focusDistance, float focusLength, float focusLengthNear, float blur, float focusDepthOffset)
	{
		GetGame().OverrideDOF(enable, focusDistance, focusLength, focusLengthNear, blur, focusDepthOffset);
	}

	static void AddPPMask(float ndcX, float ndcY, float ndcRadius, float ndcBlur)
	{
		GetGame().AddPPMask(ndcX, ndcY, ndcRadius, ndcBlur);
	}

	static void ResetPPMask()
	{
		if( GetGame() ) GetGame().ResetPPMask();
	}
	
	static void ResetDOFOverride()
	{
		OverrideDOF(false,0,0,0,0,1);
	}
	
	static void ResetLensEffect()
	{
		SetLensEffect(0,0,0,0);
	}

	static void HitEffect(float value)
	{
		float m_HitEffectColor[4];
		m_HitEffectColor[0] = Math.Lerp(Math.Clamp(m_ColorValueTotal[0],0,1), 1, value);
		m_HitEffectColor[1] = 0;
		m_HitEffectColor[2] = 0;
		m_HitEffectColor[3] = Math.Lerp(Math.Clamp(m_ColorValueTotal[0],0,1), 1, value);
		/*
		Print("---------------------------");
		Print("m_ColorValueTotal[0]: " + m_ColorValueTotal[0]);
		Print("value: " + value);
		Print("---------------------------");
		Print("r: " + m_HitEffectColor[0]);
		Print("g: " + m_HitEffectColor[1]);
		Print("b: " + m_HitEffectColor[2]);
		Print("a: " + m_HitEffectColor[3]);
		*/
		m_MatColors.SetParam("OverlayColor", m_HitEffectColor);
		m_MatColors.SetParam("OverlayFactor", 0.05);
	}
	
	static void FlashbangEffect(float value)
	{
		float hitEffectColor[4];
		hitEffectColor[0] = 1;
		hitEffectColor[1] = 1;
		hitEffectColor[2] = 1;
		hitEffectColor[3] = Math.Lerp(Math.Clamp(m_ColorValueTotal[0],0,1), 1, value);

		m_MatColors.SetParam("OverlayColor", hitEffectColor);
		m_MatColors.SetParam("OverlayFactor", 0.75);
	}
	
	static void EnableBurlapSackBlindness()
	{
		SetColorValue(m_BurlapBlindness, 0, 0, 0, 1, 1.0);
		UpdateColor();
		g_Game.SetEVValue(-5);
	}

	static void DisableBurlapSackBlindness()
	{		
		SetColorValue(m_BurlapBlindness, 0, 0, 0, 0, 0.0);
		UpdateColor();
		g_Game.SetEVValue(0);
	}
	
	static void SetDeathDarkening(float value) 
	{
		//SetColorValue(m_HitColor, 0, 0, 0, 0, 0); //workaround to colour addition of hit effect
		SetColorValue(m_DyingEffect, 0, 0, 0, 1, value);
		UpdateColor();
		if (value > 0.99)
			g_Game.SetEVValue(-5); //additional "darkness" to avoid lens flare
		else
			g_Game.SetEVValue(0);
	}

	static void UpdateSaturation()
	{
		//Material matColors = GetGame().GetWorld().GetMaterial("graphics/materials/postprocess/glow");
		m_MatColors.SetParam("Saturation", m_BloodSaturation/*+add_additional_modifiers_here*/);
	}
	
	static void UpdateVignette()
	{
		float color[4];
		float intesity;
		
		color[0] = m_UnconsciousVignetteColor[0]/*+add_additional_modifiers_here*/;
		color[1] = m_UnconsciousVignetteColor[1]/*+add_additional_modifiers_here*/;
		color[2] = m_UnconsciousVignetteColor[2]/*+add_additional_modifiers_here*/;
		
		intesity = m_UnconsciousVignetteIntesity/*+add_additional_modifiers_here*/;
		
		SetVignette( intesity, color[0], color[1], color[2] );
		
	}

	static void SetBloodSaturation(float value)
	{
		m_BloodSaturation = value;
		UpdateSaturation();
	}
	
	static void SetUnconsciousnessVignette(float value)
	{
		m_UnconsciousVignetteIntesity = value;
		UpdateVignette();
	}
	
	static void RemoveUnconsciousnessVignette()
	{
		m_UnconsciousVignetteIntesity = 0;
		UpdateVignette();
	}
	
	// appropriate parts of the code will call these functions
	static void SetColorizationNV(float r, float g, float b)
	{
		ref array<float> colorizeArray = {r, g, b};
		m_ColorizeEffects.Set(PPEffects.COLORIZE_NV, colorizeArray);
		UpdateColorize();
	}
	
	static void UpdateColorize()
	{
		bool foundActiveEffect = false;
		int lowestKey = 1000000;
		ref array<float> chosenArray;
		// search for active effect with highest priority (lower value of key better)
		for (int i = 0; i < m_ColorizeEffects.Count(); i++)
		{
			int currentKey = m_ColorizeEffects.GetKey(i);
			ref array<float> colorizeValues = m_ColorizeEffects.Get(currentKey);
			// check for non-zero active effect
			for (int j = 0; j < colorizeValues.Count(); j++)
			{
				if (colorizeValues[j] != 0.0)
				{
					if (currentKey < lowestKey)
					{
						chosenArray = colorizeValues;
						lowestKey = currentKey;
						foundActiveEffect = true;
						break;
					}
				}
			}
		}
		if (foundActiveEffect)
		{
			// active effect found
			//Material matHDR = GetGame().GetWorld().GetMaterial("Graphics/Materials/postprocess/glow");
			float color[4];
			color[0] = chosenArray[0];
			color[1] = chosenArray[1];
			color[2] = chosenArray[2];
			color[3] = 0;
	        m_MatColors.SetParam("ColorizationColor", color);
		}
		else
		{
			// no active event found, reset colorize effect
			ResetColorize();
		}
	}
	static void ResetColorize()
	{
		//Material matHDR = GetGame().GetWorld().GetMaterial("Graphics/Materials/postprocess/glow");
		float color[4];
		color[0] = 1.0;
		color[1] = 1.0;
		color[2] = 1.0;
		color[3] = 0;
        m_MatColors.SetParam("ColorizationColor", color);
	}

	// EV check for NV optics
	static void SetEVValuePP(float value)
	{
		g_Game.SetEVValue(value);
	}
	
	// light multiplier and noise intensity (using filmgrainNV.emat!) for nvg
	// added other parameters for filmgrainNV.emat, sharpness and grain size
	static void SetNVParams(float light_mult, float noise_intensity, float sharpness, float grain_size)
	{
		Material matHDR = GetGame().GetWorld().GetMaterial("Graphics/Materials/postprocess/filmgrainNV");
/*#ifdef PLATFORM_CONSOLE
//worst-case scenario console fix!
		noise_intensity = 0.0;
#endif*/
		g_Game.NightVissionLightParams(light_mult, noise_intensity);
		matHDR.SetParam("Sharpness", sharpness);
		matHDR.SetParam("GrainSize", grain_size);
	}
	
	// bloom PP, experimental stuff
	static void SetBloom(float thres, float steep, float inten)
    {
		//Material matHDR = GetGame().GetWorld().GetMaterial("Graphics/Materials/postprocess/glow");
		m_MatColors.SetParam("BloomThreshold", thres);
		m_MatColors.SetParam("BloomSteepness", steep);
		m_MatColors.SetParam("BloomIntensity", inten);
    }

	static void ResetAll()
	{
		ResetBlurEffects();
		ResetColorEffects();
		ResetVignette();
		ResetPPMask();
		ResetDOFOverride();
		ResetLensEffect();
		SetBloodSaturation(1);
		RemoveUnconsciousnessVignette();
		ResetColorize();
	}	
};