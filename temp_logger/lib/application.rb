require 'rubygems'
require 'mail'
require 'date'
require 'net/http'

class WeatherIntelligence
	def initialize sensor_device
		@sensor_device = sensor_device
		@logger = Wunderground_Logger.new('KFLSPRIN12', 'Wonderword11')
                @history = WeatherHistory.new
                extremeMonitor = Monitors::TemperatureExtremeMonitor.new({ :max => '90F', :min => '63F' })
                extremeMonitor.when_extreme_exceeded(EmailNotifier.new({ :to => 'wingfield.jon@gmail.com' }))
                @history.add_notification extremeMonitor
	end

	def gather
		sensor = File.new(@sensor_device)
		puts "Gathering Weather Intelligence from #{@sensor_device}"

		while (output = sensor.gets) 
			puts "Got sensor reading - #{output}"

			if output.start_with? 'Sensor'
				reading = WeatherReading.from_sensor(output)

				@logger.log(reading, Time.now)
                                @history.log_event(reading, Time.now)
			end
		end		

		puts "Done reading!"

		sensor.close
	end
end
