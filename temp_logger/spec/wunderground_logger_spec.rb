require 'webmock/rspec'
require 'wunderground_logger'

describe Wunderground_Logger do
	id = 'KFLSPRIN12'
	password = 'Wonderword11'

	let(:logger) { Wunderground_Logger.new(id, password) }

	it "should submit the provided weather reading to weather underground" do
		temp = Temperature.new(22.3)
		humidity = 88.3
		time = Time.now

		stub_request(:get, 
			     'http://rtupdate.wunderground.com/weatherstation/updateweatherstation.php')
			.with(:query => {
				:action => 'updateraw',
				:ID => id,
				:PASSWORD => password,
				:tempf => temp.f,
			        :humidity => 88,
				:dewptf => 68.5,
				:dateutc => time.utc.to_s.gsub(' UTC', ''),
				:realtime => 1,
				:rtfreq => 10
			}).to_return(:body => 'success')

		reading = WeatherReading.new(temp.c, humidity)
		logger.log(reading, time)
			.should == true
	end	

	it "should round the humidity value" do
		temp = Temperature.new(22.3)
		humidity = 23.8
		time = Time.now

		stub_request(:get, 
			     'http://rtupdate.wunderground.com/weatherstation/updateweatherstation.php')
			.with(:query => hash_including({ "humidity" => "24" }))
			.to_return(:body => 'success')

		reading = WeatherReading.new(temp.c, humidity)
		logger.log(reading, time)
			.should == true

	end

	it "should indicate if logging failed" do
		stub_wunderground_log_request
			.to_return(:body => 'INVALIDPASSWORDID|Password and/or id are incorrect')

		reading = WeatherReading.new(22.2, 50.0)
		logger.log(reading, Time.now)
		      .should == false
	end

	it "should indicate if logging resulted in an http error" do
		stub_wunderground_log_request
			.to_raise(StandardError)

		reading = WeatherReading.new(22.2, 50.0)
		logger.log(reading, Time.now)
			.should == false
	end

	def stub_wunderground_log_request
		stub_request(:get, 
			     'http://rtupdate.wunderground.com/weatherstation/updateweatherstation.php')
			.with(:query => hash_including({}))
	end
end
