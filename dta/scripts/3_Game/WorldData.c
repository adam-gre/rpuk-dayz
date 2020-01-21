//! Keeps information about currently loaded world, like temperature
class WorldData
{
	protected float m_DayTemperature;
	protected float m_NightTemperature;
	protected Weather m_Weather;
	
	void WorldData()
	{
		Init();
	}
	
	void Init()
	{
		m_DayTemperature = 10;
		m_NightTemperature = 6;
		m_Weather = g_Game.GetWeather();
	}
	
	float GetDayTemperature()
	{
		return m_DayTemperature;
	}

	float GetNightTemperature()
	{
		return m_NightTemperature;
	}

	bool WeatherOnBeforeChange( EWeatherPhenomenon type, float actual, float change, float time )
	{
		// default behaviour is same like setting MissionWeather (in Weather) to true
		return false;
	}
};