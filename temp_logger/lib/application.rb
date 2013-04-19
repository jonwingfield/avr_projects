require 'rubygems'
require 'mail'
require 'date'
require 'net/http'
require './lib/weather_reading'
require './lib/wunderground_logger'

class WeatherIntelligence
	def initialize sensor_device
		@sensor_device = sensor_device
		@logger = Wunderground_Logger.new('KFLSPRIN12', 'Wonderword11')
	end

	def gather
		sensor = File.new(@sensor_device)
		puts "Gathering Weather Intelligence from #{@sensor_device}"

		while (output = sensor.gets) 
			puts "Got sensor reading - #{output}"

			if output.start_with? 'Sensor'
				reading = WeatherReading.from_sensor(output)

				@logger.log(reading, Time.now)
			end
		end		

		puts "Done reading!"

		sensor.close
	end
end
