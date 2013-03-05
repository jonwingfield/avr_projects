require 'rubygems'
require 'mail'
require 'date'
require 'net/http'
require './lib/weather_reading'

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

while (line = sensor.gets) 
	puts line

	if line.start_with? 'Sensor'
		reading = WeatherReading.from_sensor(line)

		uri = URI(Upload_Url)
			uri.query = URI.encode_www_form({
			:action => 'updateraw',
			:ID => 'KFLSPRIN12',
			:PASSWORD => 'Wonderword11',
			:tempf => reading.temperature.f,
			:humidity => reading.humidity,
			:dewptf => reading.dewpoint.f,
			:dateutc => Time.now.utc.to_s.gsub(' UTC', ''),
			:realtime => 1,
			:rtfreq => 10
		})

		begin
			res = Net::HTTP.get_response(uri)
			puts Time.now.to_s + ": " + res.body if res.is_a?(Net::HTTPSuccess)
		rescue
			$stderr.print "${Time.now.to_s}: #{$!}\n"
		end
	end
end

