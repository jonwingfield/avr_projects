require 'weather_reading'
require 'date'

class HistoryRecord
	attr_reader :temp, :time

	def initialize(temp, time)
		@temp = Temperature.from(temp)
		@time = time
	end
end

class WeatherHistory
	attr_reader :max_temp, :min_temp

	def initialize
		@max_temp = HistoryRecord.new(Temperature.new(-100), DateTime.now)
		@min_temp = HistoryRecord.new(Temperature.new(100), DateTime.now)
		@notifications = []
	end

	def log_event(reading, time)
		if reading.temperature > @max_temp.temp or reading.temperature < @min_temp.temp
			@max_temp = HistoryRecord.new(reading.temperature, time) if reading.temperature > @max_temp.temp
			@min_temp = HistoryRecord.new(reading.temperature, time) if reading.temperature < @min_temp.temp

			@notifications.each do |notification|
				notification.extremes_changed(self)
			end
		end
	end

	def add_notification notification
		@notifications.push notification
	end
end

