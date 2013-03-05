require 'weather_reading'
require 'date'

class HistoryRecord
	attr_reader :temp, :time

	def initialize(temp, time)
		@temp = temp
		@time = time
	end
end

class WeatherHistory
	attr_reader :max_temp, :min_temp

	def initialize
		@max_temp = HistoryRecord.new(Temperature.new(-100), DateTime.now)
		@min_temp = HistoryRecord.new(Temperature.new(-100), DateTime.now)
	end

	def log_event(reading, time)
		@max_temp = HistoryRecord.new(reading.temperature, time) if reading.temperature > @max_temp.temp
		@min_temp = HistoryRecord.new(reading.temperature, time) if reading.temperature < @max_temp.temp
	end
end

