When(/^I recieve a reading of Temperature: (.*)C, Humidity: (\d+)% from the weather sensors$/) do |temp, humidity|
	reading = WeatherReading.new(temp.to_f, humidity.to_f)

	query = {
		:action => 'updateraw',
		:ID => 'KFLSPRIN12',
		:PASSWORD => "Wonderword11",
		:tempf => reading.temperature.f,
        :humidity => reading.humidity.to_i,
		:dewptf => reading.dewpoint.f,
		:dateutc => Time.now.utc.to_s.gsub(' UTC', ''),
		:realtime => 1,
		:rtfreq => 10
	}

	@stub_request = stub_request(:get, 
	     'http://rtupdate.wunderground.com/weatherstation/updateweatherstation.php')
		.with(:query => query)
		.to_return(:body => 'success')

	MockSensor.puts("Sensor: 1, Temperature: #{temp}C, Humidity: #{humidity}%\n\n")
end

Then(/^it should log a reading of Temperature: (.*)C, Humidity: (\d+)% to Wunderground\.com$/) do |temp, humidity|
  assert_requested @stub_request
end
