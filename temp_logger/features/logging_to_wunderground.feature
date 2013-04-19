Feature: Logging to Weather Underground

Scenario: Every sensor reading should be sent to Wunderground
	When I recieve a reading of Temperature: 25.2C, Humidity: 66% from the weather sensors
 	Then it should log a reading of Temperature: 25.2C, Humidity: 66% to Wunderground.com 