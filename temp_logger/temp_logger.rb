require 'rubygems'
require 'mail'
require 'date'
require 'net/http'
require './lib/weather_reading'
require './lib/wunderground_logger'

Upload_Url = "http://rtupdate.wunderground.com/weatherstation/updateweatherstation.php"

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

	if output.start_with? 'Sensor'
		reading = WeatherReading.from_sensor(output)

		logger.log(reading, Time.now)
	end
end

