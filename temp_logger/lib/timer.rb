class Timer
	def self.daily; Timer.new(:time_period => :daily); end
	def self.hourly; Timer.new(:time_period => :hourly); end

	attr_writer :clock

	def initialize options={}
		@clock = options[:clock] || Time

		time_period = options[:time_period]

		if time_period == :hourly
			@time_period = 60*60
		elsif time_period == :daily
			@time_period = 60*60*24
		else
			@time_period = 0
		end	

		@last_notification = @clock.at(0)
	end

	def expired?
		@clock.now - @last_notification >= @time_period
	end

	def reset!
		@last_notification = @clock.now
	end
end