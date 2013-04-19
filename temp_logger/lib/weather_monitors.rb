require 'timer'

module Monitor
	attr_reader :timer

	def send_notifications_only time_period
		@timer = Timer.new(:time_period => time_period)
	end	

	def notify
		@timer ||= Timer.new

		if @timer.expired?
			@timer.reset!
			yield
		end
	end
end

class TemperatureRangeMonitor
	include Monitor

	def initialize options
		@max_variation = Temperature.from(options[:max_variation])
	end

	def when_range_exceeded indicate_range_exceeded
		@indicate_range_exceeded = indicate_range_exceeded
	end

	def extremes_changed history
		if history.max_temp.temp.c - history.min_temp.temp.c > @max_variation.c
			notify do
				if @indicate_range_exceeded.nil?
					warn('no notification defined for temperature range') 
				else
					@indicate_range_exceeded.call(
						"Maximum temperature range of #{@max_variation} exceeded: #{history.min_temp.temp} min and #{history.max_temp.temp} max")
				end				
			end
		end
	end
end

class TemperatureExtremeMonitor
	include Monitor

	def initialize options
		@max, @min = Temperature.from(options[:max]), Temperature.from(options[:min])
	end

	def when_extreme_exceeded indicate_extreme_exceeded
		@indicate_extreme_exceeded = indicate_extreme_exceeded
	end

	def extremes_changed history
		if history.max_temp.temp > @max 
			notify do
				@indicate_extreme_exceeded.call(
					"Temperature extremes of #{@max} - #{@min} exceeded: #{history.max_temp.temp}")
			end
		elsif history.min_temp.temp < @min 
			notify do 
				@indicate_extreme_exceeded.call(
					"Temperature extremes of #{@max} - #{@min} exceeded: #{history.min_temp.temp}")
			end
		end
	end
end
