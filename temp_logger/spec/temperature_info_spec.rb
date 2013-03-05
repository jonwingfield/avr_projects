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
end

describe 'Temperature' do
	it 'should default to celcius for values provided' do
		Temperature.new(22.4).c.should == 22.4
	end

	it 'should convert to farenheit' do
		Temperature.new(25.3).f.should == 77.5
	end
end

