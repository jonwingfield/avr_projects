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

Then /^it should log a reading of Temperature: (.*)C, Humidity: (\d+)% to Wunderground\.com$/  do |temp, humidity|
  assert_requested @stub_request
end

Given /^the threshold for temperature range for (\d+) hours is (.*)F$/  do |hours, deg_f|
  # this is the default for now
end

Then /^a notification should be sent indicating that a threshold was exceeded$/  do
  pending # express the regexp above with the code you wish you had
end

Given /^the minimum temperature threshold is (.*)F$/  do |temp_f|
  # this is the default for now
  throw "Really implement this" unless temp_f == "63.0"
end

Then /^a notification should be sent indicating that the max temperature threshold was exceeded$/  do
  sleep 1.0/100.0
  mail = Mail::TestMailer.deliveries.first
  mail.to.should == ["wingfield.jon@gmail.com"]
  mail.from.should == ["weather_monitor@test.com"]
  mail.subject.should == "Temperature extremes of 32.2C - 17.2C exceeded: 17.0C"
end
