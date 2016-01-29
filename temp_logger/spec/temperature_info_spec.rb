require 'weather_reading'

describe WeatherReading do
	it 'Parses the temperature and humidity values provided' do
		data = "Sensor Id: 1, Temperature: 35.3C, Humidity: 17.4%\n"

		reading = WeatherReading.from_sensor(data)

		reading.temperature.c.should == 35.3
		reading.humidity.should == 17.4
	end

	it 'Calculates the dewpoint from the temperature and humidity' do
		data = "Sensor Id: 1, Temperature: 32.3C, Humidity: 80.4%\n"

		reading = WeatherReading.from_sensor(data)

		reading.dewpoint.c.should == 28.5
	end
	it 'sanity checks humidity values' do
		data = "Sensor Id: 1, Temperature: 32.3C, Humidity: 100.1%\n"

		reading = WeatherReading.from_sensor(data)

		reading.humidity.should == 100.0
	end

	it 'sanity checks temperature values' do
		data = "Sensor Id: 1, Temperature: 45.1C, Humidity: 100.1%\n"

		reading = WeatherReading.from_sensor(data)

		reading.temperature.c.should == 45.0
	end
end


