require 'rubygems'
require 'mail'
require 'date'
require 'net/http'
require_relative 'lib/'

Mail.defaults do
	delivery_method :smtp, { 
		:address => 'smtp.gmail.com',
		:port => '587',
	        :user_name => ENV['GMAIL_SMTP_USER'],
		:password => ENV['GMAIL_SMTP_PASSWORD'],
		:authentication => :plain,
		:enable_starttls_auto => true
	}
end

sensor = File.new('/dev/ttyUSB0')
logger = Wunderground_Logger.new('KFLSPRIN12', 'Wonderword11')

while (output = sensor.gets) 
	puts output

	prev = nil
	if output.start_with? 'Sensor'
		reading = WeatherReading.from_sensor(output)

		if not prev or ((reading.temperature.f - prev.temperature.f).abs <= 10.0 and reading.temperature.f <= 100.0) 
			prev = reading
			logger.log(reading, Time.now)
		end
	end
end

