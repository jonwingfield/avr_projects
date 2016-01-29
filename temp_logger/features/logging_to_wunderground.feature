Feature: Logging to Weather Underground

Scenario: Every sensor reading should be sent to Wunderground
    When I recieve a reading of Temperature: 25.2C, Humidity: 66% from the weather sensors
    Then it should log a reading of Temperature: 25.2C, Humidity: 66% to Wunderground.com 

Scenario: Notification should be sent when the temperature exceed the min threshold
    Given the minimum temperature threshold is 63.0F 
    When I recieve a reading of Temperature: 17.0C, Humidity: 66% from the weather sensors
    Then a notification should be sent indicating that the max temperature threshold was exceeded

Scenario: Notification should be sent when the temperature range exceeds a threshold
    Given the threshold for temperature range for 24 hours is 15.0F
    When I recieve a reading of Temperature: 25.2C, Humidity: 66% from the weather sensors
    And I recieve a reading of Temperature: 10.0C, Humidity: 66% from the weather sensors
    Then a notification should be sent indicating that a threshold was exceeded
